Yalnix - Current Chungus
Uhuru Hashimoto
Elliot Potter
Oct 27, 2022

Here lies Yalnix, the Taker of Souls. We have entered the tomb and robbed the demon of Her sleep, seeking to suspend Her in animation forever.
Fortunately, our efforts were successful! For checkpoint 3, our kernel initializes page tables for region 0 and region 1, and returns to a
modified user process (state stored in idle_pcb) pointing to the DoIdle process in our kernel text. We then bounce between our DoIdle code
and Pause clock trap indefinitely.

Note: Makefile path to source is set to VBox and may need to be changed. If correct, make commands run as expected.

Usage: ./yalnix or ./yalnix -W

## Testing:

### Simply running the OS
    ```
    ./yalnix
    ```
    Will halt after attempting to load program "init", unless this is in the top-level directory of the project

### Simple alternation between Init and Idle; also prints the PID
    ```
    ./yalnix ./checkpoint_1/test_processes/iterator
    ```
    The idle process will print "DoIdle", while the iterator will print an incrementing value. These two prints alternate.

### Delay Test
    ```
    ./yalnix ./checkpoint_1/test_processes/delay_test
    ```

### Brk Test
    ```
    ./yalnix ./checkpoint_1/test_processes/brk_test
    ```
    Note -- to observe brk_test, stop the OS after about a second of execution.
    The idle process will print "DoIdle"; brk_test will malloc and then free some memory, which should trigger an increase
    in the brk of 3 pages.

### GetPID Test
