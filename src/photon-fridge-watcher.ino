#include <photon-thermistor.h> // https://github.com/kegnet/photon-thermistor
#include <clickButton.h> // https://github.com/pkourany/clickButton

const unsigned int INIT_TEMPERATURE_SAMPLE_PERIOD_S     = 300; // seconds
const double INIT_TEMPERATURE_ALARM_THRESHOLD           = 0.0; // F
const unsigned int INIT_TEMPERATURE_ALARM_DELAY_S       = 300; // seconds
const int32_t INIT_PHOTO_RESISTOR_OPEN_THRESHOLD        = 4000.0; // photo-resistor dependent
const unsigned int INIT_DOOR_OPEN_BUZZER_DELAY_S        = 240; // seconds
const unsigned int INIT_DOOR_OPEN_NOTIFICATION_DELAY_S  = 180; // seconds

unsigned int TEMPERATURE_SAMPLE_PERIOD_S = INIT_TEMPERATURE_SAMPLE_PERIOD_S;
int setTempSamplePeriod(String input) { TEMPERATURE_SAMPLE_PERIOD_S = input.toInt(); return 0; }

double TEMPERATURE_ALARM_THRESHOLD = INIT_TEMPERATURE_ALARM_THRESHOLD;
int setTempAlarmThreshold(String input) { TEMPERATURE_ALARM_THRESHOLD = input.toFloat(); return 0; }

unsigned int TEMPERATURE_ALARM_DELAY_S = INIT_TEMPERATURE_ALARM_DELAY_S;
int setTempAlarmDelay(String input) { TEMPERATURE_ALARM_DELAY_S = input.toInt(); return 0; }

int32_t PHOTO_RESISTOR_OPEN_THRESHOLD = INIT_PHOTO_RESISTOR_OPEN_THRESHOLD;
int setLightLevelThreshold(String input) { PHOTO_RESISTOR_OPEN_THRESHOLD = input.toInt(); return 0; }

unsigned int DOOR_OPEN_BUZZER_DELAY_S = INIT_DOOR_OPEN_BUZZER_DELAY_S;
int setDoorBuzzerDelay(String input) { DOOR_OPEN_BUZZER_DELAY_S = input.toInt(); return 0; }

unsigned int DOOR_OPEN_NOTIFICATION_DELAY_S = INIT_DOOR_OPEN_NOTIFICATION_DELAY_S;
int setDoorNotificationDelay(String input) { DOOR_OPEN_NOTIFICATION_DELAY_S = input.toInt(); return 0; }

int resetSettings(String input) 
{
    TEMPERATURE_SAMPLE_PERIOD_S = INIT_TEMPERATURE_SAMPLE_PERIOD_S;
    TEMPERATURE_ALARM_THRESHOLD = INIT_TEMPERATURE_ALARM_THRESHOLD;
    TEMPERATURE_ALARM_DELAY_S = INIT_TEMPERATURE_ALARM_DELAY_S;
    PHOTO_RESISTOR_OPEN_THRESHOLD = INIT_PHOTO_RESISTOR_OPEN_THRESHOLD;
    DOOR_OPEN_BUZZER_DELAY_S = INIT_DOOR_OPEN_BUZZER_DELAY_S;
    DOOR_OPEN_NOTIFICATION_DELAY_S = INIT_DOOR_OPEN_NOTIFICATION_DELAY_S;
    return 0;
}

const uint16_t BUZZER_PIN = D0;
const uint16_t BUTTON_PIN = D5;
const uint16_t LED_PIN = D6;
const uint16_t THERMISTOR_PIN = A0;
const uint16_t PHOTORESISTOR_PIN = A1;

const int THERMISTOR_SERIES_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_RESISTANCE = 10000;
const int THERMISTOR_NOMINAL_TEMPERATURE = 25;
const int THERMISTOR_BETA_COEFFICIENT = 3435;
const int THERMISTOR_SAMPLE_SIZE = 5;
const int THERMISTOR_SAMPLE_DELAY_MS = 20;

int32_t photoresistor;
Thermistor *thermistor;
double temperature;

unsigned int lastPublishTempTime = 0;

double lastTempAlarm = 0.0;
unsigned int lastTempAlarmThresholdTime = 0;

boolean doorOpen = false;
boolean doorBuzzer = false;
boolean doorNotification = false;
unsigned int lastDoorChangeTime = 0;
unsigned int lastDoorNotificationTime = 0;
unsigned int doorOpenSeconds = 0;

ClickButton *button;
boolean buzzerToggle = false;

int setBuzzerToggle(String input)
{
    buzzerToggle = input.toInt() != 0 ? true : false;
    return 0;
}

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

    Particle.function("setTempSamplePeriod", setTempSamplePeriod);
    Particle.function("setTempAlarmThreshold", setTempAlarmThreshold);
    Particle.function("setTempAlarmDelay", setTempAlarmDelay);
    Particle.function("setLightLevelThreshold", setLightLevelThreshold);
    Particle.function("setDoorBuzzerDelay", setDoorBuzzerDelay);
    Particle.function("setDoorNotificationDelay", setDoorNotificationDelay);
    Particle.function("setToggleBuzzer", setBuzzerToggle);
    Particle.function("resetSettings", resetSettings);

    Particle.variable("photoresistor", photoresistor);
    Particle.variable("temperature", temperature);
    Particle.variable("buzzerToggle", buzzerToggle);
    Particle.variable("doorOpen", doorOpen);
}

void loop()
{
    // Update sensor data
    photoresistor = analogRead(PHOTORESISTOR_PIN);
    temperature = thermistor->readTempF();
    button->Update();
    if (button->clicks != 0)
    {
        buzzerToggle = !buzzerToggle;
    }

    // Detect if door has been opened
    if (photoresistor <= PHOTO_RESISTOR_OPEN_THRESHOLD and !doorOpen)
    {
        doorOpen = true;
        lastDoorChangeTime = millis();
    }
    // Detect if door has been closed
    if (photoresistor >= PHOTO_RESISTOR_OPEN_THRESHOLD + 10 and doorOpen)
    {
        doorOpen = false;
        lastDoorChangeTime = millis();
    }

    // Toggle buzzer when door has been left open for too long
    if (doorOpen and millis() >= (lastDoorChangeTime + 1000 * DOOR_OPEN_BUZZER_DELAY_S))
    {
        if (!doorBuzzer)
        {
            doorBuzzer = true;
        }
    }
    else if (doorBuzzer)
    {
        doorBuzzer = false;
    }
    // Sound buzzer alarm (if enabled)
    if (doorBuzzer and buzzerToggle)
    {
        digitalWrite(BUZZER_PIN, HIGH);
    }
    else
    {
        digitalWrite(BUZZER_PIN, LOW);
    }

    // Send notification when door has been left open for too long
    if (doorOpen and millis() >= (lastDoorChangeTime + 1000 * DOOR_OPEN_NOTIFICATION_DELAY_S))
    {
        if (!doorNotification)
        {
            doorNotification = true;
            lastDoorNotificationTime = millis();
            doorOpenSeconds = (millis() - lastDoorChangeTime) / 1000;
            Particle.publish("DoorAlarm", String(doorOpenSeconds), 60, PRIVATE);
        }
    }
    else if (doorNotification)
    {
        doorNotification = false;
    }

    // Light LED if buzzer is disabled OR if door alarm is active
    if (!buzzerToggle or
        doorBuzzer)
    {
        digitalWrite(LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
    }

    // Publish temperature data on a regular period
    if (millis() >= (lastPublishTempTime + 1000 * TEMPERATURE_SAMPLE_PERIOD_S))
    {
        lastPublishTempTime = millis();
        Particle.publish("Temperature", String(temperature), PRIVATE);
    }

    // Alarm if temp crosses threshold
    if (temperature > TEMPERATURE_ALARM_THRESHOLD)
    {
        // If this is the first time the threshold has been crossed, start a delay timer
        if (lastTempAlarmThresholdTime == (unsigned int)-1)
        {
            lastTempAlarmThresholdTime = millis();
        }
        else if (millis() >= (lastTempAlarmThresholdTime + 1000 * TEMPERATURE_ALARM_DELAY_S) and
                 (temperature > lastTempAlarm))
        {
            lastTempAlarmThresholdTime = (unsigned int)-1; // Alarm should not trigger more periodically than the delay
            lastTempAlarm = (temperature > lastTempAlarm ? temperature : lastTempAlarm);
            Particle.publish("TempAlarm", String::format("%.2lf", temperature), 60, PRIVATE);
        }
    }
    else if (temperature <= TEMPERATURE_ALARM_THRESHOLD)
    {
        lastTempAlarmThresholdTime = (unsigned int)-1;
    }
}
