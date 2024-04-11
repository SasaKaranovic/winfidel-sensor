# How to build firmware

For our firmware development, we are going to use Arduino. Mainly because it's user friendly, easy to use and
most hobbyists are familiar with it or have already used it. While we could build the same or even more efficient
firmware using bare-metal C and ESP-IDF, we are going to stick with Arduino and hope that this allows the project to be
more friendly and easier to use/modify by wider DIY/hacker community.

## How can I build/compile firmware?

### 1. Install PlatformIO
Installing PlatformIO CLI is pretty straight-forward and also well documented for Windows, Linux and MacOS.
You will need to follow few steps and get PlatformIO CLI installed, detailed tutorial can be found at https://platformio.org/install/cli
Make sure to install [PlatformIO Core](https://docs.platformio.org/en/latest//core/installation.html#installation-methods 'https://docs.platformio.org/en/latest//core/installation.html#installation-methods') and allso that it is available trough [shell](https://docs.platformio.org/en/latest//core/installation.html#piocore-install-shell-commands 'PlatformIO Core - Install Shell Commands¶').

### 2. Build firmware
Open shell/command-prompt and navigate to 'Firmware/platformio' folder.

1. Compile and upload the firmware with `pio run --target upload --upload-port <COM-PORT>`. Make sure to replace `<COM-PORT>` with your ESP32's COM port (ie COM1 or /dev/ttyACM0)
2. Upload the file system (Web page) with `pio run --target uploadfs --upload-port <COM-PORT>`, again replace `<COM-PORT>` with your ESP32's COM port.
3. Restart/power-cycle your board

Every time you make a firmware change, you need to run step #1.
Every time you make a change to the web page (anything inside `data` folder) you only need to run step #2.


[<- Go back to repository root](../README.md)
