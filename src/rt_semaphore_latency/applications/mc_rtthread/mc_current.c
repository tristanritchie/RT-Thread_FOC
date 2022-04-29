/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#include "mc_current.h"

void mc_adc_offset_calibration(void)
{

}

float mc_adc_count_to_volt(rt_uint32_t res)
{
    return (3.3/4095)*res;
}

rt_uint32_t mc_read_adc(rt_adc_device_t adc_dev)
{
    return 0;//HAL_ADCEx_InjectedGetValue(adc_dev->parent.user_data, 1);
}

void mc_read_currents(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev, mc_input_signals_t *input)
{
    input->ia = mc_adc_count_to_volt(mc_read_adc(adc1_dev));
    input->ib = mc_adc_count_to_volt(mc_read_adc(adc2_dev));
    input->ic = - input->ia - input->ib;

}
