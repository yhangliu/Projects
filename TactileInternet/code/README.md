# Code Readme

We used C and javascript to develop this project.
- [main.c](main/main.c) / [main.h](main/main.h)  
  This code is for ESP32, and it reads raw voltages from three sensors (i.e., thermistor, ultrasonic sensor, and ir rangefinder) and converts them to the appropriate engineering units (i.e., meter and celcius). After the conversion, the program sends them to the serial port in order for Node.JS program to read sensor values.
- [data_logger.js](nodejs/data_logger.js)  
  This code read sensor values that main programs worte into serial port. Once values are readed, it parse the output line by line to create a json format datafile. In order for the data confliction, the program delete datafile on the startup so that new data won't mix with the old datafile.
- [data_plotter.js](nodejs/data_plotter.js)  
  This code defines the website tree structure. In specific, our website has root directory ('/'), where it calls index.html to plot sensor values, and data directory ('/data'), where it reads sensor data from datafile.json.
- [index.html](nodejs/index.html)  
  This code **dynamically** plots the measured sensor values nicely using CanvasJS. The plot shows 100 data points at a time, and update the graph every 2 seconds. This way, the graph always show the up-to-date information to users.