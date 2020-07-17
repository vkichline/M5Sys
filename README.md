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
system of storing locations (latitude, longitude and altitude.) Not all the programs included with be of general interest at this point.

## Installation

To install M5Sys on an M5Stack Fire, open the project in VS Code with PlatformIO installed.
Run the build script `build-fire` on Macintosh, or `fire.ps1` on Windows.
You should see the SD directory fill with .bin files as they build, 17 in all. Once built, look at the prop script
(`prop` for Macintosh, `prop.ps1` for Windows) and adjust the appropriate one to copy to an SD card on your system,
or manually copy all files in SD to an SD card and insert it into the M5Stack.

Next, build and flash the program `menu`. This will allow your system to load any of the other programs on SD. To return to the
`menu` program, hold the first of three buttons on the M5Stack (the 'A' button) and press the red reset button, then release 'A'.

## Programs

### Astroclock

Unfortunately, this only runs in my house. Connects to my Raspberry Pi wich is running a service to monitor GPS and calculate astronomical
information for its current location. Draws a clock face wich indicates sun and moon rise and set. I'll publish AstroPi eventually.

### ClickBang

A game I wrote durring lockdown. Click the button in the column below the non-matching brick. Bonuses for speed and consistency.

### Clock

Uses Rop Gonggrijp's ezTime to render a clock for the any USA timezone, in various color schemes with different clock faces.

### Distance

Calculates the distance in miles between two locations (latitude and longitude) using a location setting as the base.

### FileManager

A collection of tools for managing files on the SD card and in SPIFFS. One handy tools deletes all the hidden files that the
Macintosh file system creates and are of no use on the SD card.

### HomeControl

Another program from my house only. A network of ESP8266 sensors and controllers talk to another Rapsberry Pi ti build an
information map using MQTT. This program displays sensor and controller status and lets me turn controllers on and off.

### IChing

An ancient Chinese work of phylosophy and and oracle, can be used to predict the future with the accuracy of a Magic 8 Ball,
or for study of the wisdom of King Wen and Confusious.

### ImuMonitor

Displays data from the built in IMU.

### Life

Conway's Game of Life, with pause, resume, restart and stats.

### mDNSMonitor

Displays Avahii registrations on the chosen WiFi network.

### menu

Provides UI for all settings, displays system info, and allows seletion of any installed program to run.
Settings are persistant and can be read and used in all other programs on the system.
Also controls logging, including logging to SD card.

### MQTTMonitor

Displays all the MQTT traffic on the slected network.

### PacketMonitor

WiFi packet sniffer. Can capture packets to SD card.

### SoundMonitor

VU meter for sound levels/frequencies.

### SoundRecorder

Records sounds and plays them back. (Doesn't write to disk yet.)

### Weather

Finds NWS weather station for selected location and makes full forecasts and hourly forecasts available.
Pretty basic at this point; will be a nice graphic app at some point.

### WiFiScanner

Scans WiFi networks and displays info about them.
