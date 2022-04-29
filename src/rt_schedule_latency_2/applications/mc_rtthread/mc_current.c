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



float _adc_count_to_volt(rt_uint32_t res)
{
    return (3.3/4095)*res;
}

rt_uint32_t _adc_read(rt_adc_device_t adc_dev, rt_uint32_t channel_rank)
{
    return  HAL_ADCEx_InjectedGetValue(adc_dev->parent.user_data, channel_rank);
}

void mc_read_currents(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev, mc_input_signals_t *input)
{
    rt_uint32_t result1, result2;
    result1 = _adc_read(adc1_dev, ADC1_CH_RANK);
    result2 = _adc_read(adc1_dev, ADC1_CH_RANK);

    input->ia = ADC_CURRENT_SCALE * (float)(result1 - input->a_offset);
    input->ib = ADC_CURRENT_SCALE * (float)(result2 - input->b_offset);

    input->ic = - input->ia - input->ib;
}

void mc_adc_offset_calibration(rt_adc_device_t adc_dev, rt_uint32_t channel, rt_uint32_t *offset)
{
    rt_uint32_t cumulative_offset = 0;
    rt_uint32_t count = 0;

    while (count < 20)
    {
        cumulative_offset += rt_adc_read(adc_dev, channel);
        count++;
    }

    *offset = cumulative_offset / 20;
}

