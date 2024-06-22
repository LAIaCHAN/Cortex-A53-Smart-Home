# Cortex-A53-Smart-Home
Smart Home System Based on Cortex-A53
Operating System: Ubuntu 18.04  
Hardware Platform: IMX8MM Development Board  
Development Tool: aarch64-poky-linux-gcc

- The driver layer initializes the light intensity sensor, DHT11 temperature and humidity sensor, flame sensor, LCD display, LED lights, and buzzer through the GPIO interface. It then obtains data from each sensor via I2C and UART communication protocols and passes the read data to the application layer through the ioctl interface. Finally, the driver layer controls the LED lights and buzzer by applying for a character device and GPIO pins to achieve the purpose of alerts, and it drives the LCD display to detect environmental information.
- The application layer reads light intensity data through file IO read/write functions and conditional judgment to achieve intelligent control of the lights, thereby conserving energy. Additionally, the buzzer is controlled based on the flame sensor data to serve as an alarm.
