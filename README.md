# RT-Thread_FOC
 FOC implementation and interrupt latency tests for RT-Thread

## Contents

Field-oriented control implemented over RT-Thread. Seperate branches are available to test interrupt latency.


## Perliminary Results

Checkout the branch corresponding to the test:
- rt_foc: FOC implemented in ISR, latency not applicable
- latency_test_semaphore: significant latency approx 11us
- latency_test_suspend_resume: improved response approx 6us
- latency_test_fast_schedule: small improvement approx 5us
- latency_test_custom_schedule: big improvement approx 2.5us, not currently stable (stack overflow)
