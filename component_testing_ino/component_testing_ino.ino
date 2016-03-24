//
//    FILE: dht22_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.01
// PURPOSE: DHT library test sketch for DHT22 && Arduino
//     URL:
//
// Released to the public domain
//

#include <Time.h>
#include <dht.h>

dht dht;

#define DHT_PIN            6
#define heaterRelay        8
#define fanRelay           9


boolean heat             = false;
boolean fan              = false;
// Tracking variables

double targetTemp        = 99.5;      // 35c == 95F
double targetHumidity    = 50;      // 50% humidity

double currentHumidity   = 0;
double currentTemp       = 0;

void setup()
{
  pinMode(heaterRelay, OUTPUT);     
  pinMode(fanRelay,    OUTPUT);     
  Serial.begin(115200);
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  //Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C),\tTemperature (F)\tTime (us)");
  Serial.println("status\tHumid%\tTempC\tTempF\tDate-Time");
  setTime(21,36,0,16,6,14); // set time to noon Jan 1 2011
}

void loop()
{
  // READ DATA
  //Serial.print("DHT22, \t");

  uint32_t start = micros();
  int chk = dht.read22(DHT_PIN);
  uint32_t stop = micros();

  switch (chk)
  {
  case DHTLIB_OK:
    Serial.print("OK\t");
    // DISPLAY DATA
    Serial.print(dht.humidity, 1);
    Serial.print("\t");
    Serial.print(dht.temperature, 1);
    Serial.print('C');
    Serial.print("\t");
    currentTemp = c2f(dht.temperature);
    Serial.print(currentTemp, 1);
    Serial.print('F');
    //Serial.print(stop - start);
    Serial.print("\t");
    //Serial.println();
    break;
  case DHTLIB_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHTLIB_ERROR_TIMEOUT:
    Serial.print("Time out error,\t");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }

  digitalClockDisplay();
  checkTemp();

  delay(2000);
  digitalWrite(heaterRelay, LOW);

  delay(2000);

}


double c2f(double temp) {
  // °C to °F 	Multiply by 9, then divide by 5, then add 32
  temp = ((temp * 9) / 5) + 32;
  return temp;
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(month());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits){
  // utility function for clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void checkTemp() {
  if (currentTemp < targetTemp) {
    Serial.print("Temp is low, target temp: ");
    Serial.print(targetTemp, 1);
    Serial.println(", enable heater and fan\t");
    heat = true;
    digitalWrite(heaterRelay, HIGH);
    fan = true;
    digitalWrite(fanRelay, LOW);
  } 
  else {
    Serial.println("Temp is high, disable heater\t");
    heat = false;
    digitalWrite(heaterRelay, LOW);
    fan = false;
    digitalWrite(fanRelay, HIGH);
  }
}
//
// END OF FILE
//

