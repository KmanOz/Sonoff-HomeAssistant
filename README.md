# Sonoff-HomeAssistant Description

Sonoff-HomeAssistant is alternative firmware for the brilliant & cheap ($$$ not quality) [ITEAD Sonoff](https://www.itead.cc/sonoff-wifi-wireless-switch.html) switch that allows you to use your own mqtt broker rather than the pre-defined 'ITEAD CLOUD' service that is shipped with the firmware in the unit. This would have to be one of the cheapest IoT switches available today. In fact, even if you knew how to build one, the components alone would cost you more that the whole unit, so why bother.

Is is designed to be installed with the Arduino IDE and has been tested on Arduino 1.6.9 but should be backwards compatible. I realize there are many versions of mqtt based firmware(s) that have been written for the Sonoff switch, but I found most of them overly complex for my liking.

This firmware is very basic but gets the job done. There is no RTC, no OTA firmware updates, no frills what so ever. In fact, I found that once the mqtt topic is set and the Sonoff has connected to your broker, you rarely need to make any modifications to the switch, especially after it's been installed in your home and configured in Home Assistant.

Even though I have called the project Sonoff-HomeAssistant, the switch could be used for many of the other home automation systems that uses a Mosquitto broker. In saying that, Im not sure why you would want to use anything else other than [Home Assistant](https://home-assistant.io/) :D It's just awesome!

Speaking of Home Assistant, I have included a snippet of how to setup the [switch](https://home-assistant.io/components/switch.mqtt/) component in configuration.yaml and if you have installed Version 1.0t the [sensor](https://home-assistant.io/components/sensor.mqtt/) component as well.

And finally, I did this to help beginners get up and running quickly with HomeAssistant and Sonoff.

# Installation

## 1. Clone the Repository

Clone the **Sonoff-HomeAssistant** repository to your local machine. Copy the required version to your Arduino directory.

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

If you've installed Version 1.0t, you can also setup sensors in HomeAssistant to display both Temperature & Humidity. Modify your configuraation.yaml and add the following.

```bash
- platform: mqtt
  name: "Living Room Temp"
  state_topic: "home/sonoff/living_room/1/temp"
  qos: 0
  unit_of_measurement: "°C"
  value_template: "{{ value_json.Temp }}"
- platform: mqtt
  name: "Living Room Humidity"
  state_topic: "home/sonoff/living_room/1/temp"
  qos: 0
  unit_of_measurement: "%"
  value_template: "{{ value_json.Humidity }}"
```

## 6. Commands and Operation

As mentioned previously, the commands are very basic. In fact the switch will repond to 4 basic mqtt commands and they are :-

- on (Turns the relay and LED on)
- off (Turns the relay and LED off)
- stat (Returns the status of the switch via mqtt message)
- reset (Forces a restart of the switch) (4 long flashes of the status LED)

If you have installed Version 1.0t you have an additional option.

- temp (Forces a temperature & humidity check otherwise it's reported every 1 minute) (1 short flash of the status LED)

When power is first appplied the unit will immediately connect to your WiFi access point / router and mqtt broker. When it connects the status LED will flash fast 4 times. That's it, your connected.

If you've installed v1.0t immediately after the 4 fast flashes you will see a short single flash to indicate that the temperature & humidity has been published via mqtt.

Press the switch on top to turn on the relay. Press it again to turn it off and watch the status change on HomeAssistant. Toggle the switch on HomeAssistant and the switch will change status accordingly. I could have made it more complex, but why?

To reset the switch manually, press and hold the switch for more than 4 seconds. The switch will respond with 4 long flashes and reboot.

## 6. Versions

***Version 1.0p***

Firmware to control relay only with ON/FF functionality and publish it's status via mqtt.

***Version 1.0t***

Firmware to control relay with ON/OFF functionality and temperature reporting via DHT11/22 and publish via mqtt.

## 7. DHT22 Sensor Installation

Installing the DHT11 or 22 sensor is relatively straight forward. In the photo below, GREY is SIGNAL (gpio14), WHITE is +V and BLACK is GND. Note how it connects to the Sonoff pins.

![alt Sonoff Sensor Install](images/sonoff_temp.JPG "Sonoff Sensor Install")

You will need a 10K resistor between +5V and SIGNAL. I have the soldered the 10K resistor under the heatshrink tubing so it isn't visable.

![alt Sensor Wiring](images/sensor_wiring.jpg "Sensor Wiring")

How you get the wires out of the casing after it's all assembled is completely up to you. I guess a Dremel and some handywork would be handy.

Make sure to modify the Arduino code to indicate which sensor you are using.

``` bash
#define DHTTYPE         DHT22                                // (11 or 22) DHT Type
```

## 8. Conclusion

That's about it. Enjoy! Any suggestions are welcome and I would be happy to answer any further questions as well you may have.