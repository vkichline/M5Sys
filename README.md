# M5Sys - An M5Stack-Based Computer System

This project combines a few libraries with a collection of apps (some borrowed, some original) to create
a small, usable computer system based on the excellent M5Stack Fire ESP32-based micro.

During isolation, I discovered two libraries that transformed my usage of the M5Stack; the first was [M5Stack-SD-Updater](https://github.com/tobozo/m5stack-sd-updater),
which allows the user to load and execute programs from the SD card. Only a few lines of code need to be added to each program to make the system work cooperatively.
This permitted using the M5Stack as a computer system, rather than as a device dedicated to a single program, which turns out to be a significant change.

The second library was [M5ez](https://github.com/ropg/M5ez), which provides a simple but very usable UI for M5Stack programs. It doesn't solve every problem,
but it provides a critical mass of functionality that reduces the effort of writing utility programs substantially. Since I now had a portable system with loadable
software, it made sense to invest some time in developing utility programs that were more than knock-offs for one-time use. I can use M5ez to build simple
but reliable utilities I can use for developing other software, and the SD card makes them all available when needed.

On top of these libraries, I built a thin veneer; M5Sys (actually, two versions: one with and one without M5ez, because not everything needed custom UI.) I added some
functionality I really wanted, such as more specific Wifi connection settings, system-adjustable logging, both to serial and SD (or to SPIFFS, or the screen) and a
system of storing locations (latitude, longitude and altitude.) Not all the programs included with be of general interest at this point. AstroClock, for example,
needs to attach to a Raspberry Pi program I haven't published which calculates astronomical events.  I will include more info on the included programs at a later date.
