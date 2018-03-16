#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <TinyGPS.h>
#include <dht.h>

TinyGPS gps;
SFE_BMP180 pressure;

String ssid = "Red";
String password = "12345678";
String data;
String server = "flipdd.in";
String uri = "/adems/send.php";
String pres;
double Temp1, Humi;
char lat[20], lon[20];

dht DHT;
#define DHT11_PIN 8

SoftwareSerial esp(6, 7);// RX, TX
SoftwareSerial ss(4, 3); // RX, TX

void setup() {

  Serial.begin(9600);

  // DHT11 pin Initialization
  int chk = DHT.read11(DHT11_PIN);
  Serial.println("DHT11 Initialized - Checkpoint 1");

  // BMP180 connection status
  if (pressure.begin())
    Serial.println("BMP180 init success - Checkpoint 2");
  else
    Serial.println("BMP180 init fail");

  // GPS module initialization
  ss.begin(9600);
  Serial.println("GPS module initialized - Checkpoint 3");

  // ESP8266 Connection
  esp.begin(115200);
  reset();
  connectWifi();
  Serial.println("Wifi Connected - Checkpoint 4");
}

//Reset ESP8266
void reset() {
  esp.println("AT+RST");
  delay(1000);
  if (esp.find("OK") )
    Serial.println("Module Reset");
}

//Connect to wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  if (esp.find("OK")) {
    Serial.println("Connected!");
  }
  else {
    connectWifi();
    Serial.println("Cannot connect to wifi");
  }
}

void dht() {

  int chk = DHT.read11(DHT11_PIN);
  Temp1 = DHT.temperature;
  Humi = DHT.humidity;

}

void gpsLoop()
{
  ss.begin(9600);
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
      dtostrf(flat, 9, 6 , lat);
    //Serial.println(String(lat));
    if (flon != TinyGPS::GPS_INVALID_F_ANGLE)
      dtostrf(flon, 9, 6 , lon);
    //Serial.println(String(lon));
  }
}



float bmpLoop()
{
  char status;
  double T, P;

  status = pressure.startPressure(3);
  if (status != 0)
  {
    delay(status);
    status = pressure.getPressure(P, T);
    if (status != 0)
      return P;
  }
  return 0;
}

void loop () {

  // DHT11 data
  dht();
  //Serial.println("DHT11 Recevied");

  // BMP180 data
  pres = String(bmpLoop());
  //Serial.println("BMP180 Recevied");

  // GPS data
  gpsLoop();
  //Serial.println("GPS Recevied");


  if (String(lat) == "")
    data = "temp=" + String(Temp1) + "&humi=" + String(Humi) + "&pres=" + pres;
  else
    data = "temp=" + String(Temp1) + "&humi=" + String(Humi) + "&pres=" + pres + "&lat=" + String(lat) + "&lon=" + String(lon);
  
  Serial.println(data);
  httppost();
  delay(1000);
}

void httppost () {

  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.

  if ( esp.find("OK")) {

    Serial.println("TCP connection ready");

  } delay(1000);

  String postRequest =

    "POST " + uri + " HTTP/1.0\r\n" +

    "Host: " + server + "\r\n" +

    "Accept: *" + "/" + "*\r\n" +

    "Content-Length: " + data.length() + "\r\n" +

    "Content-Type: application/x-www-form-urlencoded\r\n" +

    "\r\n" + data;

  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.

  esp.print(sendCmd);

  esp.println(postRequest.length() );

  delay(500);

  if (esp.find(">")) {
    Serial.println("Sending.."); esp.print(postRequest);

    if ( esp.find("SEND OK")) {
      Serial.println("Packet sent");

      while (esp.available()) {

        String tmpResp = esp.readString();

        Serial.println(tmpResp);

      }

      // close the connection

      esp.println("AT+CIPCLOSE");

    }

  }
}
