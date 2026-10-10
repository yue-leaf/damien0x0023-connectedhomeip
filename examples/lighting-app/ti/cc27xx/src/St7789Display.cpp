/*
 * Copyright (c) 2020-2025, Texas Instruments Incorporated
 * Copyright (c) 2026 Project CHIP Authors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Texas Instruments Incorporated nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "St7789Display.h"

#include <ti/drivers/GPIO.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti_drivers_config.h>

namespace {
constexpr uint32_t kSpiBitRate = 8000000;
constexpr uint8_t kCommandColumnAddressSet = 0x2A;
constexpr uint8_t kCommandRowAddressSet    = 0x2B;
constexpr uint8_t kCommandMemoryWrite      = 0x2C;
} // namespace

bool St7789Display::Transfer(const uint8_t * data, size_t count)
{
    SPI_Transaction transaction = {};
    transaction.count           = count;
    transaction.txBuf           = const_cast<uint8_t *>(data);
    transaction.rxBuf           = nullptr;
    return SPI_transfer(mSpi, &transaction);
}

CHIP_ERROR St7789Display::WriteCommand(uint8_t command)
{
    GPIO_write(CONFIG_GPIO_LCD_DC, 0);
    GPIO_write(CONFIG_GPIO_LCD_CS, 0);
    bool ok = Transfer(&command, 1);
    GPIO_write(CONFIG_GPIO_LCD_CS, 1);
    GPIO_write(CONFIG_GPIO_LCD_DC, 1);
    return ok ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR St7789Display::WriteData(const uint8_t * data, size_t count)
{
    if (data == nullptr || count == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    GPIO_write(CONFIG_GPIO_LCD_DC, 1);
    GPIO_write(CONFIG_GPIO_LCD_CS, 0);
    bool ok = Transfer(data, count);
    GPIO_write(CONFIG_GPIO_LCD_CS, 1);
    return ok ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR St7789Display::WriteCommandData(uint8_t command, const uint8_t * data, size_t count)
{
    CHIP_ERROR error = WriteCommand(command);
    if (error != CHIP_NO_ERROR || count == 0)
    {
        return error;
    }
    return WriteData(data, count);
}

CHIP_ERROR St7789Display::SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t address[4] = { static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1), static_cast<uint8_t>(x2 >> 8),
                           static_cast<uint8_t>(x2) };
    CHIP_ERROR error = WriteCommandData(kCommandColumnAddressSet, address, sizeof(address));
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    address[0] = static_cast<uint8_t>(y1 >> 8);
    address[1] = static_cast<uint8_t>(y1);
    address[2] = static_cast<uint8_t>(y2 >> 8);
    address[3] = static_cast<uint8_t>(y2);
    error      = WriteCommandData(kCommandRowAddressSet, address, sizeof(address));
    return error == CHIP_NO_ERROR ? WriteCommand(kCommandMemoryWrite) : error;
}

CHIP_ERROR St7789Display::Init()
{
    static const uint8_t b2[] = { 0x1F, 0x1F, 0x00, 0x33, 0x33 };
    static const uint8_t e0[] = { 0xF0, 0x04, 0x07, 0x04, 0x04, 0x04, 0x25, 0x33, 0x3C, 0x36, 0x14, 0x12, 0x29, 0x30 };
    static const uint8_t e1[] = { 0xF0, 0x02, 0x04, 0x05, 0x05, 0x21, 0x25, 0x32, 0x3B, 0x38, 0x12, 0x14, 0x27, 0x31 };
    static const uint8_t e4[] = { 0x1D, 0x00, 0x00 };
    static const uint8_t d0[] = { 0xA4, 0xA1 };
    SPI_Params params;

    GPIO_write(CONFIG_GPIO_LCD_CS, 1);
    GPIO_write(CONFIG_GPIO_LCD_DC, 1);
    GPIO_write(CONFIG_GPIO_LCD_RESET, 1);
    GPIO_write(CONFIG_GPIO_LCD_BACKLIGHT, 0);

    SPI_init();
    SPI_Params_init(&params);
    params.transferMode = SPI_MODE_BLOCKING;
    params.dataSize     = 8;
    params.bitRate      = kSpiBitRate;
    params.frameFormat  = SPI_POL0_PHA0;
    mSpi                = SPI_open(CONFIG_SPI_LCD, &params);
    if (mSpi == nullptr)
    {
        return CHIP_ERROR_OPEN_FAILED;
    }

    GPIO_write(CONFIG_GPIO_LCD_RESET, 0);
    ClockP_usleep(100000);
    GPIO_write(CONFIG_GPIO_LCD_RESET, 1);
    ClockP_usleep(100000);

    CHIP_ERROR error = WriteCommand(0x11);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }
    ClockP_usleep(120000);

    uint8_t data = 0x00;
#define LCD_TRY(operation)                                                                                                  \
    do                                                                                                                      \
    {                                                                                                                       \
        error = (operation);                                                                                                \
        if (error != CHIP_NO_ERROR)                                                                                         \
        {                                                                                                                   \
            return error;                                                                                                   \
        }                                                                                                                   \
    } while (false)

    LCD_TRY(WriteCommandData(0x36, &data, 1));
    data = 0x05;
    LCD_TRY(WriteCommandData(0x3A, &data, 1));
    LCD_TRY(WriteCommandData(0xB2, b2, sizeof(b2)));
    data = 0x35;
    LCD_TRY(WriteCommandData(0xB7, &data, 1));
    data = 0x2B;
    LCD_TRY(WriteCommandData(0xBB, &data, 1));
    data = 0x2C;
    LCD_TRY(WriteCommandData(0xC0, &data, 1));
    data = 0x01;
    LCD_TRY(WriteCommandData(0xC2, &data, 1));
    data = 0x0F;
    LCD_TRY(WriteCommandData(0xC3, &data, 1));
    data = 0x20;
    LCD_TRY(WriteCommandData(0xC4, &data, 1));
    data = 0x13;
    LCD_TRY(WriteCommandData(0xC6, &data, 1));
    LCD_TRY(WriteCommandData(0xD0, d0, sizeof(d0)));
    data = 0xA1;
    LCD_TRY(WriteCommandData(0xD6, &data, 1));
    LCD_TRY(WriteCommandData(0xE0, e0, sizeof(e0)));
    LCD_TRY(WriteCommandData(0xE1, e1, sizeof(e1)));
    LCD_TRY(WriteCommandData(0xE4, e4, sizeof(e4)));
    LCD_TRY(WriteCommand(0x21));
    LCD_TRY(WriteCommand(0x29));
#undef LCD_TRY

    return CHIP_NO_ERROR;
}

CHIP_ERROR St7789Display::SetBacklight(bool enabled)
{
    GPIO_write(CONFIG_GPIO_LCD_BACKLIGHT, enabled ? 1 : 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR St7789Display::Fill(uint16_t rgb565)
{
    return FillRect(0, 0, kWidth, kHeight, rgb565);
}

CHIP_ERROR St7789Display::FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t rgb565)
{
    if (width == 0 || height == 0 || x >= kWidth || y >= kHeight || width > kWidth - x || height > kHeight - y)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < sizeof(mPixelBuffer); i += 2)
    {
        mPixelBuffer[i]     = static_cast<uint8_t>(rgb565 >> 8);
        mPixelBuffer[i + 1] = static_cast<uint8_t>(rgb565);
    }

    CHIP_ERROR error = SetWindow(x, y, static_cast<uint16_t>(x + width - 1), static_cast<uint16_t>(y + height - 1));
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    size_t remaining = static_cast<size_t>(width) * height * 2;
    GPIO_write(CONFIG_GPIO_LCD_DC, 1);
    GPIO_write(CONFIG_GPIO_LCD_CS, 0);
    while (remaining > 0)
    {
        size_t count = remaining > sizeof(mPixelBuffer) ? sizeof(mPixelBuffer) : remaining;
        if (!Transfer(mPixelBuffer, count))
        {
            GPIO_write(CONFIG_GPIO_LCD_CS, 1);
            return CHIP_ERROR_INTERNAL;
        }
        remaining -= count;
    }
    GPIO_write(CONFIG_GPIO_LCD_CS, 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR St7789Display::DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t * data)
{
    if (data == nullptr || width == 0 || height == 0 || x >= kWidth || y >= kHeight || width > kWidth - x ||
        height > kHeight - y)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR error = SetWindow(x, y, static_cast<uint16_t>(x + width - 1), static_cast<uint16_t>(y + height - 1));
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    size_t remaining = static_cast<size_t>(width) * height * 2;
    GPIO_write(CONFIG_GPIO_LCD_DC, 1);
    GPIO_write(CONFIG_GPIO_LCD_CS, 0);
    while (remaining > 0)
    {
        size_t count = remaining > sizeof(mPixelBuffer) ? sizeof(mPixelBuffer) : remaining;
        if (!Transfer(data, count))
        {
            GPIO_write(CONFIG_GPIO_LCD_CS, 1);
            return CHIP_ERROR_INTERNAL;
        }
        data += count;
        remaining -= count;
    }
    GPIO_write(CONFIG_GPIO_LCD_CS, 1);
    return CHIP_NO_ERROR;
}
