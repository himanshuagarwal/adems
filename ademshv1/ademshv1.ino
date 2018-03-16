#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <TinyGPS.h>

TinyGPS gps;
SFE_BMP180 pressure;

String ssid = "Red";
String password = "12345678";
String data;
String server = "flipdd.in";
String uri = "/adems/send.php";
String temp , hum, pres;
float lat, lon;
float coord[2];

int DHpin = 8;
byte dat [5];

SoftwareSerial esp(6, 7);// RX, TX
SoftwareSerial ss(3, 4); // RX, TX

void setup() {

  Serial.begin(9600);

  // DHT11 pin Initialization
  pinMode (DHpin, OUTPUT);
  Serial.println("DHT11 Initialized - Checkpoint 1");

  // BMP180 connection status
  if (pressure.begin())
    Serial.println("BMP180 init success - Checkpoint 2");
  else
    Serial.println("BMP180 init fail");

  // GPS module initialization
  ss.begin(4800);
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

// Bus read for DHT11 IC
byte read_data () {
  byte data;
  for (int i = 0; i < 8; i ++) {
    if (digitalRead (DHpin) == LOW) {
      while (digitalRead (DHpin) == LOW); // wait for 50microseconds
      delayMicroseconds (30); // determine the duration of the high level to determine the data is '0 'or '1'
      if (digitalRead (DHpin) == HIGH)
        data |= (1 << (7 - i)); // high front and low in the post
      while (digitalRead (DHpin) == HIGH);
      // data '1 ', wait for the next one receiver
    }
  } return data;
}

// Start output DHT11
void start_test () {
  digitalWrite (DHpin, LOW); // bus down, send start signal
  delay (30); // delay greater than 18ms, so DHT11 start signal can be detected
  digitalWrite (DHpin, HIGH);
  delayMicroseconds (40); // Wait for DHT11 response
  pinMode (DHpin, INPUT);
  while (digitalRead (DHpin) == HIGH);
  delayMicroseconds (80);

  // DHT11 response, pulled the bus 80us

  if (digitalRead (DHpin) == LOW);
  delayMicroseconds (80);

  // DHT11 80us after the bus pulled to start sending data

  for (int i = 0; i < 4; i ++)
    dat[i] = read_data ();

  pinMode (DHpin, OUTPUT);
  digitalWrite (DHpin, HIGH);

  // send data once after releasing the bus, wait for the host to open the next Start signal
}

void gpsLoop(float &lat, float &lon)
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
    unsigned long age;
    gps.f_get_position(&lat, &lon, &age);
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
  start_test ();
  hum = String(dat[0]);
  temp = String(dat[2]);
  Serial.println("DHT11 Recevied");

  // BMP180 data
  pres = String(bmpLoop());
  Serial.println("BMP180 Recevied");

  // GPS data
  //int gpsStatus = gpsLoop(&lat, &lon);
  Serial.println("GPS Recevied");
  
  data = "temp=" + temp + "&humi=" + hum + "&pres=" + pres + "&lat=" + lat + "&lon=" + lon;
  // data sent must be under this form name1=value1&name2=value2.
  Serial.println(data);
  //httppost();
  
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
