#include <math.h> //photon 01 code
#include <photon-thermistor.h> // https://github.com/kegnet/photon-thermistor

const float MAX_ANALOG_READ_AT_3_3_V = 4095.0;
const float MAX_VOLTAGE = 3.3;

const int THERMISTOR_SERIES_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_TEMPERATURE = 25;
const int THERMISTOR_BETA_COEFFICIENT = 3380;
const int THERMISTOR_SAMPLE_SIZE = 5;
const int THERMISTOR_SAMPLE_DELAY_MS = 20;

const unsigned int TEMP_SAMPLE_PERIOD_SECONDS = 30;

const unsigned int DOOR_OPEN_BUZZER_DELAY_SECONDS = 10;

float photoresistor;
Thermistor *thermistor;

boolean doorOpen = false;
boolean doorAlarm = false;
unsigned int doorOpenCount = 0;

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

    if (photoresistor <= 1500.0 and !doorOpen) // Photoresitor detectect door opening
    {
        doorOpen = true;
        doorOpenCount = millis();
        Particle.publish("Door", "Open");
    } //pubilsh the event 1 time

    if (photoresistor >= 1510.0 and doorOpen) // Photoresitor detectect door closing
    {
        doorOpen = false;
        doorOpenCount = millis();
        Particle.publish("Door", "Closed");
    } // Publish the event 1 time

    if (doorOpen and millis() >= (doorOpenCount + 1000 * DOOR_OPEN_BUZZER_DELAY_SECONDS)) {
        if (!doorAlarm) {
            doorAlarm = true;
            Particle.publish("DoorAlarm");
            //digitalWrite(D0, LOW);
        }
    } else if (doorAlarm) {
        doorAlarm = false;
        digitalWrite(D0, HIGH);
    } // Toggle door-open buzzer

    if (millis() >= (count + 1000 * TEMP_SAMPLE_PERIOD_SECONDS)) // If statment that occuring every X seconds
    {
        count = millis(); // Reset the value of count
        //Particle.publish("Photo", String::format("%.1f", photoresistor));
        Particle.publish("TemperatureF_T1", String(thermistor->readTempF()));
    } // publish temeperature
}
