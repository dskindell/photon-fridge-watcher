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

const unsigned int TEMP_SAMPLE_PERIOD_SECONDS = 10;

float Photo;
boolean doorOpen = false;
unsigned int count = 0;
int Button_pressed = 0;

Thermistor *thermistor;

void setup()
{
    //pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    //pinMode(A2, INPUT);
    //Particle.subscribe("Button_Ini2", Button);

    thermistor = new Thermistor(A0,
        THERMISTOR_SERIES_RESISTANCE,
        THERMISTOR_NOMINAL_RESISTANCE,
        THERMISTOR_NOMINAL_TEMPERATURE,
        THERMISTOR_BETA_COEFFICIENT,
        THERMISTOR_SAMPLE_SIZE,
        THERMISTOR_SAMPLE_DELAY_MS);
}

void Button(const char *event, const char *data)
{
    Button_pressed = 1;
}

void loop()
{
    Photo = analogRead(A1);

/*
    Therm1 = analogRead(A0);
    // https://www.electronicwings.com/particle/thermistor-interfacing-with-particle-photon
    OutputVoltage = (Therm1 / MAX_ANALOG_READ_AT_3_3_V) * MAX_VOLTAGE;
    ThermistorResistance = ((MAX_VOLTAGE * THERMISTOR_SERIES_RESISTANCE) / OutputVoltage) - THERMISTOR_SERIES_RESITANCE;
    // Steinhart-Hart Thermistor Equation:
    // Temperature in Kelvin = 1 / (A + B[ln(R)] + C[ln(R)]^3)
    // where A = 0.001129148, B = 0.000234125 and C = 8.76741*10^-8
    ThermResLn = log(ThermistorResistance);
    TemperatureK = (1.0 / (0.001129148 + (0.000234125 * ThermResLn) + (0.0000000876741 * ThermResLn * ThermResLn * ThermResLn)));
    TemperatureF = (TemperatureK - 273.15) * 9.0 / 5.0 + 32.0; // Convert temp in K to F
*/

    if (Photo <= 1500.0 and doorOpen == false) // Photoresitor detectect door opening
    {
        doorOpen = true;
        Particle.publish("DoorOn", "Open");
    } //pubilsh the event 1 time

    if (Photo >= 1510.0 and doorOpen == true) // Photoresitor detectect door closing
    {
        doorOpen = false;
        Particle.publish("DoorOff", "Closed");
    } // Publish the event 1 time

    if (millis() >= (count + 1000 * TEMP_SAMPLE_PERIOD_SECONDS)) // If statment that occuring every X seconds
    {
        count = millis(); // Reset the value of count
        Particle.publish("Photo", String::format("%.1f", Photo));
        Particle.publish("Temperature_T1", String(thermistor->readTempF()));
    } // publish temeperature

/*
    if ((TemperatureF2 <= 40.0) and (Button_pressed == 1)) // If statemnt that says when the temperature of the sensor 2 has a good temeprature
    {
        Particle.publish("Beverage", "Ready"); // Publish the envent that says that the sesor has now a cold temperaure
        delay(10);
        Button_pressed = 0; // Reset the cycle for the button
    }

    if ((Button_pressed == 1) and (millis() >= (count2 + 60000))) // When button was presed, publish the temperautre of the sensor.
    {
        count2 = millis();
        Particle.publish("Temperature_T2", String(TemperatureF2));
    }
*/
}
