# \# STM32 Motor Health Monitor

# 

# A microcontroller-based project for motor control and condition monitoring using the STM32F103 platform.

# 

# The project aims to combine motor control, sensor data acquisition, and real-time monitoring through a serial terminal. It is being developed incrementally, with a focus on embedded systems fundamentals, peripheral configuration, and modular firmware design.

# 

# \## Project Overview

# 

# The STM32 Motor Health Monitor is designed as a learning and development project for embedded motor monitoring.

# 

# The system is intended to support:

# 

# \- Motor control using an STM32 microcontroller.

# \- Serial communication for monitoring and debugging.

# \- Distance measurement using an HC-SR04 ultrasonic sensor.

# \- Future monitoring of motor current, vibration, temperature, and rotational speed.

# 

# \## Hardware

# 

# | Component | Purpose | Status |

# |---|---|---|

# | STM32F103 | Main microcontroller | In use |

# | DC motor and driver | Motor control | Implemented in M10 |

# | HC-SR04 | Ultrasonic distance measurement | In development |

# | Current sensor | Motor current monitoring | Planned |

# | MPU6050 | Vibration and acceleration monitoring | Planned |

# | Temperature sensor | Motor temperature monitoring | Planned |

# | Hall sensor or encoder | Motor speed measurement | Planned |

# 

# \*The exact motor model, driver, and sensor specifications depend on the hardware configuration used for testing.\*

# 

# \## Software and Tools

# 

# \- \*\*Firmware:\*\* Embedded C

# \- \*\*Microcontroller:\*\* STM32F103

# \- \*\*IDE:\*\* Keil MDK-ARM

# \- \*\*Configuration:\*\* STM32CubeMX

# \- \*\*Serial terminal:\*\* PuTTY

# \- \*\*Version control:\*\* Git and GitHub

# 

# \## Current Features

# 

# \### 1. Motor Control

# 

# The initial motor-control milestone (M10) has been implemented and tested on the target hardware.

# 

# \### 2. Serial Communication

# 

# UART communication is used to display information and interact with the firmware through a serial terminal such as PuTTY.

# 

# \### 3. Ultrasonic Distance Measurement

# 

# HC-SR04 integration is being developed to measure the distance to nearby objects.

# 

# The implementation uses timer input capture to measure the echo pulse duration and calculate distance.

# 

# \## Development Roadmap

# 

# \- \[x] Implement and test the initial motor-control milestone (M10).

# \- \[x] Establish serial-terminal monitoring using PuTTY.

# \- \[ ] Complete HC-SR04 integration and validate distance readings.

# \- \[ ] Add motor current measurement.

# \- \[ ] Add vibration monitoring using an accelerometer.

# \- \[ ] Add motor temperature monitoring.

# \- \[ ] Add rotational speed measurement.

# \- \[ ] Develop threshold-based motor condition warnings.

# \- \[ ] Integrate sensor readings into a unified monitoring interface.

# \- \[ ] Perform system-level testing under different operating conditions.

# 

# \*Roadmap items are subject to change as the project evolves.\*

# 

# \## Repository Structure

# 

# ```text

# stm32-motor-health-monitor/

# └── firmware/

# &#x20;   └── motor\_monitor/

# &#x20;       ├── Core/

# &#x20;       │   ├── Inc/

# &#x20;       │   └── Src/

# &#x20;       ├── Drivers/

# &#x20;       │   ├── CMSIS/

# &#x20;       │   ├── STM32F1xx\_HAL\_Driver/

# &#x20;       │   └── HCSR04/

# &#x20;       ├── Services/

# &#x20;       ├── MDK-ARM/

# &#x20;       └── motor\_monitor.ioc

# ```

# 

# The structure above highlights the main firmware directories. Additional project files and generated configuration files may be present.

# 

# \## Getting Started

# 

# \### Prerequisites

# 

# Before building the firmware, install:

# 

# 1\. Keil MDK-ARM with the required STM32 device support.

# 2\. STM32CubeMX, if peripheral configuration needs to be reviewed or regenerated.

# 3\. The appropriate ST-Link or other supported programming interface.

# 4\. PuTTY or another serial terminal.

# 

# \### Build and Flash

# 

# 1\. Clone the repository.

# 2\. Open the Keil project file under `firmware/motor\_monitor/MDK-ARM/`.

# 3\. Check the target device, clock configuration, and peripheral settings.

# 4\. Build the project and resolve any compiler errors.

# 5\. Connect the STM32 board and flash the firmware.

# 6\. Open PuTTY with the configured serial port and UART settings.

# 7\. Verify the motor-control behavior and test available monitoring features.

# 

# \*UART settings and hardware connections must match the actual firmware and board configuration.\*

# 

# \## Development Notes

# 

# \- Keep existing motor-control functionality working when adding new sensors.

# \- Verify timer assignments and interrupt handlers before integrating additional peripherals.

# \- Validate sensor readings against known reference conditions.

# \- Document hardware wiring, configuration changes, and test results.

# \- Use separate feature branches for independent development tasks.

# 

# \## Safety

# 

# \- Disconnect power before changing motor-driver or sensor wiring.

# \- Verify voltage levels and current ratings before connecting hardware.

# \- Ensure sensor outputs are compatible with the STM32 input voltage limits.

# \- Use an appropriate driver stage rather than powering a motor directly from a microcontroller GPIO.

# \- Secure rotating components during testing.

# 

# \## Future Goals

# 

# The long-term goal is to build a modular embedded monitoring system capable of collecting multiple motor-related measurements and identifying abnormal operating conditions through configurable thresholds.

# 

# Future development may include data logging, improved fault detection, and a more user-friendly monitoring interface.

# 

# \## License

# 

# No license has been specified yet. Check the repository for licensing information before reusing or redistributing this project.

# 

# \---

# 

# \*\*Project status:\*\* Active development  

# \*\*Platform:\*\* STM32F103  

# \*\*Primary focus:\*\* Motor control and embedded condition monitoring

