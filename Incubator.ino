// Egg Incubator Sketch
// Written by: Matt Schreiner <Arduino@MidnightFantasy.com>
// June 9th, 2014
// Version 1.0

#include <dht.h>              // Temp and Humidity Sensor library
#include <LiquidCrystal.h>    // LCD Display library
#include <Time.h>
#include <TimeAlarms.h>       // Be SURE to modify the TimeAlarms.h to increment the dtNBR_ALARMS definition or alarms won't work correctly, if at all!
                              // Total needed are 3 + numer of egg turns (7 default) so set to a minimum of 10, 27 is the max for 24 egg turns per day.

// ------------------------------------------------------------------------------------------------------
// ---  SET DEFAULT VALUES FOR CHICKEN OR DUCK INCUBATION AND HATCHING DAYS, TEMP, AND HUMITITY HERE   --
// --- IF YOU CHANGE THESE VALUES HERE, BE SURE TO ALSO SET THEM IN THE updateDisplay EGG TYPE SECTION --
// ------------------------------------------------------------------------------------------------------
boolean  duck               = true;   // Using default values for DUCK eggs, set to FALSE if you want to default to CHICKEN eggs
int      incubationDays     = 25;     // Duck == 25, Chicken == 18
double   incubationTemp     = 99.5;   // Chicken and Ducks are the same, 99.5 for incubation temp
double   incubationHumidity = 86;     // Duck == 86, Chicken == 50
double   hatchingTemp       = 98.5;   // Chicken and Ducks are the same, 99.5 for hatching temp
double   hatchingHumidity   = 94;     // Duck == 94, Chicken == 65
int      turnsDaily         = 7;      // Set to (minimum) 3 turns, once every 8 hours, or it will turn the eggs 7 times daily.
                                      // The value should be an odd numer (if not hourly) so yolk doesn't stick to embryo overnight.

#define DHTTYPE DHT22   // DHT 22  (AM2302)

// PIN definitions / assignments
//Open Pin                  0    // RX Serial Pin
//Open Pin                  1    // TX Serial Pin
#define LCDD7               2    // LCD D7 pin to digital pin 2
#define LCDD6               3    // LCD D6 pin to digital pin 3
#define LCDD5               4    // LCD D5 pin to digital pin 4
#define LCDD4               5    // LCD D4 pin to digital pin 5
#define DHT_PIN             6    // data wire for DHT environmental sensor
#define endstop1            7    // one end of travel for egg turner - wire with a pull DOWN resistor
#define endstop2            8    // opposite end of travel for egg turner - wire with a pull DOWN resistor
#define screenButton        9    // button1 for toggling the screenMode - wire with a pull DOWN resistor
#define settingsButton     10    // button2 for incrementing days, hours, and minutes - wire with a pull DOWN resistor
#define LCDEnable          11    // LCD Enable pin to digital pin 11
#define LCDRS              12    // LCD RS pin to digital pin 12
#define led                13    // LED status indicator to show in water, heater, fan, or egg relays are on
#define heaterRelay        A0    // signal wire for heater relay
#define fanRelay           A1    // signal wire for fan relay
#define waterRelay         A2    // signal wire for water valve relay
#define eggRelay           A3    // signal wire for egg turner relay
//Open Pin                 A4
//Open Pin                 A5

// -----------------------------------------------------------------------------------------------
// -- NPN or PNP transistor type dictates if the base is set to HIGH or LOW to enable the relay --
// -- Set the off and on values for LOW and HIGH to set the correct behavior expected based on  --
// -- on your electrical wiring.                                                                --
// -----------------------------------------------------------------------------------------------
#define off                HIGH    // Set to LOW if NPN, HIGH for PNP
#define on                 LOW     // Set to HIGH if NPN, LOW for PNP

// State Machine variables
boolean heat             = false;
boolean fan              = true;
boolean water            = false;
boolean egg              = false;
boolean eggSwitch1Status = LOW;
boolean eggSwitch2Status = LOW;

int chk;                           // Environment sensor status
int eggSwitchTarget      = 0;      // Endswitch destination
int currentSwitch        = 0;      // Which endswitch is currently active
int dayCount             = 1;      // Day in the incubation cycle

int screenMode           = 1;      // Which screen to show on the LCD for status or setting day, hour, and minute
int screenButtonState    = LOW;    // Button 1 for toggling the screen mode
int settingsButtonState  = LOW;    // Button 2 for incrementing day, hour, and minute

int debug                = 1;      // Enable (1) or Disable (0) debug output on serial line for tightest running loops - will have performance impact

// Tracking variables
double targetTemp        = incubationTemp;
double targetHumidity    = incubationHumidity;
double currentHumidity   = 0;
double currentTemp       = 0;

// Instantiate classes for temp/humidity sensor and LCD display
dht dht;
LiquidCrystal lcd(LCDRS, LCDEnable, LCDD4, LCDD5, LCDD6, LCDD7);

void setup() {                
  setTime(9,4,30,6,1,14); // set time to 9:04:30am June 1 2014

  Alarm.alarmRepeat(9,0,0, trackDays);        // bump the daycount every morning at 9am
  Alarm.timerRepeat(5, checkStateMachine);    // every 5 seconds, check the state machine of the incubator environment 
  Alarm.timerRepeat(1, checkEggTurn);         // every second, check to see if we need to turn, or stop turning, the eggs

  if (turnsDaily == 3) {
    Serial.println("Setting alarms to turn eggs 3 times daily");
    Alarm.alarmRepeat(0,5,0, turnEggs);   // 1am every day
    Alarm.alarmRepeat(8,5,0, turnEggs);   // 9am every day
    Alarm.alarmRepeat(16,5,0, turnEggs);  // 5pm every day
  } 
  else {
    Serial.println("Setting alarms to turn eggs 7 times daily");
    //    Alarm.alarmRepeat(0,5,0, turnEggs);
    Alarm.alarmRepeat(1,5,0, turnEggs);
    //    Alarm.alarmRepeat(2,5,0, turnEggs);
    //    Alarm.alarmRepeat(3,5,0, turnEggs);
    //    Alarm.alarmRepeat(4,5,0, turnEggs);
    Alarm.alarmRepeat(5,5,0, turnEggs);
    //    Alarm.alarmRepeat(6,5,0, turnEggs);
    //    Alarm.alarmRepeat(7,5,0, turnEggs);
    //    Alarm.alarmRepeat(8,5,0, turnEggs);
    Alarm.alarmRepeat(9,5,0, turnEggs);
    //    Alarm.alarmRepeat(10,5,0, turnEggs);
    //    Alarm.alarmRepeat(11,5,0, turnEggs);
    Alarm.alarmRepeat(12,5,0, turnEggs);
    //    Alarm.alarmRepeat(13,5,0, turnEggs);
    //    Alarm.alarmRepeat(14,5,0, turnEggs);
    Alarm.alarmRepeat(15,5,0, turnEggs);
    //    Alarm.alarmRepeat(16,5,0, turnEggs);
    //    Alarm.alarmRepeat(17,5,0, turnEggs);
    Alarm.alarmRepeat(18,5,0, turnEggs);
    //    Alarm.alarmRepeat(19,5,0, turnEggs);
    //    Alarm.alarmRepeat(20,5,0, turnEggs);
    Alarm.alarmRepeat(21,5,0, turnEggs);
    //    Alarm.alarmRepeat(22,5,0, turnEggs);
    //    Alarm.alarmRepeat(23,5,0, turnEggs);
  }

  lcd.begin(16, 2);
  lcd.clear();
  // initialize the digital pin as an output.
  pinMode(led,            OUTPUT);    
  pinMode(heaterRelay,    OUTPUT);
  pinMode(fanRelay,       OUTPUT);
  pinMode(waterRelay,     OUTPUT);
  pinMode(eggRelay,       OUTPUT);
  pinMode(DHT_PIN,        INPUT);
  pinMode(endstop1,       INPUT);
  pinMode(endstop2,       INPUT);
  pinMode(screenButton,   INPUT);
  pinMode(settingsButton, INPUT);

  digitalWrite(fanRelay, on);
  checkStateMachine();

  Serial.begin(115200);
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
}

// the loop routine runs over and over again forever:
void loop() {
  updateDisplay(currentTemp, currentHumidity);      // Update LCD display to show current state or update day, hour, or minutes
  Alarm.delay(250);
}

void checkStateMachine() {    // Alarm which fires every few seconds to check the state machine and see if anything has or needs to change
  updateEnvironment();        // Check temperature and humidity - Do not fire more frequently than once every 2 seconds
}

void checkButtons() {
  screenButtonState   = digitalRead(screenButton);      // BUGBUG: Implement Debounce method here
  settingsButtonState = digitalRead(settingsButton);    // BUGBUG: Implement Debounce method here
  if (screenButtonState == HIGH) {
    screenMode++;
    Alarm.delay(250);
    lcd.clear();
    if (screenMode >=6) {
      screenMode = 1;
    }
  }
  if (settingsButtonState == HIGH) {
    lcd.clear();
  }
}

void turnEggs(){
  if (debug) { 
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("!! Egg Turn Time Event Reached !!");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
  if ((dayCount <= incubationDays)) {
    egg = true;
  } else {
    egg = false;
  }
}

void checkEggTurn() {
  eggSwitch1Status = digitalRead(endstop1);          // BUGBUG: Implement Debounce method here
  eggSwitch2Status = digitalRead(endstop2);          // BUGBUG: Implement Debounce method here

  // check to see if the egg carriage is touching either sensor, if not, turn until sensor 1 is reached
  if ((eggSwitch1Status == 0) && (eggSwitch2Status == 0) && (eggSwitchTarget == 0) ) {    // Initial startup and not at either endstop, move to initial position
    egg = true;
    eggSwitchTarget = 1;   //set target point for next turning to other switch
  }

  if ((egg == true) && (dayCount <= incubationDays)) { 
    if ((eggSwitch1Status == true) && (eggSwitchTarget == 1)) {    // Arrived at target endstop 1, stop turning, set target to endstop 2 for next turn
      if (debug) { Serial.println("Reached egg turning point position 1\t"); }
      egg = false;
      eggSwitchTarget = 2;   //set target point for next turning to other switch
    }

    if ((eggSwitch1Status == false) && (eggSwitchTarget == 1)) {   // Trying to stop at endstop 1, not reached yet, keep turning
      if (debug) { Serial.println("Keep turning eggs until endstop 1 reached\t"); }
      egg = true;
    }

    if ((eggSwitch2Status == true) && (eggSwitchTarget == 2)) {    // Arrived at target endstop 2, stop turning, set target to endstop 1 for next turn
      if (debug) { Serial.println("Reached turning point position 2\t"); }
      egg = false;
      eggSwitchTarget = 1;   //set target point for next turning to other switch
    }

    if ((eggSwitch2Status == false) && (eggSwitchTarget == 2)) {   // Trying to stop at endstop 2, not reached yet, keep turning
      if (debug) { Serial.println("Keep turning eggs until endstop 2 reached\t"); }
      egg = true;
    }
  } 
  else {
    if (dayCount > incubationDays) {
      if (debug) { Serial.println("Hatching"); }
      egg = false;
    }
    if (debug) { Serial.println("Incubating"); }
  }

  if ((egg) && (dayCount <= incubationDays)) {
    if (debug) { Serial.println("Turning Eggs"); }
    digitalWrite(eggRelay, on);
  } else {
    digitalWrite(eggRelay, off);
  }
}

void updateEnvironment() {
  Serial.print("Environment Sensor, \t");
  chk = dht.read22(DHT_PIN);                // Fetch data from sensor - Can only be read once every 2 seoncds, max.
  switch (chk)
  {
  case DHTLIB_OK:  
    Serial.print("OK,\t");                  // If sensor is working:
    checkTemp();                            // Do we need to make changes for temperature
    checkHumidity();                        // Do we need to make changes for humidity level
    checkFan();
    break;
  case DHTLIB_ERROR_CHECKSUM: 
    Serial.print("Checksum error,\t"); 
    heat  = false;
    fan   = false;
    water = false;
    break;
  case DHTLIB_ERROR_TIMEOUT: 
    Serial.print("Time out error,\t");
    heat  = false;
    fan   = false;
    water = false;
    break;
  default: 
    Serial.print("Unknown error,\t"); 
    heat  = false;
    fan   = false;
    water = false;
    break;
  }
}

void checkTemp() {
  currentTemp = c2f(dht.temperature);      // Fetch Temp data
  if (currentTemp < targetTemp) {
    heat = true;
    digitalWrite(heaterRelay, on);
  } 
  else {
    heat = false;
    digitalWrite(heaterRelay, off);
  }
}

void checkHumidity() {
  currentHumidity = dht.humidity;          // Fetch Humidity data
  if (currentHumidity < targetHumidity) {
    water = true;
    digitalWrite(waterRelay, on);
  } 
  else {
    water = false;
    digitalWrite(waterRelay, off);
  }
}

void checkFan() {
  fan = true;
  digitalWrite(fanRelay, on);              // Is there any reason we would ever want to turn the fan off?
}

void trackDays() {
  if (debug) { 
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("!! Day Increment               !!");         
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
  dayCount++;
  checkIncubation();
}

void checkIncubation() {
  if (dayCount <= incubationDays) {
    targetHumidity = incubationHumidity;
    targetTemp     = incubationTemp;
  } 
  else {
    targetHumidity = hatchingHumidity;
    targetTemp     = hatchingTemp;
    egg            = false;
  }
}

double c2f(double temp) {                  // °C to °F 	Multiply by 9, then divide by 5, then add 32
  temp = ((dht.temperature * 9) / 5) + 32;
  return temp;
}

void updateDisplay(int temp, int humid) {
  checkButtons();

  if (heat || water || egg) {              // Switch LED on if temp or humidity is low
    digitalWrite(led, HIGH);               // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(led, LOW);
  }

  switch (screenMode) {
    case 1:                                // Standard Runmode
      lcd.setCursor(0, 0);
      lcd.print("T");
      lcd.setCursor(1, 0);
      lcd.print(temp);

      lcd.setCursor(5, 0);
      lcd.print("H");
      lcd.setCursor(6, 0);
      lcd.print(humid);
    
      if (debug) {
        Serial.print("Temp: ");
        Serial.print(currentTemp, 1);
        Serial.print("Humidity: ");
        Serial.println(currentHumidity, 1);
      }

      lcd.setCursor(9, 0);                 // Update status for Egg Type of D(uck) or C(hicken)
      if (duck) {
        lcd.print("D");
        Serial.println("Incubation Egg Mode: Duck");
      } else {
        lcd.print("C");
        Serial.println("Incubation Egg Mode: Chicken");
      }
    
      lcd.setCursor(10, 0);                // Update status for relays and endstops
      lcd.print(eggSwitch1Status);
      lcd.setCursor(11, 0);
      lcd.print(eggSwitch2Status);
    
      lcd.setCursor(12, 0);                // Update status for Heater
      if (heat) {
        lcd.print("H");
        if (debug) { Serial.println("Heater: ON"); }
      } 
      else {
        lcd.print("_");
        if (debug) { Serial.println("Heater: OFF"); }
      }
    
      lcd.setCursor(13, 0);                // Update status for Fan
      if (fan) {
        lcd.print("F");
        if (debug) { Serial.println("Fan: ON"); }
      }
      else {
        lcd.print("_");
        if (debug) { Serial.println("Fan: OFF"); }
      }
    
      lcd.setCursor(14, 0);                // Update status for Water
      if (water) {
        lcd.print("W");
        if (debug) { Serial.println("Water: ON"); }
      } 
      else {
        lcd.print("_");
        if (debug) { Serial.println("Water: OFF"); }
      }
    
      lcd.setCursor(15, 0);                // Update status for Egg turning
      if (egg) {
        lcd.print("E");
        if (debug) { Serial.println("Turning Eggs"); }
      }
      else {
        lcd.print("_");
        if (debug) { Serial.println("Do NOT Turn Eggs"); }
      }
      digitalClockDisplay();
      Alarm.delay(100);
      break;
    case 2:                                  // Set HOUR
      while (screenButtonState == LOW) {
        checkButtons();
        lcd.setCursor(0,0);
        lcd.print("Set Hour: ");
        lcd.setCursor(10,0);
        lcd.print(hour());
        if (settingsButtonState == HIGH) {
          lcd.clear();
          // increment hour +1
          int currentHour = hour();
          int currentMinute = minute();
          currentHour++;
          if (currentHour >= 24) {
            currentHour = 0;
          }
          setTime(currentHour,currentMinute,0,6,1,14); // set time to time +1 hour
        }
        digitalClockDisplay();
        Alarm.delay(100);
      }
      break;
    case 3:                                  // Set MINUTE
      while (screenButtonState == LOW) {
        checkButtons();
        lcd.setCursor(0,0);
        lcd.print("Set Minute: ");
        lcd.setCursor(12,0);
        lcd.print(minute());
        if (settingsButtonState == HIGH) {
          lcd.clear();                       // increment minute +1
          int currentHour = hour();
          int currentMinute = minute();
          currentMinute++;
          if (currentMinute >= 60) {
            currentMinute = 0;
          }
          setTime(currentHour,currentMinute,0,6,1,14); // set time to time +1 minute
        }
        digitalClockDisplay();
        Alarm.delay(100);
      }
      break;
    case 4:                                  // Set DAY
      while (screenButtonState == LOW) {
        checkButtons();
        lcd.setCursor(0,0);
        lcd.print("Set Day: ");
        lcd.setCursor(9,0);
        lcd.print(dayCount);
        if (settingsButtonState == HIGH) {
          // increment minute +1
          lcd.clear();
          dayCount++;
          if (dayCount >= (incubationDays + 3)) {
            dayCount = 1;
          }
        }
        digitalClockDisplay();
        Alarm.delay(100);
      }
      break;
    case 5:                                  // Toggle between Duck and Chicken incubation modes
      while (screenButtonState == LOW) {
        checkButtons();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Egg Type:");
        lcd.setCursor(0,1);
        if (settingsButtonState == HIGH) {
          duck = !duck;                      // Toggle value between duck and chicken
        }
        // ------------------------------------------------------------------------------------------
        // --- VALUES TO USE FOR CHICKEN OR DUCK INCUBATION AND HATCHING DAYS, TEMP, AND HUMITITY ---
        // ------------------------------------------------------------------------------------------
        //			DUCKS
        // Phase:		Incubation	Hatching
        // Days:		1-25		26-28
        // Temperature:		99.5		98.5
        // Humidity:		86		94
        // Turns Per Day:	3-7		0
        // ------------------------------------------------------------------------------------------
        //			CHICKENS
        // Phase:		Incubation	Hatching
        // Days:		1-18		19-21
        // Temperature:		99.5		98.5
        // Humidity:		50		65
        // Turns Per Day:	3-7		0
        // ------------------------------------------------------------------------------------------
        if (duck) {
          lcd.print("Ducks   ");
          incubationDays     = 25;
          incubationTemp     = 99.5;
          incubationHumidity = 86;
          hatchingTemp       = 98.5;
          hatchingHumidity   = 94;
        } else {                    // If not DUCK Eggs, then we must be incubating CHICKEN Eggs
          lcd.print("Chickens");
          incubationDays     = 18;
          incubationTemp     = 99.5;
          incubationHumidity = 50;
          hatchingTemp       = 98.5;
          hatchingHumidity   = 65;
        }
        checkIncubation();
        Alarm.delay(100);
      }
      break;
    default:                                 // ERROR - SCRAM the Incubator
      Serial.println ("ERROR: Unknown State for Display! SHUTDOWN!");
      lcd.setCursor(0,0);
      lcd.print("ERR Display Mode");
      lcd.setCursor(0,1);
      lcd.print("SCRAM Incubator!");
      egg   = false;
      heat  = false;
      fan   = false;
      water = false;
      break;
  }
  Alarm.delay(100);
}

void digitalClockDisplay(){                  // digital clock display of the time
  lcd.setCursor(0, 1);
  lcd.print("D");
  Serial.print("Day: ");

  lcd.setCursor(1,1);
  if (dayCount > 9) {
    lcd.print(dayCount);
  } 
  else {
    lcd.print("0");
    lcd.print(dayCount);
  }
  Serial.print(dayCount);
  Serial.print(" ");

  lcd.setCursor(4,1);
  if (hour() < 10) {
    lcd.print("0");
    Serial.print("0");
  }
  lcd.print(hour());
  Serial.print(hour());

  lcd.setCursor(6,1);
  lcd.print(":");
  Serial.print(":");

  lcd.setCursor(7,1);
  if (minute() < 10) {
    lcd.print("0");
    Serial.print("0");
  }
  lcd.print(minute());
  Serial.print(minute());

  lcd.setCursor(9,1);
  lcd.print(":");
  Serial.print(":");

  lcd.setCursor(10,1);
  if (second() < 10) {
    lcd.print("0");
    Serial.print("0");
  }
  lcd.print(second());
  Serial.println(second());

  lcd.setCursor(13,1);
  if (dayCount <= incubationDays) {
    lcd.print("Inc");
    Serial.print("Incubation Mode");
  } 
  else {
    lcd.print("Hat");
    Serial.print("Hatching Mode");
  }
  Serial.println();
}

