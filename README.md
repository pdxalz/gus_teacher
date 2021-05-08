# GUS Teacher 

### Overview

GUS is a "Game for Understanding Social distancing".  The GUS Teacher project is the sourcecode for the teacher's teacher's device used to control the student GUS Badges.

### Requirements

This project uses the [adafruit_2_8_tft_touch_v2](https://docs.zephyrproject.org/latest/boards/shields/adafruit_2_8_tft_touch_v2/doc/index.html) display, and the following board:

- nrf5340dk_nrf5340_cpuapp 

NOTE: The current version of the nrf5340 board files in Zephyr does not fully support the Arduino definitions required by the adafruit display. In order to correct this a patch is included, which can be applied as follows:
  1) Move the nrf5340_display.patch file from this repo into your Zephyr base directory
  2) From the Unix bash (or Git command line in Windows) run *"git apply nrf5340_display.patch"*



