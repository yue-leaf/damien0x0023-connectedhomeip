/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>

#include "trace.h"
#include "io_ir.h"

/**
  * @brief  Entry of APP code
  * @return int (To avoid compile warning)
*/
int main(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);
    __enable_irq();

    ir_demo();

    while (1)
    {
        ;

    }
}



