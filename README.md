# Smart Pomodoro Timer

This is the main source code file of my Pomodoro Timer (Embedded Systems Project).

## Overview

The `main.cpp` file contains the entry point of the program and serves as the starting point for the execution. It is responsible for initializing the necessary components, setting up timers, and managing the overall flow of the Pomodoro Timer.

## Functionality

The main functionality of the Pomodoro Timer is to implement the Pomodoro Technique, a time management method that uses a timer to break work into intervals, traditionally 25 minutes in length, separated by short breaks.

The `main.cpp` file includes the following features:

- Timer setup and configuration
- State management for different Pomodoro phases (work, short break, long break)
- User input handling
- Displaying the current phase and remaining time on an embedded display
- Sound notifications for phase changes

## Usage

To use the Pomodoro Timer, follow these steps:

1. Connect the embedded device to a power source.
2. Compile and upload the code to the device.
3. The timer will start automatically, displaying the work phase and remaining time on the display.
4. Follow the Pomodoro Technique by working for the specified time interval, taking short breaks, and longer breaks after a certain number of work intervals.
5. The timer will notify you with sound and display changes when transitioning between phases.

## Dependencies

The `main.cpp` file relies on the following libraries:

- [LedControl](https://github.com/wayoda/LedControl) version ^1.0.6
- [LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C) version ^1.1.4
- [NewPing](https://github.com/teckel12/NewPing) version ^1.9.7

Make sure to install these libraries before compiling the code.

## Contributing

Contributions to the Pomodoro Timer Embedded project are welcome. If you find any issues or have suggestions for improvements, please open an issue or submit a pull request on the [GitHub repository](https://github.com/ProgrammerJM/pomodoro-timer-embedded).
