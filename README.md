## Websockets Tutorial Work Folder.

Working from: https://m1cr0lab-esp32.github.io/remote-control-with-websocket/  
This tutorial covers creating a simple bit of code that uses websockets to interact with an LED on an ESP32. I am modifying the code slightly from the tutorial because I want to include additional features (such as outputting some information to an OLED display).  
I also want to adjust some of the code to match my experience with the Arduino framework, making it easier to understand and integrate.

### Modifying the LED Struct

One of the modifications I am working on is to make the **LED struct** more object-oriented. I will:
- Create a **constructor** for the `LED` object.
- The constructor will **initialise the pin mode** as an output pin.
- Allow the LED object to **default to either "on" or "off"** when initialised, making the code more flexible.  If not specified, will default to off.
