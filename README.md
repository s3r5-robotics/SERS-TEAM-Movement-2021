# S3R5 TEAM Movement

Code for controlling the movement of the robot. 

# Hardware

- Arduino Uno

## Software 

- C++
- PlatformIO

# Principle of working

The movement of the robot works by by following the right wall. If the right wall disappears it turns to the right. If a wall appears in front of the robot it turns to the left and keeps following the right wall. When a black tile is detected it goes back and turns for 90 degrees to the left and starts going forward until it finds a new wall.

