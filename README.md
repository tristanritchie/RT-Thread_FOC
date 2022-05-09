# RT-Thread_FOC
 Field-oriented control implemented over RT-Thread for STM32F429ZI. 

## Contents

Field-oriented control implemented over RT-Thread. Seperate branches are available to test interrupt latency.

STM32F4 BSP Extention:
    - config for centre-aligned PWM complementary output
    - config for ADC injected-trigger on PWM mid-point
    - callback register for ADC injected conversion complete interrupt (low latency)

Kernel Extention:
    - fast direct context switching from ISR

## Usage

mc_user_definitions.c contains all configurations for motor, ADC, control loop, (etc), and control method parameters

