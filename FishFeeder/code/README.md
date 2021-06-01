# Code Readme

The code comprised from four files. Each file implement independent modules, making our code easy to read and maintain. Also, because of its modularity, our code is extensible for future challenges.
Specifically we have the following modules:
1. Timer module: Keep track of feeding time interval  
   - Implemented in display.c/.h  
2. Servo module: Rotate back and forth three times to feed fish  
   - Implemented in servo.c/.h  
3. Display module: Display the remaining time until the next feeding session  
   - Implemented in display.c/.h  
4. Helper module: Contains useful functions used by other modules  
   - Implemented in helper.c/.h  

# Acknowledgement
- [ESP-IDF Official GitHub Repo](https://github.com/espressif/esp-idf)
- [Values for the 14-Segment Display](https://github.com/dmadison/LED-Segment-ASCII/blob/master/14-Segment/14-Segment-ASCII_BIN.txt)