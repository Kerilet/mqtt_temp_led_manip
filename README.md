This project was made as a submission to the following challenge proposed by Professor Gustavo Ferreira Palma:

Based on the topics discussed on our classes, develop a firmware in C/C++ for the Raspberry Pi Pico 2 W capable of sending and receiving data using the MQTT protocol.
The firmware needs to have the following functionalities:

**1. Data Publishing**
- Use the built-in button (BOOTSEL) as an input device.
- Every time the button is pressed, the system will publish a message containing the current Raspberry Pi Pico 2 W CPU core temperature, in Celsius (ºC), on the following MQTT topic: "/your_name/your_srn/temperatura"

**2. Subscription and LED control**
- The firmware will need to subscribe on the following topic: "/your_name/your_srn/led"
- Every message on this topic needs to be validated:
  - If the content is a positive whole number, the value needs to be interpretated as an interval, in seconds, to configure the board's built-in LED's blink
  - If the content is 0 or anything besides a valid whole number, the timer needs to be interrupted and the LED needs to stay turned off.
