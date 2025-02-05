# Description
The Rasp-Keyboard-Latching-Tester is a tool designed to test keyboard input latency on a Raspberry Pi. It utilizes the GPIO pins of the Raspberry Pi to trigger keyboard actions and leverages Linux's input event system to capture keyboard events. By measuring the timestamp differences between events, the software calculates the keyboard's response delay, providing insights into its performance and latency.
# Compile
## Prepare
1. Requirements
   - gcc 
   - make 
   - gpiod
2. Install requirments
    ```bash
    apt update
    apt install gcc make libgpiod-dev
    ```
3. Make sure you have install all requirments
    ```bash
    gcc --version
    make --version
    dpkg -l | grep libgpiod-dev
    ```
## Build
1. Move to the folder where the project is located, which is the folder where this file is located.
2. Run the following command to compile the project:
    ```bash
    make
    ```
    This will execute the build process using the Makefile.
## Check the Results
- Once the compilation completes successfully, you should see the output file in the project directory.
- To verify the build, you can run the resulting program:
  ```bash
  ./keyboard_latency_test
  ```
  If the program runs without errors, the build was successful.
# Useage
The software supports two operating modes:

1. Direct Measurement Mode
   
    In this mode, the GPIO pin triggers a signal that directly closes an analog switch, shorting the keyboard switch to simulate a key press. This method is suitable for most keyboards, including mechanical and magnetic switch keyboards.

2. Measurement with Resistor Mode
   
   In this mode, a relay adds a resistor in series with the analog switch. When the GPIO signal is triggered, it does not short the switch directly but instead parallels a resistor with the switch. This mode is particularly useful for testing special features of magnetic switch keyboards.

## Parameter Description
|Prameter|   Value   |Description|
|:------:|:---------:|:----------|
|   -a   |    none   |Resistive Load Regulation Mode.|
|   -b   |microsecond|Set the interval between each test (in milliseconds).|
|   -h   |    none   |Displays help information, listing all available commands and options.|
|   -l   |    none   |Testing with a resistive load.|
|   -n   |   number  |Set the number of tests.|
|   -s   |  filename |Save test results (file name optional).|
|   -t   |mircosecond|Set the trigger duration (in milliseconds) for each test.|
|   -v   |    none   |Displays the current version number.|

## Demo
### Direct Measurement Mode
The following command will execute a test all default

```bash
./keyboard_latency_test
```

The following command will execute a test with the trigger duration set to 50ms, the test interval set to 100ms, and the test results saved as test_result file.

```bash
./keyboard_latency_test -t 50 -b 100 -s test_result
```

### Measurement with Resistor Mode

#### 