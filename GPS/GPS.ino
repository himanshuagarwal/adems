#include <SoftwareSerial.h>

#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

TinyGPS gps;
SoftwareSerial ss(4, 3);
char lat[20],lon[20];

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
}

void loop()
{
  bool newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c))
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    if (flat != TinyGPS::GPS_INVALID_F_ANGLE)
      dtostrf(flat, 9, 6 ,lat);
      Serial.println(String(lat));
    if (flon != TinyGPS::GPS_INVALID_F_ANGLE)
      dtostrf(flon, 9, 6 ,lon);
      Serial.println(String(lon));
  }
}
