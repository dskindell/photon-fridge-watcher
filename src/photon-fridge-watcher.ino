#include <photon-thermistor.h> // https://github.com/kegnet/photon-thermistor

const int THERMISTOR_SERIES_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_TEMPERATURE = 25;
const int THERMISTOR_BETA_COEFFICIENT = 3984;
const int THERMISTOR_SAMPLE_SIZE = 5;
const int THERMISTOR_SAMPLE_DELAY_MS = 20;

const unsigned int TEMP_SAMPLE_PERIOD_S = 60;
const unsigned int DOOR_OPEN_BUZZER_DELAY_S = 120;
const unsigned int DOOR_OPEN_NOTIFICATION_REPEAT_DELAY_S = 30;
const double WARN_TEMPERATURE_THRESHOLD_F = 40.0;

float photoresistor;
Thermistor *thermistor;
double temperatureF;

boolean doorOpen = false;
boolean doorAlarm = false;
boolean warnTempAlarm = false;
double lastWarnTemp = 0.0;
unsigned int doorOpenCount = 0;
unsigned int doorOpenRepeatCount = 0;
unsigned int doorOpenSeconds = 0;

unsigned int count = 0;

void setup()
{
    pinMode(D0, OUTPUT); // Buzzer
    digitalWrite(D0, HIGH);

    pinMode(A1, INPUT); // Photoresistor
    thermistor = new Thermistor(A0,
        THERMISTOR_SERIES_RESISTANCE,
        THERMISTOR_NOMINAL_RESISTANCE,
        THERMISTOR_NOMINAL_TEMPERATURE,
        THERMISTOR_BETA_COEFFICIENT,
        THERMISTOR_SAMPLE_SIZE,
        THERMISTOR_SAMPLE_DELAY_MS);
}

void loop()
{
    photoresistor = analogRead(A1);

    // Photoresitor detectect door opening
    if (photoresistor <= 1500.0 and !doorOpen)
    {
        doorOpen = true;
        doorOpenCount = millis();
        Particle.publish("Door", "Open");
    }

    // Photoresitor detectect door closing
    if (photoresistor >= 1510.0 and doorOpen)
    {
        doorOpen = false;
        doorOpenCount = millis();
        Particle.publish("Door", "Closed");
    }

    // Toggle alarm when fridge door has been left open for too long
    if (doorOpen and millis() >= (doorOpenCount + 1000 * DOOR_OPEN_BUZZER_DELAY_S)) {
        if (!doorAlarm) {
            doorAlarm = true;
            //digitalWrite(D0, LOW);
            NotifyDoorAlarm();
        }
    } else if (doorAlarm) {
        doorAlarm = false;
        digitalWrite(D0, HIGH);
    }

    // Repeat notification if door continues to remain open after initial alarm
    if (doorAlarm and millis() >= (doorOpenRepeatCount + 1000 * DOOR_OPEN_NOTIFICATION_REPEAT_DELAY_S)) {
        NotifyDoorAlarm();
    }

    // Publish temperature data on a regular period
    if (millis() >= (count + 1000 * TEMP_SAMPLE_PERIOD_S))
    {
        count = millis(); // Reset the value of count
        temperatureF = thermistor->readTempF();
        Particle.publish("TemperatureF_T1", String(temperatureF), PRIVATE);

        // Alarm if fridge temp crosses threshold
        if (temperatureF > WARN_TEMPERATURE_THRESHOLD_F) {
            if (temperatureF > lastWarnTemp or !warnTempAlarm) {
                warnTempAlarm = true;
                NotifyWarnTemperature();
            }
        } else if (temperatureF <= WARN_TEMPERATURE_THRESHOLD_F - 2.0) {
            warnTempAlarm = false;
        }
    }
}

void NotifyDoorAlarm() {
    doorOpenRepeatCount = millis();
    doorOpenSeconds = (millis() - doorOpenCount) / 1000;
    //Particle.publish("DoorAlarm", String(doorOpenSeconds));
    Particle.publish("fridgeDoorOpen", String(doorOpenSeconds), 60, PRIVATE);
}

void NotifyWarnTemperature() {
    lastWarnTemp = temperatureF;
    Particle.publish("fridgeWarnTemp", String::format("%.2lf", temperatureF), 60, PRIVATE);
}
