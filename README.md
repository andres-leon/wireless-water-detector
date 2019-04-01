# Wireless, Battery-Powered, Water Detector
A water detector runs on LiPO batteries and transmits its status and notifications via MQTT to Home Assistance.

## About
A while back my indoor air conditioner air handler's condensation pipe clogged and caused a small flood. Thankfully we were home and so it didn't cause much damage. So instead of being sensible and getting a $15 water alarm from Home Depot, I decided to spend an inordinate amount of time, money, and effort into creating a water alarm that would motify me if my air conditioner decided to flood again. My version will not only yell at me but will also send me a notification to my Home Assistant which will them let me know via phone and email. Oh.. and it should run on a 2500 mAh LiPO battery for about 2439 hours or around 101 days and 15 hours (according to http://www.of-things.de/battery-life-calculator.php)

Here is the result.

## Pictures

## Hardware Needed
* ESP8266 12E or F (https://www.amazon.com/gp/product/B01M4IOFIT)
* ESP8266 Serial Port WiFi Module Adapter Plate (https://www.amazon.com/gp/product/B01NCQFSEG)
* FTDI Mini USB to TTL Serial Converter Adapter Module to program ESP (https://www.amazon.com/gp/product/B00IJXZQ7C/)
* Low Power Labs' TPL 5110 (https://lowpowerlab.com/shop/product/147?search=tpl5110) or Adafruit TPL5110 (https://www.adafruit.com/product/3435)
* Pololu Mini Pushbutton Power Switch with Reverse Voltage Protection (https://www.pololu.com/product/2808)
* JST Connectors (https://www.amazon.com/gp/product/B07449V33P)
* 5v DC Buzzer (https://www.amazon.com/gp/product/B0716FD838)
* 1 - 220k ohm resistor and 1 - 1m ohm resistor (To divide voltage from battery and calculate its capacity with the ESP's ADC pin.)
* 2 - 1 k ohm resistors
* 1- 150k to 170 k ohm resistor to control the TPL5110 delay. This one is needed if using Low Power Lab's TPL5110. Adafruit's TPL has an adjustable potentiometer to control the delay.
* 2 - BC337 NPN transistors 
* Rust-proof water probes (https://www.amazon.com/gp/product/B07L94MMP)

## Circuit Diagram
![alt text](schematic.png "Diagram")

## Soldering and Programming the Barebones Board
The following video shows how to solder the ESP to the plate and how to program it using the Arduino IDE: https://www.youtube.com/watch?v=O2SSyfP6OM0

## Code
The code can be found in the included ino file.

## Enclosure
I designed the enclosure using Fusion360. You can view and download it here https://a360.co/2OCKjhl
I printed it with a Prusa MK3 using PLA.(https://www.youtube.com/embed/VsiMdKhxB_E)



