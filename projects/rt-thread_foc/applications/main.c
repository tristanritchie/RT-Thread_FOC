/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-30     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "mc_rtthread/mc_foc.h"

int main(void)
{
    mc_foc_init();

    int count = 1;

    while (count)
    {
        rt_thread_suspend(rt_thread_self());
    }

    return RT_EOK;
}
