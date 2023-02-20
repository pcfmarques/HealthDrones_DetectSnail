#include <Connection.h>

Connection connection("ERALUJA","83231749","192.168.1.80","1026", "controlePorMensageria");

void setup() {
  Serial.begin(115200);
  for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    
    connection.connectWifi();
    Serial.println("Conectando");
    Serial.printf("connectAck %i",connection.connectWifiAck());
    delay(1000);
    if(connection.connectWifiAck()){
      Serial.println("entrou no if");
      Serial.println("Connected");
      
    }
    delay(500);
}

void loop() {
  Serial.println("Sending t...");
  connection.droneControl('t');
  Serial.println(connection.droneResponse());
  delay(10000);
  Serial.println("Sending g...");
  connection.droneControl('g');
  Serial.println(connection.droneResponse());
  delay(5000);
  Serial.println("Sending l...");
  connection.droneControl('l');
  Serial.println(connection.droneResponse());
  delay(20000);
}
