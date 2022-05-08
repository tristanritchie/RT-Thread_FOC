# RT-Thread_FOC
 Field-oriented control implemented over RT-Thread for STM32F429ZI. 

 ## Contents

Sensored field oriented control:
    - d/q axis PI control
    - speed PI control
    - dual-shunt current sensing
    - incremental quadrature encoder feedback for angle and speed measurements
    - forced rotor alignment

STM32F4 BSP Extention:
    - config for centre-aligned PWM complementary output
    - config for ADC injected-trigger on PWM mid-point
    - callback register for ADC injected conversion complete interrupt (low latency)

Kernel Extention:
    - fast direct context switching from ISR

## Usage

mc_user_definitions.c contains all configurations for motor, ADC, control loop, (etc), and control method parameters

## Perliminary Results

- rt_foc: not applicable
- rt_semaphore_latency: significant latency approx 11us
- rt_schedule_latency_1: improved response approx 6us
- rt_schedule_latency_2: small improvement approx 5us
- custom_schedule_latency: big improvement approx 2.5us, not currently stable and crashes after 10 seconds...
