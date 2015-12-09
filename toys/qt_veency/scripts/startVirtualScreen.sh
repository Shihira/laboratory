#!/bin/bash
xrandr --newmode "1024x768_60.00"   63.50  1024 1072 1176 1328  768 771 775 798 -hsync +vsync
xrandr --addmode VIRTUAL1 1024x768_60.00
xrandr --output VIRTUAL1 --mode 1024x768_60.00 --right-of eDP1

