# wireless-water-detector
A water detector to prevent flooding that runs on LiPO batteries and transmits its status and notifications via MQTT.

## About
A while back my indoor air conditioner air handler's condensation pipe clogged and caused a small flood. Thankfully we were home and so it didn't cause much damage. So I diecided to spend an inordinate amount of time, money, and effort into creating a water alarm that would motify me if my air conditioner decided to flood again. Here is the result.

## Parts
* ESP8266 12E or F (https://www.amazon.com/gp/product/B01M4IOFIT)
* ESP8266 Serial Port WiFi Module Adapter Plate (https://www.amazon.com/gp/product/B01NCQFSEG)
* Low Power Labs' TPL 5110 (https://lowpowerlab.com/shop/product/147?search=tpl5110) or Adafruit TPL5110 (https://www.adafruit.com/product/3435)
* Pololu Mini Pushbutton Power Switch with Reverse Voltage Protection (https://www.pololu.com/product/2808)
* JST Connectors (https://www.amazon.com/gp/product/B07449V33P)
* 5v DC Buzzer (https://www.amazon.com/gp/product/B0716FD838)
* 1 - 220k ohm resistor
* 1 - 1m ohm resistor
* 2 - 1 k ohm resistors
* 1- 150k to 170 k ohm resistor to control the TPL5110 delay. This one is needed if using Low Power Lab's TPL5110. ADafruit's TPL has a POT that you can control and therefore the resistor is not needed
* 2 - BC337 NPN transistors 
