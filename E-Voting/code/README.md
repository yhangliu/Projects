# Code Readme

This leader election algorithm is based on Bully Algorithm, and it is consisted of the following components:
- [main.c](main/main.c) / [main.h](main/main.h)
  - timer_evt_task() function triggers appropriate events based on the timer.
  - onboard_led_task() function controls the onboard led to indicate the current state.
    - on: leader state
    - off: non-leader state
    - blink: election in progress
- [udp.c](main/udp.c) / [udp.h](main/udp.h)
  - udp_client_task() function sends udp messages to other esp32 devices using udp. It determines the message contents based on the events triggered.
  - udp_server_task() function receives udp messages from other esp32 devices using udp. Based on the received messages, it determines (1) if the leader is still alive, (2) if there's other devices with lower myID in election state.


The electric voting is done using ir communications, and implemented in the following components
- [ir.c](main/ir.c) / [ir.h](main/ir.h)
  - recv_task() function continuously receives IR signals from other fobs. If the payload is valid, then update the led color status based on the content.
  - led_task() function controls the leds based on the color status.
  - id_task() function indicates id status using the onboard led.
  - button_task() function controls two buttons. The button connected to pin 4 sends UART signal to other fobs. The button connected to pin 36 cycles through the led color.

Raspberry Pi runs the webserver and database server to keep track of voting counts and show the realtime counts on the graph.
- [data_logger.js](nodejs/data_logger.js)
   - Using UDP, it receives the voting info from the leader fob over wifi.
   - It stores voting counts info in the mongo database.
- [data_plotter.js](nodejs/data_plotter.js)
   - Reads vote counts data from mongodb, and display real-time plots using CanvasJS. The plots updates itself every 2 seconds to show up-to-date information. 
- [index.html](nodejs/index.html)
   - Design of web application for remote clients.
   - A button to reset the database. 
   - Displays the real-time plots of the vote counts.