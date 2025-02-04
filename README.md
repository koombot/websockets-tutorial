## Websockets Tutorial Work Folder.

Working from: https://m1cr0lab-esp32.github.io/remote-control-with-websocket/  
This tutorial covers creating a simple bit of code that uses websockets to interact with an LED on an ESP32. I am modifying the code slightly from the tutorial because I want to include additional features (such as outputting some information to an OLED display).  
I also want to adjust some of the code to match my experience with the Arduino framework, making it easier to understand and integrate.

### Modifying the LED Struct

One of the modifications I am working on is to make the **LED struct** more object-oriented. I will:
- Create a **constructor** for the `LED` object.
- The constructor will **initialise the pin mode** as an output pin.
- Allow the LED object to **default to either "on" or "off"** when initialised, making the code more flexible.  If not specified, will default to off.

04.02.2024
Created a new branch to look into the button logic.
-establish if the logic can be made easier to read whilst retaining the functionality to distinguish between pressed, held and released
-add a constructor to initialise the pin so I can remove the pin from setup.
-the logic of buttons is HARD.  I'm giving up for now.  I will return to this at a later date.  The way the button is programmed is quite clever as the state counter only lets it register a push once.  The held state is useful for holding a button but doesn't really lend itself to extra functionality.

Added SSD2306 support and the web UI elemnts
