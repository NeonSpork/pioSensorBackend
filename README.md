# PlatformIO GPIO Backend
![Default neon sign logo](https://raw.githubusercontent.com/NeonSpork/ngGetBeer/main/src/assets/images/craggs_lounge.svg)

Backend part of a complete dispenser program for a custom kegerator.
[Frontend](https://github.com/NeonSpork/ngGetBeer) can be found here.
## API
API interface to use a frontend that can send HTTP requests that trigger interface with GPIO pins.

## Backend
Runs on a NodeMCU, use PlatformIO to install and flash this.

## First time logging onto Sensor Backend
Log on to `getBeer_WiFi_configuration` hotspot and choose the correct wifi. *Make sure you choose the same WiFi as your frontend*.  

Once the backend is logged on, go to your router and reserve the IP `192.168.10.99` (you may need to change this to `192.168.0.99` or another IP of your choosing in the frontend, depending on your router). Alternatively you can reserve whatever IP the _SensorBackend_ received, and update the frontend accordingly.
