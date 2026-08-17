/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "app_section.h"
#include "os_mem.h"
#include "os_pool.h"
#include "os_queue.h"
#include "os_sync.h"
#include "osif.h"
#include "trace.h"


typedef struct t_os_slot
{
    struct t_os_slot *p_next;
    uint8_t           pool_id;
    uint16_t          buf_size;
    uint16_t          buf_count;
    void             *header_addr;
    void             *payload_addr;
    T_OS_QUEUE        buf_q;
} T_OS_SLOT;

/* OS Pool Buffer Bit Mask */
#define OS_BUFFER_ALLOCATED     0x01

typedef struct t_os_buffer
{
    struct t_os_buffer *p_next;
    T_OS_SLOT          *p_slot;
    uint16_t            flags;
    void               *p_data;
} T_OS_BUFFER;

#define POOL_HDR_SIZE       sizeof(T_OS_BUFFER)

/**
 * OS Pool 32 bit flags are divided into 4 sections.
 * bit  0 ~  7: set pool status.
 * bit  8 ~ 15: set pool memory type.
 * bit 16 ~ 23: reserved.
 * bit 24 ~ 31: set pool buffer alignment.
 */
#define OS_POOL_CREATED     0x01

#define OS_POOL_MEM_TYPE_SET(flag, type)    ( (flag) |= (((uint8_t)(type)) << 8)         )
#define OS_POOL_MEM_TYPE_GET(flag, type)    ( (type) = (RAM_TYPE)(((flag) >> 8) & 0xFF)  )

#define OS_POOL_MEM_ALIGN_SET(flag, align)  ( (flag) |= (((uint8_t)(align)) << 24)       )
#define OS_POOL_MEM_ALIGN_GET(flag, align)  ( (align) = (uint8_t)(((flag) >> 24) & 0xFF) )

typedef struct t_os_pool
{
    uint32_t    flags;
    T_OS_QUEUE  slot_q;
} T_OS_POOL;

/* maximum number of pools created by os_pool_create */
#define OS_POOL_TABLE_SIZE      4

#define POOL_BUFFER_ADDR_MASK   (sizeof(void *) - 1)

#define POOL_BUFFER_ALIGN_SIZE  (sizeof(void *))
#define POOL_BUFFER_ALIGN_MASK  (POOL_BUFFER_ALIGN_SIZE - 1)

T_OS_POOL os_pool_table[OS_POOL_TABLE_SIZE];

bool os_slot_create(uint8_t pool_id, RAM_TYPE ram_type, uint16_t buf_size, uint16_t buf_count)
{
    T_OS_POOL   *p_pool;
    T_OS_SLOT   *p_slot;
    T_OS_SLOT   *p_prev_slot;
    T_OS_SLOT   *p_curr_slot;
    T_OS_BUFFER *p_buf;
    void        *p_hdr, *p_data;
    size_t       data_len;
    uint16_t     i;

    p_slot = (T_OS_SLOT *)os_mem_zalloc(RAM_TYPE_DATA_ON, sizeof(T_OS_SLOT));
    if (p_slot == NULL)
    {
        return false;
    }

    p_hdr = (void *)os_mem_alloc(RAM_TYPE_DATA_ON, POOL_HDR_SIZE * buf_count);
    if (p_hdr == NULL)
    {
        os_mem_free(p_slot);
        return false;
    }

    data_len = (sizeof(void *) + buf_size + POOL_BUFFER_ALIGN_MASK) & ~POOL_BUFFER_ALIGN_MASK;
    p_data = (void *)os_mem_alloc(ram_type, data_len * buf_count);
    if (p_data == NULL)
    {
        os_mem_free(p_hdr);
        os_mem_free(p_slot);
        return false;
    }

    for (i = 0; i < buf_count; i++)
    {
        /* sizeof(T_OS_BUFFER) will be default alignment */
        p_buf          = (T_OS_BUFFER *)((uint8_t *)p_hdr + POOL_HDR_SIZE * i);
        p_buf->p_data  = (uint8_t *)p_data + data_len * i;
        p_buf->p_slot  = p_slot;
        p_buf->flags   = 0;
        p_buf->p_next  = NULL;
        memcpy(p_buf->p_data, &p_buf, sizeof(T_OS_BUFFER *));

        os_queue_in(&p_slot->buf_q, p_buf);
    }

    p_pool      = &os_pool_table[pool_id];
    p_prev_slot = NULL;
    p_curr_slot = (T_OS_SLOT *)p_pool->slot_q.p_first;
    /* sort slots in ascending buffer size order */
    while (p_curr_slot != NULL)
    {
        if (p_curr_slot->buf_size >= buf_size)
        {
            break;
        }
        p_prev_slot = p_curr_slot;
        p_curr_slot = (T_OS_SLOT *)p_curr_slot->p_next;
    }

    p_slot->buf_size  = buf_size;
    p_slot->buf_count = buf_count;
    p_slot->pool_id   = pool_id;

    p_slot->header_addr  = p_hdr;
    p_slot->payload_addr = p_data;

    os_queue_insert(&p_pool->slot_q, p_prev_slot, p_slot);

    return true;
}

bool os_pool_create_intern_imp(uint8_t *p_handle, RAM_TYPE ram_type, uint16_t buf_size,
                               uint16_t buf_count, const char *p_func, uint32_t file_line)
{
    T_OS_POOL  *p_pool = NULL;
    uint8_t     id;

    if (buf_size == 0 || buf_count == 0)
    {
        OSIF_PRINT_ERROR4("os_pool_create_intern: %s<%u> invalid buf size %u, buf count %u",
                          TRACE_STRING(p_func), file_line, buf_size, buf_count);
        return false;
    }

    for (id = 0; id < OS_POOL_TABLE_SIZE; id++)
    {
        if (!(os_pool_table[id].flags & OS_POOL_CREATED))
        {
            p_pool = &os_pool_table[id];
            p_pool->flags = 0;
            break;
        }
    }

    if (id == OS_POOL_TABLE_SIZE)
    {
        OSIF_PRINT_ERROR2("os_pool_create_intern: %s<%u> pool table overflow",
                          TRACE_STRING(p_func), file_line);
        return false;
    }

    if (os_slot_create(id, ram_type, buf_size, buf_count) == false)
    {
        OSIF_PRINT_ERROR2("os_pool_create_intern: %s<%u> os_slot_create failed",
                          TRACE_STRING(p_func), file_line);
        return false;
    }

    p_pool->flags |= OS_POOL_CREATED;

    OS_POOL_MEM_TYPE_SET(p_pool->flags, ram_type);

    *p_handle = id;

    return true;
}

bool os_pool_extend_intern_imp(uint8_t handle, uint16_t buf_size, uint16_t buf_count,
                               const char *p_func, uint32_t file_line)
{
    T_OS_POOL *p_pool;
    RAM_TYPE   ram_type;

    if (buf_size == 0 || buf_count == 0)
    {
        OSIF_PRINT_ERROR4("os_pool_extend_intern: %s<%u> invalid buf size %u, buf count %u",
                          TRACE_STRING(p_func), file_line, buf_size, buf_count);
        return false;
    }

    if (handle >= OS_POOL_TABLE_SIZE)
    {
        OSIF_PRINT_ERROR3("os_pool_extend_intern: %s<%u> invalid pool handle %u",
                          TRACE_STRING(p_func), file_line, handle);
        return false;
    }

    p_pool = &os_pool_table[handle];
    if (!(p_pool->flags & OS_POOL_CREATED))
    {
        OSIF_PRINT_ERROR3("os_pool_extend_intern: %s<%u> invalid pool flags 0x%08x",
                          TRACE_STRING(p_func), file_line, p_pool->flags);
        return false;
    }

    OS_POOL_MEM_TYPE_GET(p_pool->flags, ram_type);

    if (os_slot_create(handle, ram_type, buf_size, buf_count) == false)
    {
        OSIF_PRINT_ERROR2("os_pool_extend_intern: %s<%u> os_pool_slot_create failed",
                          TRACE_STRING(p_func), file_line);
        return false;
    }

    return true;
}

bool os_pool_delete_intern_imp(uint8_t handle, const char *p_func, uint32_t file_line)
{
    T_OS_POOL  *p_pool;
    T_OS_SLOT  *p_slot;
    uint32_t    s;

    if (handle >= OS_POOL_TABLE_SIZE)
    {
        OSIF_PRINT_ERROR3("os_pool_delete_intern: %s<%u> invalid handle %u",
                          TRACE_STRING(p_func), file_line, handle);
        return false;
    }

    p_pool = &os_pool_table[handle];
    if (!(p_pool->flags & OS_POOL_CREATED))
    {
        OSIF_PRINT_ERROR3("os_pool_delete_intern: %s<%u> pool %u not created",
                          TRACE_STRING(p_func), file_line, handle);
        return false;
    }

    s = os_lock();

    p_slot = (T_OS_SLOT *)p_pool->slot_q.p_first;
    while (p_slot != NULL)
    {
        if (p_slot->buf_count != p_slot->buf_q.count)
        {
            OSIF_PRINT_ERROR3("os_pool_delete_intern: %s<%u> pool %u is busy",
                              TRACE_STRING(p_func), file_line, handle);
            os_unlock(s);
            return false;
        }

        p_slot = (T_OS_SLOT *)p_slot->p_next;
    }

    while ((p_slot = os_queue_out(&p_pool->slot_q)) != NULL)
    {
        os_mem_free(p_slot->header_addr);
        os_mem_free(p_slot->payload_addr);
        os_mem_free(p_slot);
    }

    os_unlock(s);

    p_pool->flags &= ~OS_POOL_CREATED;

    return true;
}

void *os_buffer_get_intern_imp(uint8_t handle, uint16_t buf_size,
                               const char *p_func, uint32_t file_line)
{
    void        *p_buf = NULL;
    T_OS_BUFFER *p_buf_hdr;
    T_OS_SLOT   *p_slot;
    T_OS_POOL   *p_pool;
    uint32_t     s;

    if (buf_size == 0)
    {
        OSIF_PRINT_ERROR3("os_buffer_get_intern: %s<%u> invalid buf size %u",
                          TRACE_STRING(p_func), file_line, buf_size);
        return NULL;
    }

    if (handle >= OS_POOL_TABLE_SIZE)
    {
        OSIF_PRINT_ERROR3("os_buffer_get_intern: %s<%u> invalid handle %u",
                          TRACE_STRING(p_func), file_line, handle);
        return NULL;
    }

    p_pool = &os_pool_table[handle];
    if (!(p_pool->flags & OS_POOL_CREATED))
    {
        OSIF_PRINT_ERROR3("os_buffer_get_intern: %s<%u> pool %u not created",
                          TRACE_STRING(p_func), file_line, handle);
        return NULL;
    }

    p_slot = (T_OS_SLOT *)p_pool->slot_q.p_first;
    s = os_lock();
    while (p_slot != NULL)
    {
        if (p_slot->buf_size >= buf_size)
        {
            if (p_slot->buf_q.count > 0)
            {
                break; /* find the buffer */
            }
        }

        p_slot = (T_OS_SLOT *)p_slot->p_next;
    }

    if (p_slot == NULL)
    {
        os_unlock(s);
        OSIF_PRINT_ERROR3("os_buffer_get_intern: %s<%u> no buf in size %u",
                          TRACE_STRING(p_func), file_line, buf_size);
        return NULL;
    }

    p_buf_hdr = (T_OS_BUFFER *)os_queue_out(&p_slot->buf_q);

    os_unlock(s);

    p_buf_hdr->flags = OS_BUFFER_ALLOCATED;

    memcpy(p_buf_hdr->p_data, &p_buf_hdr, sizeof(T_OS_BUFFER *));

    p_buf = (uint8_t *)p_buf_hdr->p_data + sizeof(T_OS_BUFFER *);

    return p_buf;
}

bool os_buffer_put_intern_imp(void *p_buf, const char *p_func, uint32_t file_line)
{
    T_OS_BUFFER *p_buf_hdr;
    T_OS_SLOT   *p_slot;
    uint32_t     s;

    if ((p_buf == NULL) || ((size_t)p_buf & POOL_BUFFER_ADDR_MASK))
    {
        OSIF_PRINT_ERROR3("os_buffer_put_intern: %s<%u> invalid buf %p",
                          TRACE_STRING(p_func), file_line, p_buf);
        return false;
    }

    memcpy(&p_buf_hdr, (uint8_t *)p_buf - sizeof(void *), sizeof(void *));

    if (!(p_buf_hdr->flags & OS_BUFFER_ALLOCATED))
    {
        OSIF_PRINT_ERROR3("os_buffer_put_intern: %s<%u> dobule free buf %p",
                          TRACE_STRING(p_func), file_line, p_buf);
        return false;
    }

    p_buf_hdr->flags = 0;

    p_slot = p_buf_hdr->p_slot;

    s = os_lock();
    os_queue_in(&p_slot->buf_q, p_buf_hdr);
    os_unlock(s);

    return true;
}

void os_pool_dump_imp(uint8_t handle)
{
    uint8_t    pool_id;
    uint8_t    pool_max;
    T_OS_SLOT *p_slot;

    if (handle >= OS_POOL_TABLE_SIZE)
    {
        /* dump all pools if invalid handle */
        pool_id = 0;
        pool_max = OS_POOL_TABLE_SIZE;
    }
    else
    {
        pool_id = handle;
        pool_max = pool_id + 1;
    }

    for (; pool_id < pool_max; pool_id++)
    {
        if (os_pool_table[pool_id].flags & OS_POOL_CREATED)
        {
            OSIF_PRINT_ERROR3("pool id %d, flags 0x%x, slot num %d",
                              pool_id, os_pool_table[pool_id].flags, os_pool_table[pool_id].slot_q.count);

            p_slot = (T_OS_SLOT *)os_pool_table[pool_id].slot_q.p_first;
            while (p_slot)
            {
                OSIF_PRINT_ERROR5("slot(%p), pool id %d, buf size %d, total num %d, free num %d",
                                  p_slot, p_slot->pool_id, p_slot->buf_size,
                                  p_slot->buf_count, p_slot->buf_q.count);

                p_slot = (T_OS_SLOT *)p_slot->p_next;
            }
        }
    }
}

void os_pool_func_init(void) APP_FLASH_TEXT_SECTION;
void os_pool_func_init(void)
{
    os_pool_create_intern = os_pool_create_intern_imp;
    os_pool_extend_intern = os_pool_extend_intern_imp;
    os_pool_delete_intern = os_pool_delete_intern_imp;
    os_buffer_get_intern = os_buffer_get_intern_imp;
    os_buffer_put_intern = os_buffer_put_intern_imp;
    os_pool_dump = os_pool_dump_imp;
}
