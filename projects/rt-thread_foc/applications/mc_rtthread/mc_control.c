/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */


#include "mc_control.h"



void mc_pi_control(mc_pi_controller_t *const param)
{
    float err;
    float out;
    float exc;

    err  = param->in_ref - param->in_meas;
    out  = param->d_sum + param->kp * err;

    /* Limit checking for PI output */
    if( out > param->out_max )
    {
       param->out = param->out_max;
    }
    else if( out < param->out_min )
    {
       param->out = param->out_min;
    }
    else
    {
       param->out = out;
    }

    exc = out - param->out;
    param->d_sum = param->d_sum + param->ki * err - param->kc * exc;
}


void mc_axis_decouple(mc_pi_controller_t *q_axis, mc_pi_controller_t *d_axis, float speed)
{
    /* Vd' = Vd - (Iq * Lq * We) */
    d_axis->out -= q_axis->in_meas * MOTOR_D_AXIS_INDUCTANCE * speed;
    /* Vq' = Vq + (((Id * Ld) + Ke) * We) */
    q_axis->out += ((d_axis->in_meas * MOTOR_Q_AXIS_INDUCTANCE) + MOTOR_FLUX_LINKAGE) * speed;

    return;
}


void mc_reset_controller(mc_pi_controller_t *const param)
{
    /**************** PI D Term ***********************************************/
    param->d_sum = 0;
    param->out = 0;
}
