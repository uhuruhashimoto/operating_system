Yalnix - Current Chungus
Uhuru Hashimoto
Elliot Potter
Oct 20, 2022

Here lies Yalnix, the Taker of Souls. We have entered the tomb and robbed the demon of Her sleep, seeking to suspend Her in animation forever. Fortunately, our efforts were successful! For checkpoint 1, our kernel initializes page tables for region 0 and region 1, and returns to a modified user process (state stored in idle_pcb) pointing to the DoIdle process in our kernel text. We then bounce between our DoIdle code and Pause clock trap indefinitely.

Note: Makefile path to source is set to VBox and may need to be changed. If correct, make commands run as expected.

Usage: ./yalnix or ./yalnix -W
