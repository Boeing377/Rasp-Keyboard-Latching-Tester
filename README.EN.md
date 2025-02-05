# Preparation
1. Ensure that you have had a Rasp-Keyboard-Latency-Tester hardware and install it correctly. [instruction](Hardware/README.EN.md).
2. Compile the Rasp-Keyboard-Latency-Tester on your Raspberry Pi. [instruction](Software/README.EN.md).
# Usage
1. Move to the directory where the software executable program is located.
2. Run the program
    ```bash
    ./keyboard_latency_test
    ```
3. Follow the instruction on the screen, test your keyboard.
## Parameter 
| Prameter |    Value    | Description                                                           |
| :------: | :---------: | :-------------------------------------------------------------------- |
|    -a    |    none     | Resistive Load Regulation Mode                                        |
|    -b    | millisecond | Set the interval between each test                                    |
|    -h    |    none     | Displays help information, listing all available commands and options |
|    -l    |    none     | Testing with a resistive load                                         |
|    -n    |   number    | Set the number of tests                                               |
|    -s    |  filename   | Save test results (file name optional)                                |
|    -t    | millisecond | Set the trigger duration for each test                                |
|    -v    |    none     | Displays the current version number                                   |
### Example
The following command will execute a test while the trigger duration set to 50ms and the test interval set to 100ms. The test results saved as test_result file.

```bash
./keyboard_latency_test -t 50 -b 100 -s test_result
```
