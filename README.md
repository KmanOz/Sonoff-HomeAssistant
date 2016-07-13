# Sonoff-HomeAssistant Description

Sonoff-HomeAssistant is alternative firmware for the brilliant & cheap ($$$ not quality) [ITEAD Sonoff](https://www.itead.cc/sonoff-wifi-wireless-switch.html) switch that allows you to use your own mqtt broker rather than the pre-defined 'ITEAD CLOUD' service that is shipped with the firmware in the unit. This would have to be one of the cheapest IoT switches available today. In fact, even if you knew how to build one, the components alone would cost you more that the whole unit, so why bother.

Is is designed to be installed with the Arduino IDE and has been tested on Arduino 1.6.9 but should be backwards compatible. I realize there are many versions of mqtt based firmware(s) that have been written for the Sonoff switch, but I found most of them overly complex for my liking.

This firmware is very basic but gets the job done. There is no RTC, no OTA firmware updates, no frills what so ever. In fact, I found that once the mqtt topic is set and the Sonoff has connected to your broker, you rarely need to make any modifications to the switch, especially after it's been installed in your home and configured in Home Assistant.

Even though I have called the project Sonoff-HomeAssistant, the switch could be used for many of the other home automation systems that uses a Mosquitto broker. In saying that, Im not sure why you would want to use anything else other than [Home Assistant](https://home-assistant.io/) :D It's just awesome!

Speaking of Home Assistant, I have included a snippet of how to setup the [switch](https://home-assistant.io/components/switch.mqtt/) component in configuration.yaml as well.

And finally, I did this to get begginers up and running quickly with HomeAssistant and Sonoff. I hope it helps :D

# Installation

## 1. Clone the Repository

Clone the **Sonoff-HomeAssistant** repository to your local machine.

``` bash
$ git clone https://github.com/KmanOz/Sonoff-HomeAssistant
```

## 2. Clone the lmroy/pubsubclient mqtt library to your local machine.

I use the [lmroy](https://github.com/Imroy/pubsubclient) version of this excellent mqtt library, mainly becasue it supports QOS1 and keepalive settings right from within the sketch. No other modifications to files are necesary to achieve a rock solid connection to your mqtt broker.

It's currently setup to use only v3.1.1 of the mqtt standard and will only work on that version broker unless you modify the code.

``` bash
$ git clone https://github.com/Imroy/pubsubclient
```

## 3. Flash the software to the Sonoff Switch

I won't go into the specifics on how to install the code on the Sonoff and will assume you have the necesary skills to make it happen. You'll need the Arduino IDE and you will need to move the files you just cloned to the right directories. There are plenty or articles that cover all the steps involved already published on the Interwebs.

As for the switch modifiations, it's simply a matter of opening up the unit, installing a 4 (or 5) pin header (you need to know how to solder) and then holding down the main switch on the unit before you power it up with your FTDI adapter. You are then good to go to re-flash your new firmware.

If that didn't make any sense at all, I suggest you do some reading on how to install alternative software on a Sonoff switch before attempting anything else.

## 4. Modify the details in the Arduino code to your specific details and environment.

Change the WIFI_SSID, WIFI_PASS, MQTT_CLIENT, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS in the Arduino code provided to suit your environment.

Note: MQTT_CLIENT needs to be unique for each Sonoff switch you install. For that matter so does MQTT_TOPIC.

``` bash
#define MQTT_CLIENT     "Sonoff_Living_Room_v1.0p"           // mqtt client_id (Must be unique for each Sonoff)
#define MQTT_SERVER     "192.168.0.100"                      // mqtt server
#define MQTT_PORT       1883                                 // mqtt port
#define MQTT_TOPIC      "home/sonoff/living_room/1"          // mqtt topic (Must be unique for each Sonoff)
#define MQTT_USER       "user"                               // mqtt user
#define MQTT_PASS       "pass"                               // mqtt password

#define WIFI_SSID       "homewifi"                           // wifi ssid
#define WIFI_PASS       "homepass"                           // wifi password
```

## 5. Modify configuration.yaml in HomeAssistant and add the following to it.

```bash
switch:
  platform: mqtt
  name: "Living Room"
  state_topic: "home/sonoff/living_room/1/stat"
  command_topic: "home/sonoff/living_room/1"
  qos: 1
  payload_on: "on"
  payload_off: "off"
  retain: true
```
Assuming you make no changes to the topic in the code, you should be able to test the switch and be happy that you now have control using Home Assistant.

## 6. Commands and Operation

As mentioned previously, the commands are very basic. In fact the switch will repond to 4 basic mqtt commands and they are :-

- on (Turns the relay and LED on)
- off (Turns the relay and LED off)
- stat (Returns the status of the switch via mqtt message)
- reset (Forces a restart of the switch) (4 long flashes of the status LED)

When power is first appplied the unit will immediately connect to your WiFi access point / router and mqtt broker. When it connects the status LED will flash fast 4 times. That's it, your connected.

Press the switch on top to turn it on. Press it again to turn it off and watch the status change on HomeAssistant. Toggle the switch on HomeAssistant and the switch will change status accordingly. I could have made it more complex, but why?

To reset the switch manually, press and hold the switch for more than 4 seconds. The switch will respond with 4 long flashes and reboot.

## 6. Versions

***Version 1.0p***

Initial Release

## 6. Conclusion

That's about it. Enjoy ! I have code that allows for a DHT-22 temperature sensor as well. The new batch of Sonoff switches expose GPIO14 to the header on pin 5 and it's fairly easy to install the sensor. I'm not sure that you should do it because internally all components are at mains potential. If your soldering is bad, it could be lethal. You've been warned !!

I can however post it up here if people are interested.