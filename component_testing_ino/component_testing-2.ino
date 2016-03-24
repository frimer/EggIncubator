// Code contribution for Egg Incubator by Viskor - http://www.instructables.com/member/viskor/
// It is assumed that the LCD module is connected to the following pins.
// SCK		- Pin 8
// MOSI		- Pin 9
// DC		- Pin 10
// RST		- Pin 11
// CS		- Pin 12

#include <LCD5110_Basic.h>	// librairie LCD5110_basic
#include "DHT.h"		// librairie DHT
#include <Servo.h>

#define DHTTYPE	DHT11		// Type of DHT
#define DHTPIN	5		// pin of the DHT
#define RELAY1	6		// relay1 on pin 6
#define RELAY2	7		// Relay2 on pin 7

// Create classes
Servo myservo;			// Servo class
DHT dht(DHTPIN, DHTTYPE);	// DHT Sensor class
LCD5110 myGLCD(8,9,10,11,12);	// LCD Display pin declarations

// LCD Display Fonts
extern uint8_t SmallFont[];	// small font (original)
extern uint8_t WideFont[];	// wide font (must be add to librairie)
extern uint8_t BigNumbers[];	// big number original

// Tracking values
int pos = 	5;		// servo position in degrees
int sens =	1;		// sens 1 = pos 5-185, sens 2 = pos 185-5
int rotation =	1;		// loop when rotation = 25 move the servo
int redPint =	A0;		// Red LED temperature connected to analogic pin 0
int greenPint =	A1;		// Green LED temperature connected to analogic pin 1
int redPinh =	A2;		// Red LED humidity connected to analogic pin 2
int greenPinh =	A3;		// Green LED humidity connected to analogic pin 3

void setup() {
	Serial.begin(9600);
	Serial.println("DHTxx test!");
	myGLCD.InitLCD();
	dht.begin();
	myservo.attach(2);
	pinMode(redPint,	OUTPUT);	// sets the analog pin as output
	pinMode(greenPint,	OUTPUT);	// sets the analog pin as output
	pinMode(redPinh,	OUTPUT);	// sets the analog pin as output
	pinMode(greenPinh,	OUTPUT);	// sets the analog pin as output
	pinMode(RELAY1,		OUTPUT);
	pinMode(RELAY2,		OUTPUT);
}

void loop() {
	if(sens = 1)  {			// goes from 0 degrees to 180 degrees
		pos += 5;		// in steps of 1 degree
		myservo.write(pos);	// tell servo to go to position in variable 'pos'
		delay(15);		// waits 15ms for the servo to reach the position
		if(pos > 180) {
			// sens = 2;
			pos = 5;
		}
	}

	//if(sens = 2) {		// goes from 180 degrees to 0 degrees
	//	pos -= 5;		// in steps of 1 degree
	//	myservo.write(pos);	// tell servo to go to position in variable 'pos'
	//	delay(15);		// waits 15ms for the servo to reach the position
	//	if(pos <= 10) {
	//		sens = 1;
	//	}
	//}

	delay(5000);				// Wait a few seconds between measurements.

	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float h = dht.readHumidity();		// Read temperature as Celsius
	float t = dht.readTemperature();	// Check if any reads failed and exit early (to try again).

	if (isnan(h) || isnan(t)) {		// Check to be sure sensor returned values we can use.
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	// Print for serial interface
	Serial.print("Humidity: ");
	Serial.print(h);
	Serial.print(" %\t");
	Serial.print("Temperature: ");
	Serial.print(t);
	Serial.print(" *C ");

	// Statut of led Green/RED of TEMPERATURE
	if(t > 26) { //turn red when is OVER + turn off relais heat + display HOT
		myGLCD.setFont(SmallFont);
		myGLCD.print("- HOT -", LEFT, 25);	// display hot
		digitalWrite(greenPint, LOW);		// sets the Green LED off
		digitalWrite(redPint, HIGH);		// sets the Red LED on
		// digitalWrite(RELAY1,HIGH);
	}
	else if(t < 25) { //turn red when is Lower + turn on relais heat + display COLD
		myGLCD.setFont(SmallFont);
		myGLCD.print("- COLD -", LEFT, 25);	// display cold
		digitalWrite(greenPint, LOW);		// sets the Green LED off
		digitalWrite(redPint, HIGH);		// sets the Red LED on
		// digitalWrite(RELAY1,LOW);
	}
	else { // stay green when is not lower or hight + display good
		myGLCD.setFont(SmallFont);
		myGLCD.print("- GOOD -", LEFT, 25);
		digitalWrite(redPint, LOW);		// sets the Red LED off
		digitalWrite(greenPint, HIGH);		// sets the Green LED on
		// digitalWrite(RELAY1,LOW);
	}
	// Statut of led Green/RED of HUMIDITY
	if(h > 48) { //turn red when is OVER
		myGLCD.setFont(SmallFont);
		myGLCD.print("- HIGH -", LEFT, 40);
		digitalWrite(greenPinh, LOW);		// sets the Green LED off
		digitalWrite(redPinh, HIGH);		// sets the Red LED on
		// digitalWrite(RELAY2,HIGH);
	}
	else if(h < 38) { //turn red when is Lower
		myGLCD.setFont(SmallFont);
		myGLCD.print("- LESS -", LEFT, 40);
		digitalWrite(greenPinh, LOW);		// sets the Green LED off
		digitalWrite(redPinh, HIGH);		// sets the Red LED on
		// digitalWrite(RELAY2,HIGH);
	} else { // stay green when is not lower or over
		myGLCD.setFont(SmallFont); // D
		myGLCD.print("- GOOD -", LEFT, 40);
		digitalWrite(redPinh, LOW);		// sets the Red LED off
		digitalWrite(greenPinh, HIGH);		// sets the Green LED on

	}

	// Display on LCD
	myGLCD.setFont(SmallFont);
	myGLCD.print("~Viskor~", LEFT, 0);

	// display for temperature
	myGLCD.setFont(WideFont);
	myGLCD.print("Temp.", LEFT, 20);
	myGLCD.setFont(BigNumbers);
	myGLCD.printNumI(t, 48, 0);
	myGLCD.setFont(WideFont);
	myGLCD.print("C", RIGHT, 10);

	// display for humidity
	myGLCD.setFont(SmallFont);
	myGLCD.print("Humidite", LEFT, 35);
	myGLCD.setFont(BigNumbers);
	myGLCD.printNumI(h, 48, 28);
	myGLCD.setFont(WideFont);
	myGLCD.print("%", RIGHT, 35);
}
