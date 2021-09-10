# photon-fridge-watcher

A Particle Photon project to monitor a fridge or freezer

## Features

#### Sensors

- Photoresistor - Used to monitor when the fridge door is open or closed based on fridge or ambient light
- Thermistor - Used to monitor the temperature inside the fridge
- Buzzer - Local alarm that sounds if the fridge door is left open for too long
- Button - Used to toggle the buzzer
- LED - LED that lights if the user disables the buzzer (via the button) OR if the fridge door has been left open for too long


### Monitor Fridge Door

A Photoresistor is used to monitor the ambient light in the fridge. If the door is opened, an intern timer will start. If the door is kept open for a configured amount of time, a buzzer will sound and an event (`FridgeDoorAlarm`) will be published.

#### Buzzer

The buzzer sounds if the door has been left open **continously** for a configured length of time.

- The buzzer can be silenced by pressing the button
- A warning LED will be visable anytime the the buzzer has been disabled letting the user know the buzzer will not sound if the fridge door has been left open
- The buzzer will continue until the fridge door is closed (or the button is used to disable the buzzer)

#### Event ```FridgeDoorAlarm```

This event is published if the door has been left open **continously** for a configured length of time. The published event can be used to trigger notifications such as IFTTT applets.

*Note: The button does **NOT** disable the notification*

**Data:** The event is published with a value that represents how many seconds the fridge door has been left open


### Monitor Fridge Temperature

A thermistor is used to track temperature inside the fridge and send notifications if the temperature exceeds a configured threshold after a configured amount of time.

#### Event ```FridgeTempAlarm```

This event is published when the fridge temperature has reached a configured threshold and has continued to say above this threshold for a **continous** length of time.

**Data:** The event is published with a `int` value that represents the current temperature reading (in F)

#### Event ```FridgeTemperature```

This event is pushed on a regular interval and reports the current temperature reading from the thermistor at that time (in F).

**Data:** The event is pushed with a `float` value that represents the current temperature reading (in F)

## Functions

- `setTempSamplePeriod(int)` - Configure delay (in seconds) between publishing the `FridgeTemperature` event (*Default:* 300)
- `setTempAlarmThreshold(float)` -  Configure the temperature threshold (in F) that will publish the `FridgeTempAlarm` event (*Default:* 39.0)
- `setTempAlarmDelay(int)` - Configure the length of time (in seconds) the fridge temperature must remain above the threshold before publishing the `FridgeTempAlarm` event (*Default:* 300)
- `setLightLevelThreshold(int)` - Configure light level threshold (voltage reading between 0-4095) that determines if the fridge door is open (higher value means more sensitive) (*Default:* 1500)
- `setDoorBuzzerDelay(int)` - Configure the length of time (in seconds) the fridge door must remain *continously* open before the buzzer alarm is triggerd (*Default:* 240)
- `setDoorNotificationDelay(int)` - Configure the length of time (in seconds) the fridge door must remain *continously* open before the `FridgeDoorAlarm` event is published. This also configures the time between re-sending the event if the fridge door remains open (*Default:* 180)
- `setToggleBuzzer(int)` - Toggles the buzzer to `0 => off` or `!0 => on`
- `resetSettings(void)` - Reset all thresholds, delays, etc... back to defaults

## Variables

- `photoresistor` - The current analog voltage reading from the photoresistor (0-4095) where a lower values denotes a high light level
- `temperature` - The current temperature (in F) reading inside the fridge
- `buzzerToggle` - `true` iff the buzzer is currently enabled, `false` otherwise
- `doorOpen` - `true` iff the fridge door is currently detected as 'open', `false` otherwise

## Libraries

- [photon-thermistor](https://github.com/kegnet/photon-thermistor)
- [clickButton](https://github.com/pkourany/clickButton)

## Support

Tested using firmware 3.1.0 on a [Particle Photon](https://store.particle.io/products/photon?_pos=1&_sid=e8e5c88f6&_ss=r)
