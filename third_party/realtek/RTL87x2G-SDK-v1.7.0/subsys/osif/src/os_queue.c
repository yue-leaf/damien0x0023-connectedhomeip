/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "app_section.h"
#include "os_queue.h"
#include "osif.h"
#include "trace.h"


void os_queue_init_imp(T_OS_QUEUE *p_queue)
{
    p_queue->p_first = NULL;
    p_queue->p_last  = NULL;
    p_queue->count   = 0;
}

void os_queue_in_imp(T_OS_QUEUE *p_queue, void *p_elem)
{
    T_OS_QUEUE_ELEM *p_queue_elem = (T_OS_QUEUE_ELEM *)p_elem;

    p_queue_elem->p_next = NULL;

    if (p_queue->p_last == NULL)
    {
        p_queue->p_first = p_queue_elem;
    }
    else
    {
        p_queue->p_last->p_next = p_queue_elem;
    }

    p_queue->p_last = p_queue_elem;
    p_queue->count++;
}

void *os_queue_out_imp(T_OS_QUEUE *p_queue)
{
    T_OS_QUEUE_ELEM *p_first = p_queue->p_first;

    if (p_first != NULL)
    {
        p_queue->p_first = p_first->p_next;

        if (p_queue->p_first == NULL)
        {
            p_queue->p_last = NULL;
        }

        p_queue->count--;
    }

    return p_first;
}

void *os_queue_peek_imp(T_OS_QUEUE *p_queue, int32_t index)
{
    T_OS_QUEUE_ELEM *p_curr_elem = NULL;

    if (index == 0)
    {
        return p_queue->p_first;
    }

    if (index == -1)
    {
        return p_queue->p_last;
    }

    if (index >= p_queue->count)
    {
        return NULL;
    }

    if (index + p_queue->count < 0)
    {
        return NULL;
    }

    if (index < 0)
    {
        index = index + p_queue->count;
    }

    p_curr_elem = p_queue->p_first;
    while (index > 0 && p_curr_elem != NULL)
    {
        p_curr_elem = p_curr_elem->p_next;
        index--;
    }

    return p_curr_elem;
}

bool os_queue_search_imp(T_OS_QUEUE *p_queue, void *p_elem)
{
    T_OS_QUEUE_ELEM *p_curr_elem  = p_queue->p_first;
    T_OS_QUEUE_ELEM *p_queue_elem = (T_OS_QUEUE_ELEM *)p_elem;

    while (p_curr_elem != NULL &&
           p_curr_elem != p_queue_elem)
    {
        p_curr_elem = p_curr_elem->p_next;
    }

    /* queue is empty or the element is not found */
    if (p_curr_elem == NULL)
    {
        return false;
    }

    return true;
}

void os_queue_insert_imp(T_OS_QUEUE *p_queue, void *p_elem, void *p_new_elem)
{
    T_OS_QUEUE_ELEM *p_queue_elem     = (T_OS_QUEUE_ELEM *)p_elem;
    T_OS_QUEUE_ELEM *p_queue_new_elem = (T_OS_QUEUE_ELEM *)p_new_elem;

    /* if p_queue is empty, insert p_new_elem directly */
    if (p_queue->p_first == NULL)
    {
        p_queue->p_first = p_queue_new_elem;
        p_queue->p_last = p_queue_new_elem;
        p_queue_new_elem->p_next = NULL;
    }
    /* if p_elem is null, insert p_new_elem as first element */
    else if (p_queue_elem == NULL)
    {
        p_queue_new_elem->p_next = p_queue->p_first;
        p_queue->p_first = p_queue_new_elem;
    }
    /* caller should make sure that p_elem belongs to p_queue */
    else
    {
        p_queue_new_elem->p_next = p_queue_elem->p_next;
        p_queue_elem->p_next = p_queue_new_elem;
    }

    p_queue->count++;
}

bool os_queue_delete_imp(T_OS_QUEUE *p_queue, void *p_elem)
{
    T_OS_QUEUE_ELEM *p_queue_elem = (T_OS_QUEUE_ELEM *)p_elem;
    T_OS_QUEUE_ELEM *p_curr_elem  = p_queue->p_first;
    T_OS_QUEUE_ELEM *p_next_elem  = NULL;

    /* queue is empty */
    if (p_curr_elem == NULL)
    {
        return false;
    }

    /* first element is the one to be deleted */
    if (p_curr_elem == p_queue_elem)
    {
        p_queue->p_first = p_curr_elem->p_next;

        if (p_queue->p_first == NULL)
        {
            p_queue->p_last = NULL;
        }

        p_queue->count--;
        return true;
    }

    while ((p_next_elem = p_curr_elem->p_next) != NULL &&
           p_next_elem != p_queue_elem)
    {
        p_curr_elem = p_next_elem;
    }

    /* the element to be deleted is not in the queue */
    if (p_next_elem == NULL)
    {
        return false;
    }

    p_curr_elem->p_next = p_next_elem->p_next;

    if (p_curr_elem->p_next == NULL)
    {
        p_queue->p_last = p_curr_elem;
    }

    p_queue->count--;
    return true;
}

void os_queue_func_init(void) APP_FLASH_TEXT_SECTION;
void os_queue_func_init(void)
{
    os_queue_init = os_queue_init_imp;
    os_queue_in = os_queue_in_imp;
    os_queue_out = os_queue_out_imp;
    os_queue_peek = os_queue_peek_imp;
    os_queue_search = os_queue_search_imp;
    os_queue_insert = os_queue_insert_imp;
    os_queue_delete = os_queue_delete_imp;
}
