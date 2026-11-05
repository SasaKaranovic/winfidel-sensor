# How to build firmware

For our firmware development, we are going to use Arduino. Mainly because it's user friendly, easy to use and
most hobbyists are familiar with it or have already used it. While we could build the same or even more efficient
firmware using bare-metal C and ESP-IDF, we are going to stick with Arduino and hope that this allows the project to be
more friendly and easier to use/modify by wider DIY/hacker community.

## How can I build/compile firmware?

### 1. Install PlatformIO
Installing PlatformIO CLI is pretty straight-forward and also well documented for Windows, Linux and MacOS.
You will need to follow few steps and get PlatformIO CLI installed, detailed tutorial can be found at https://platformio.org/install/cli
Make sure to install [PlatformIO Core](https://docs.platformio.org/en/latest//core/installation.html#installation-methods 'https://docs.platformio.org/en/latest//core/installation.html#installation-methods') and also that it is available trough [shell](https://docs.platformio.org/en/latest//core/installation.html#piocore-install-shell-commands 'PlatformIO Core - Install Shell Commands¶').

### 2. Build firmware
Open shell/command-prompt and navigate to 'Firmware/' folder.

1. Compile and upload the firmware with `pio run --target upload --upload-port <COM-PORT>`. Make sure to replace `<COM-PORT>` with your ESP32's COM port (ie COM1 or /dev/ttyACM0)
2. Restart/power-cycle your board

Every time you make a change to firmware or the web-page, you need to recompile and upload new firmware (run step #1).


## Online flasher

Each new firmware release gets automatically compiled by the automated build system and final compiled binaries get uploaded to Web flasher tool that is available at https://docs.sasakaranovic.com/firmware/winfidel/


[<- Go back to repository root](../README.md)
