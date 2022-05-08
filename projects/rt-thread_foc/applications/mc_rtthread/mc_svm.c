/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */
#include "mc_svm.h"
#include "mc_transform.h"
#include "mc_current.h"

void mc_svpwm_time_calc(mc_svpwm_t * const svm)
{
    svm->t1 = (svm->period) * svm->t1;
    svm->t2 = (svm->period) * svm->t2;
    svm->tc = (svm->period - svm->t1 - svm->t2)/2;
    svm->tb = svm->tc + svm->t2;
    svm->ta = svm->tb + svm->t1;
}

void mc_svpwm_gen(const mc_clarke_tansform_t *clarke, mc_svpwm_t *svm)
{
    svm->vr1 = clarke->beta;
    svm->vr2 = (-clarke->beta/2 + SQRT3_BY2 * clarke->alpha);
    svm->vr3 = (-clarke->beta/2 - SQRT3_BY2 * clarke->alpha);

      if( svm->vr1 >= 0 )
      {
            // (xx1)
            if( svm->vr2 >= 0 )
            {
                  // (x11)
                  // Must be Sector 3 since Sector 7 not allowed
                  // Sector 3: (0,1,1)  0-60 degrees
                  svm->t1 = svm->vr2;
                  svm->t2 = svm->vr1;
                  mc_svpwm_time_calc(svm);
                  svm->pwm1 = (uint32_t)svm->ta;
                  svm->pwm2 = (uint32_t)svm->tb;
                  svm->pwm3 = (uint32_t)svm->tc;
            }
            else
            {
                  // (x01)
                  if( svm->vr3 >= 0 )
                  {
                        // Sector 5: (1,0,1)  120-180 degrees
                        svm->t1 = svm->vr1;
                        svm->t2 = svm->vr3;
                        mc_svpwm_time_calc(svm);
                        svm->pwm1 = (uint32_t)svm->tc;
                        svm->pwm2 = (uint32_t)svm->ta;
                        svm->pwm3 = (uint32_t)svm->tb;
                  }
                  else
                  {
                        // Sector 1: (0,0,1)  60-120 degrees
                        svm->t1 = -svm->vr2;
                        svm->t2 = -svm->vr3;
                        mc_svpwm_time_calc(svm);
                        svm->pwm1 = (uint32_t)svm->tb;
                        svm->pwm2 = (uint32_t)svm->ta;
                        svm->pwm3 = (uint32_t)svm->tc;
                  }
           }
      }
      else
      {
            // (xx0)
            if( svm->vr2 >= 0 )
            {
                  // (x10)
                  if( svm->vr3 >= 0 )
                  {
                        // Sector 6: (1,1,0)  240-300 degrees
                        svm->t1 = svm->vr3;
                        svm->t2 = svm->vr2;
                        mc_svpwm_time_calc(svm);
                        svm->pwm1 = (uint32_t)svm->tb;
                        svm->pwm2 = (uint32_t)svm->tc;
                        svm->pwm3 = (uint32_t)svm->ta;
                  }
                  else
                  {
                        // Sector 2: (0,1,0)  300-0 degrees
                        svm->t1 = -svm->vr3;
                        svm->t2 = -svm->vr1;
                        mc_svpwm_time_calc(svm);
                        svm->pwm1 = (uint32_t)svm->ta;
                        svm->pwm2 = (uint32_t)svm->tc;
                        svm->pwm3 = (uint32_t)svm->tb;
                  }
            }
            else
            {
                  // (x00)
                  // Must be Sector 4 since Sector 0 not allowed
                  // Sector 4: (1,0,0)  180-240 degrees
                  svm->t1 = -svm->vr1;
                  svm->t2 = -svm->vr2;
                  mc_svpwm_time_calc(svm);
                  svm->pwm1 = (uint32_t)svm->tc;
                  svm->pwm2 = (uint32_t)svm->tb;
                  svm->pwm3 = (uint32_t)svm->ta;
            }
      }
}


