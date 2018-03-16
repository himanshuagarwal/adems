#include <dht.h>
#include <Wire.h>

dht DHT;
#define DHT11_PIN 8

void setup() {
  Serial.begin(9600); 

}

void loop() {
int chk = DHT.read11(DHT11_PIN);
Serial.print("\ttemperature= ");
Serial.print(DHT.temperature);
Serial.print("\thumidity= ");
Serial.println(DHT.humidity);
delay(1000);
}
