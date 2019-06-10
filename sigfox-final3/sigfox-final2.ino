#include <SoftwareSerial.h>
#include "LowPower.h"

#define ECHOPIN 11 // Pin to receive echo pulse
#define TRIGPIN 12 // Pin to send trigger pulse
#define sensor_pin 6
#define rxPin 8
#define txPin 7

// set up a new serial port
SoftwareSerial Sigfox = SoftwareSerial(rxPin, txPin);

//Set to 0 if you don't need to see the messages in the console
#define DEBUG 1

//Message buffer
uint16_t msg[12];

// the setup function runs once when you press reset or power the board

void setup()
{
    // initialize digital pin LED_BUILTIN as an output.

    pinMode(LED_BUILTIN, OUTPUT);

    if (DEBUG)
    {
        Serial.begin(9600);
    }

    // open Wisol communication
    // define pin modes for tx, rx:
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    pinMode(TRIGPIN, OUTPUT);
    digitalWrite(ECHOPIN, HIGH);
    pinMode(sensor_pin, OUTPUT);
    Sigfox.begin(9600);
    delay(100);
    getID();
    delay(100);
    getPAC();
}

// the loop function runs over and over again forever
void loop()
{
    digitalWrite(sensor_pin, HIGH);
    delay(4000);

    digitalWrite(LED_BUILTIN, HIGH);

    digitalWrite(TRIGPIN, LOW); // Set the trigger pin to low for 2uS
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH); // Send a 10uS high to trigger ranging
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);                   // Send pin low again
    int distance = pulseIn(ECHOPIN, HIGH, 26000); // Read in times pulse
    distance = distance / 58;
    Serial.println(distance);

    uint16_t distbyte = (distance + 100) * 100;
    Serial.println(distbyte);

    msg[0] = highByte(distbyte);
    msg[1] = lowByte(distbyte);

    sendMessage(msg, 2);

    // In the ETSI zone, due to the reglementation, an object cannot emit more than 1% of the time hourly
    // So, 1 hour = 3600 sec
    // 1% of 3600 sec = 36 sec
    // A Sigfox message takes 6 seconds to emit
    // 36 sec / 6 sec = 6 messages per hours -> 1 every 10 minutes
    // delay(3600000);

    digitalWrite(sensor_pin, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    for (int i = 0; i < 2; i++)
    {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}

void blink()
{
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(1000);                     // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);
}

//Get Sigfox ID
String getID()
{

    String id = "";
    char output;

    Sigfox.print("AT$I=10\r");
    delay(2000);
    output = Sigfox.read();
    id += output;
    delay(10);

    if (DEBUG)
    {
        Serial.println("Sigfox Device ID: ");
        Serial.println(id);
    }

    return id;
}

//Get PAC number
String getPAC()
{

    String pac = "";
    char output;

    Sigfox.print("AT$I=11\r");

    delay(2000);
    output = Sigfox.read();
    pac += output;
    delay(10);

    if (DEBUG)
    {
        Serial.println("PAC number: ");
        Serial.println(pac);
    }

    return pac;
}

//Send Sigfox Message
void sendMessage(uint16_t msg[], int size)
{

    String status = "";
    char output;

    Sigfox.print("AT$SF=");
    for (int i = 0; i < size; i++)
    {
        Sigfox.print(String(msg[i], HEX));
        if (DEBUG)
        {
            Serial.print("Byte:");
            Serial.println(msg[i], HEX);
        }
    }

    Sigfox.print("\r");

    delay(2000);
    output = (char)Sigfox.read();
    status += output;
    delay(10);

    if (DEBUG)
    {
        Serial.println();
        Serial.print("Status \t");
        Serial.println(status);
    }
}
