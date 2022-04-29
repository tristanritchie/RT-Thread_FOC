# RT-Thread_FOC
 FOC implementation and interrupt latency tests for RT-Thread

 ## Contents

5 RT-Thread Studio projects for the STM32F429ZI:
- rt_foc: field-oriented control implemented in the ADC ISR using RT-Thread
- rt_semaphore_latency: latency measurement using RT-Thread semaphore
- rt_schedule_latency_1: latency measurement using RT-Thread manual scheduling (suspend/resume)
- rt_schedule_latency_2: latency measurement using modified RT-Thread schedule function to remove unused overhead when switching from ISR
- custom_schedule_latency: rewritting RT-Thread scheduler to greatly accelerate context switching from ISR


## Perliminary Results

- rt_foc: not applicable
- rt_semaphore_latency: significant latency approx 11us
- rt_schedule_latency_1: improved response approx 6us
- rt_schedule_latency_2: small improvement approx 5us
- custom_schedule_latency: big improvement approx 2.5us, not currently stable and crashes after 10 seconds...
