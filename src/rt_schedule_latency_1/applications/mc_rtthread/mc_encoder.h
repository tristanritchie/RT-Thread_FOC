/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "drivers/pulse_encoder.h"
#include "mc_user_definitions.h"


void mc_read_qe(rt_device_t adc2_dev, mc_input_signals_t *input);
