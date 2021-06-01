# Code Readme

- [main.c/main.h](main/main.c)
  - Controls the ESC Servo to maintain the speed of 0.1-0.4m/s.
  - Controls the steering servo to keep a certain distance from the wall. (In the tech demo, we set it to 5cm ~ 10cm).
  - Measures the distance to the wall in the front using the ultrasonic range sensor. If the wall is within 20cm, the car stops to avoid the collision.
  - Measures the distance to the side wall using the IR Range Finder. 
- [udp.c/udp.h](main/udp.c)
  - Receives the udp request from the smartphone. If receives '1', then start the buggy. Otherwise, it locks the buggy and prevent it from starting accidentally.
- [display.c/display.h](main/display.c)
  - Controls the Alphanumeric display to show the current speed of the buggy in m/s.
- [helper.c/helper.h](main/helper.c)
  - Defines helper functions for display module.
- [backend.js](nodejs/backend.js)
  - Takes user input command (START / STOP) and send it to ESP32 using UDP.
- [index.html](nodejs/index.html)
  - Defines the basic web GUI.
