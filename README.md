# Sonoff-HomeAssistant Description

Sonoff-HomeAssistant is alternative firmware for the brilliant & cheap ($$$ not quality) [ITEAD Sonoff](https://www.itead.cc/sonoff-wifi-wireless-switch.html) switch that allows you to use your own mqtt broker rather than the pre-defined 'ITEAD CLOUD' service that is shipped with the firmware in the unit. This would have to be one of the cheapest IoT switches available today. In fact, even if you knew how to build one, the components alone would cost more so why bother.

It's designed to be installed with the Arduino IDE and has been tested on Arduino 1.6.9 but should be backwards & forwards compatible with other versions released. I realize there are many versions of mqtt based firmware(s) that have been written for the Sonoff switch, but I found most of them overly complex for my liking. This firmware is very basic but stable and just gets the job done. There is no RTC, no OTA firmware updates, no frills what so ever just the core functionality the switch needs to turn the relay on and off (and report temperature if using that ver). I've found that once the mqtt topic is set and the Sonoff switch has connected to your broker, you don't need to make any modifications to it ever again unless your infrascructure changes (i.e walls in home have moved, light becomes a fan, additional switches added in room etc). Even if you add additional switches, if your naming convention is right, the switch will not need to be touched again.

I've called the project Sonoff-HomeAssistant but the switch could be used for many of the other home automation systems that use a Mosquito broker. I'm not sure why you'd want to use anything other than [Home Assistant](https://home-assistant.io/) though :D

Speaking of Home Assistant, I have included a snippet of how to setup the [switch](https://home-assistant.io/components/switch.mqtt/) component in configuration.yaml and if you've installed Version 1.0t the [sensor](https://home-assistant.io/components/sensor.mqtt/) component as well.

And finally, I did this to help beginners get up and running quickly with HomeAssistant and Sonoff switches. A couple of hours of work and you can have your home automated at a fraction of the cost of some of the switches available today. 

# Installation

## 1. Clone the Repository

Clone the **Sonoff-HomeAssistant** repository to your local machine. Copy the required version (p OR t) to your Arduino directory. If using the temperature version of the code, you'll need the DHT library as well. It's available all over the internet.

``` bash
$ git clone https://github.com/KmanOz/Sonoff-HomeAssistant
```

## 2. Clone the lmroy/pubsubclient mqtt library to your local machine.

I use the [lmroy](https://github.com/Imroy/pubsubclient) version of this excellent mqtt library, mainly because it supports QOS1 and keepalive settings right from within the sketch. No other modifications to library files are necessary to achieve a rock solid connection to your mqtt broker.

It's currently setup to use only v3.1.1 of the mqtt standard and will only work on that version broker unless you modify the code so make sure your broker is setup to use v3.1.1 of the mqtt standard and not v3.1.

``` bash
$ git clone https://github.com/Imroy/pubsubclient
```

## 3. Flash the software to the Sonoff Switch

I won't go into the specifics on how to install the code onto the Sonoff and will assume you have the necessary skills to make it happen. You'll need the Arduino IDE and you will need to move the files you just cloned to the right directories. There are plenty or articles that cover all the steps involved already published on the Internet and a Google search should get you some good results.

As for the switch modifications, it's simply a matter of opening up the unit, installing a 4 pin (or 5 pin for Temperature) header (you need to know how to solder) and then holding down the main switch on the unit before you power it up with your FTDI adapter. You are then good to go to re-flash your new firmware.

If that didn't make any sense at all, I suggest you do some reading on how to install alternative software on a Sonoff switch before attempting anything else otherwise you risk turning it into toast (although it's pretty hard I have to admit).

## 4. Modify the details in the Arduino code to your specific details and environment.

Change the WIFI_SSID, WIFI_PASS, MQTT_CLIENT, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS in the Arduino code provided to suit your environment.

**** IMPORTANT **** MQTT_CLIENT needs to be unique for each Sonoff switch you install. MQTT_USER & MQTT_PASS can be the same but consider MQTT_CLIENT to be the equivalent of a MAC address. MQTT_CLIENT is how your broker identifies what hardware (or software for that matter) is talking to the broker at any one time and needs to be unique. So does MQTT_TOPIC unless you want 2 switches to respond to 1 topic.

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
Assuming you make no changes to the topic in the code provided, you should be able to test the switch and be happy that you now have control using Home Assistant.

If you've installed Version 1.0t, you can also setup sensors in HomeAssistant to display both Temperature & Humidity. Modify your configuration.yaml and add the following.

```bash
- platform: mqtt
  name: "Living Room Temp"
  state_topic: "home/sonoff/living_room/1/temp"
  qos: 1
  unit_of_measurement: "°C"
  value_template: "{{ value_json.Temp }}"
- platform: mqtt
  name: "Living Room Humidity"
  state_topic: "home/sonoff/living_room/1/temp"
  qos: 1
  unit_of_measurement: "%"
  value_template: "{{ value_json.Humidity }}"
```

## 6. Commands and Operation

As mentioned earlier, the commands are very basic. In fact the switch will respond to 4 basic mqtt commands and they are :-

- on (Turns the relay and LED on)
- off (Turns the relay and LED off)
- stat (Returns the status of the switch via mqtt message)
- reset (Forces a restart of the switch) (4 long flashes of the status LED)

If you've installed Version 1.0t you have an additional option.

- temp (Forces a temperature & humidity check otherwise it's reported every 1 minute) (1 short flash of the status LED)

When power is first applied the unit will immediately connect to your WiFi access point / router and your mqtt broker. When it connects the status LED will flash fast 4 times. That's it, your connected.

If you've installed v1.0t immediately after the 4 fast flashes you will see a short single flash to indicate that the temperature & humidity has been published as well.

Press the switch on top to turn on the relay. Press it again to turn it off and watch the status change in HomeAssistant. Toggle the switch in HomeAssistant and the relay & LED will toggle accordingly. I could have made it more complex, but why?

To reset the switch manually, press and hold the switch for more than 4 seconds. The switch will respond with 4 long flashes and reboot.

## 6. Versions

***Version 1.0p***

Firmware to control relay only with ON/FF functionality and publish it's status via mqtt.

***Version 1.0t***

Firmware to control relay with ON/OFF functionality and temperature reporting via DHT11/22 and publish via mqtt.

## 7. DHT22 Sensor Installation

Installing the DHT11 or 22 sensor is relatively straight forward. DHT-11 reports only whole number values for temperture & humidity while the DHT-22 will report fractions as well. I recommend the DHT-22. In the photo below, GREY is SIGNAL (gpio14), WHITE is +V and BLACK is GND. Note how it connects to the Sonoff pins.

![alt Sonoff Sensor Install](images/sonoff_temp.JPG "Sonoff Sensor Install")

You will need a 10K resistor between +V and SIGNAL. In the image above, I've the soldered the 10K resistor under the heatshrink tubing so it isn't visible.

![alt Sensor Wiring](images/sensor_wiring.jpg "Sensor Wiring")

How you get the wires out of the casing after it's all assembled is completely up to you. I guess a Dremel and some handywork will be required but it's a tight squeeze. Remember... even though the switch uses a capacitor dropper and small switchmode supply to reduce mains voltage down to 3.3v, the whole circuit board is at mains potential and should be considered dangerous. BE CAREFUL.

Make sure to modify the Arduino code to indicate which sensor you are using.

``` bash
#define DHTTYPE         DHT22                                // (11 or 22) DHT Type
```

## 8. Conclusion

That's about it. Any feature suggestions are welcome and I would be happy to answer any further questions that you may have.

Enjoy!
