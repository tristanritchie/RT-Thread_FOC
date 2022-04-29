/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

typedef struct mc_pi_controller_t
{
    float kp;
    float ki;
    float kc;
    float out_max;
    float out_min;
    float d_sum;
    float in_ref;
    float in_meas;
    float out;
} mc_pi_controller_t;


void mc_pi_control(mc_pi_controller_t *const param);
void mc_reset_controller(mc_pi_controller_t *const param);
