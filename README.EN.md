# Preparation
1. Ensure that you have had a Rasp-Keyboard-Latching-Terser hardware and install it as the [instruction](Hardware/README.EN.md) say.
2. Compile the Rasp-Keyboard-Latching-Tester on your Raspberry Pi follow thr [instruction](Software/README.EN.md).
# Useage
1. Move to the directory where the software executable program is located.
2. Run the program with follow command
    ```bash
    ./keyboard_latency_test
    ```
3. Follow the instruction on the screen, test your keyboard.
## Parameter Description
|Prameter|   Value   |Description|
|:------:|:---------:|:----------|
|   -a   |    none   |Resistive Load Regulation Mode.|
|   -b   |microsecond|Set the interval between each test (in milliseconds)|
|   -h   |    none   |Displays help information, listing all available commands and options.|
|   -l   |    none   |Testing with a resistive load.|
|   -n   |   number  |Set the number of tests|
|   -s   |  filename |Save test results (file name optional).|
|   -t   |mircosecond|Set the trigger duration (in milliseconds) for each test.|
|   -v   |    none   |Displays the current version number.|
### Demo of useage with parameter
The following command will execute a test with the trigger duration set to 50ms, the test interval set to 100ms, and the test results saved as test_result file.

```bash
./keyboard_latency_test -t 50 -b 100 -s test_result
```


