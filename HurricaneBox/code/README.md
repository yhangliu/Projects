# Code Readme
This code is broken down into major components.
First component is responsible for ESP32, and the second component is responsible for Raspberry Pi. Each components contains several modules that representing one of the pieces of the complete solution. 

[ESP32 Components](./main)
1. main.c, main.h
   - Main function for the ESP32.
   - Initialize necessary modules and calls all other functions for the accelerometer, thermistor, and LED. 
   
2. accelerometer.c, accelerometer.h
   - Interprets changes measured by the ADXL343 accelerometer module and translates it into human readable units for the x, y, and z axes. 
   - Calculates pitch and roll information using the 3 axes.
   
3. ledc.c, ledc.h
   - Utilizes the ESP-IDF's LED Control module to power the LED. 
   - Setting different duty level allows the user to set the intensity of the LED. 
   
4. thermistor.c, thermistor.h
   - Responsible for translating voltage readings from the Thermistor module and translating it into usable engineering units (i.e., Celcius).
  
5. udp.c, udp.h
   - Responsible for establishing connection from ESP32 board to the local network.
   - Communicates the data collected from the sensors wirelessly over wifi using UDP protocol.

[Raspberry Pi Components](./nodejs)
1. data_logger.js
   - Using UDP, it receives all the sensor readings from ESP32 over wifi. 
   - Formats the received UDP messages into json format, and stores them into datafile.json.
2. data_plotter.js
   - Reads json data from datafile.json, and display real-time plots of each sensor modules using CanvasJS. The plots updates itself every 2 seconds to show up-to-date information. 
   - Send the user input (LED Intensity from 0 to 9) to ESP32 using UDP over wifi to dynamically control the intensity of LED on the remote ESP32.
3. index.html
   - Design of web application for remote clients.
   - Displays real-time web camera video streaming from Raspberry Pi.
   - Dropdown menu to receive user input for LED intensity. 
   - Displays the real-time plots of thermistor, accelerometer, and tilt data.
