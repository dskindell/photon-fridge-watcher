#include <photon-thermistor.h> // https://github.com/kegnet/photon-thermistor
#include <clickButton.h> // https://github.com/pkourany/clickButton

const int THERMISTOR_SERIES_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_TEMPERATURE = 25;
const int THERMISTOR_BETA_COEFFICIENT = 3435;
const int THERMISTOR_SAMPLE_SIZE = 5;
const int THERMISTOR_SAMPLE_DELAY_MS = 20;

const unsigned int TEMP_SAMPLE_PERIOD_S = 60;
const double WARN_TEMPERATURE_THRESHOLD_F = 85.0;

const float PHOTO_RESISTOR_OPEN_THRESHOLD = 1500.0;
const float PHOTO_RESISTOR_CLOSE_THRESHOLD = 1510.0;

const unsigned int DOOR_OPEN_BUZZER_DELAY_S = 10;
const unsigned int DOOR_OPEN_NOTIFICATION_REPEAT_DELAY_S = 30;

const uint16_t BUZZER_PIN = D0;
const uint16_t BUTTON_PIN = D5;
const uint16_t LED_PIN = D6;
const uint16_t THERMISTOR_PIN = A0;
const uint16_t PHOTORESISTOR_PIN = A1;

float photoresistor;
Thermistor *thermistor;
double temperature;

boolean warnTempAlarm = false;
double lastWarnTemp = 0.0;
unsigned int lastPublishTempTime = 0;

boolean doorOpen = false;
boolean doorAlarm = false;
unsigned int lastDoorChangeTime = 0;
unsigned int lastDoorAlarmTime = 0;
unsigned int doorOpenSeconds = 0;

ClickButton *button;
boolean buzzerToggle = false;

void setup()
{
    // LED
    pinMode(LED_PIN, OUTPUT);

    // Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Button
    button = new ClickButton(BUTTON_PIN, LOW, CLICKBTN_PULLUP);

    // Photoresistor
    pinMode(PHOTORESISTOR_PIN, INPUT);

    // Thermistor
    thermistor = new Thermistor(THERMISTOR_PIN,
                                THERMISTOR_SERIES_RESISTANCE,
                                THERMISTOR_NOMINAL_RESISTANCE,
                                THERMISTOR_NOMINAL_TEMPERATURE,
                                THERMISTOR_BETA_COEFFICIENT,
                                THERMISTOR_SAMPLE_SIZE,
                                THERMISTOR_SAMPLE_DELAY_MS);
}

void loop()
{
    UpdateBuzzerToggle();

    UpdateDoorState();

    UpdateDoorAlarm();

    UpdateLEDState();

    PublishTemperature();
}

void UpdateLEDState()
{
    // Light LED if buzzer is disabled OR if door alarm is active
    if (!buzzerToggle or
        doorAlarm)
    {
        digitalWrite(LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
    }
}

void UpdateBuzzerToggle()
{
    button->Update();

    if (button->clicks != 0)
    {
        buzzerToggle = !buzzerToggle;
    }
}

void UpdateDoorState()
{
    photoresistor = analogRead(PHOTORESISTOR_PIN);

    // Photoresitor detectect door opening
    if (photoresistor <= PHOTO_RESISTOR_OPEN_THRESHOLD and !doorOpen)
    {
        doorOpen = true;
        lastDoorChangeTime = millis();
        Particle.publish("FridgeDoor", "Open");
    }

    // Photoresitor detectect door closing
    if (photoresistor >= PHOTO_RESISTOR_CLOSE_THRESHOLD and doorOpen)
    {
        doorOpen = false;
        lastDoorChangeTime = millis();
        Particle.publish("FridgeDoor", "Closed");
    }
}

void PublishDoorAlarm()
{
    lastDoorAlarmTime = millis();
    doorOpenSeconds = (millis() - lastDoorChangeTime) / 1000;
    Particle.publish("FridgeDoorAlarm", String(doorOpenSeconds), 60, PRIVATE);
}

void UpdateDoorAlarm()
{
    // Toggle alarm when fridge door has been left open for too long
    if (doorOpen and millis() >= (lastDoorChangeTime + 1000 * DOOR_OPEN_BUZZER_DELAY_S))
    {
        if (!doorAlarm)
        {
            doorAlarm = true;
            PublishDoorAlarm();
        }
    }
    else if (doorAlarm)
    {
        doorAlarm = false;
    }

    // Repeat notification if door continues to remain open after initial alarm
    if (doorAlarm and millis() >= (lastDoorAlarmTime + 1000 * DOOR_OPEN_NOTIFICATION_REPEAT_DELAY_S))
    {
        PublishDoorAlarm();
    }

    // Sound buzzer alarm (if enabled)
    if (doorAlarm and buzzerToggle)
    {
        digitalWrite(BUZZER_PIN, HIGH);
    }
    else
    {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void PublishTemperature()
{
    // Publish temperature data on a regular period
    if (millis() >= (lastPublishTempTime + 1000 * TEMP_SAMPLE_PERIOD_S))
    {
        lastPublishTempTime = millis();
        temperature = thermistor->readTempF();
        Particle.publish("FridgeTemperature", String(temperature), PRIVATE);

        // Alarm if fridge temp crosses threshold
        if (temperature > WARN_TEMPERATURE_THRESHOLD_F)
        {
            if (temperature > lastWarnTemp or !warnTempAlarm)
            {
                warnTempAlarm = true;
                lastWarnTemp = temperature;
                Particle.publish("FridgeWarnTemp", String::format("%.2lf", temperature), 60, PRIVATE);
            }
        }
        else if (temperature <= WARN_TEMPERATURE_THRESHOLD_F - 2.0)
        {
            warnTempAlarm = false;
        }
    }
}
