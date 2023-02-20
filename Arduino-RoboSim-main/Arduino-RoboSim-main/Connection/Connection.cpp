#include "Connection.h"

Connection::Connection(const char* ssid, const char* passwd, String ip, String port){
    SSID = ssid;
    PASSWD = passwd;
    IP = ip;
    PORT = port;
    BASEURL = "http://" + IP + ":" + PORT + "/v2/entities/Map001/attrs/cell";
}

void Connection::connectWifi(){
  wifiMulti.addAP(SSID, PASSWD);  //WiFi connection
}

boolean Connection::connectedWifi(){
   return wifiMulti.run() == WL_CONNECTED; 
}

void Connection::myLocation(int line, int column){    
 //HTTPClient http;    //Declare object of class HTTPClient
  int httpCode; 

  //getting north status
  String url = BASEURL + String(line - 1) + String(column) + "/value";
  http.begin(url);
  httpCode = http.GET();
  if(httpCode == 200){
    neighbors[0] = http.getString().toInt();
  }
  else
  {
    neighbors[0] = -1;
  }
  
  //getting east status
  url = BASEURL + String(line) + String(column + 1) + "/value";
  http.begin(url);
  httpCode = http.GET();
  if(httpCode == 200){
    neighbors[1] = http.getString().toInt();
  }
  else
  {
    neighbors[1] = -1;
  }

  //getting wets status
  url = BASEURL + String(line) + String(column - 1) + "/value";
  http.begin(url);
  httpCode = http.GET();
  if(httpCode == 200){
    neighbors[2] = http.getString().toInt();
  }
  else
  {
    neighbors[2] = -1;
  }

 //getting south status
  url = BASEURL + String(line + 1) + String(column) + "/value";
  http.begin(url);
  httpCode = http.GET();
  if(httpCode == 200){
    neighbors[3] = http.getString().toInt();
  }
  else
  {
    neighbors[3] = -1;
    coordinatesAck = false;
  }
  coordinatesAck = true;
  http.end();
}

int *Connection::coordinatesFiware(){
  return neighbors;
}

boolean Connection::coordinatesFiwareAck(){
  return coordinatesAck;
}

void Connection::updateFiware(int line, int column, int state){
  if (wifiMulti.run() == WL_CONNECTED) { //Check WiFi connection status
    
    HTTPClient http;    //Declare object of class HTTPClient
    String url = BASEURL + String(line) + String(column) + "/value";
    http.begin(url);      //Specify request destination
    http.addHeader("Content-Type", "text/plain");    //Specify content-type header
    updateHttpCode = http.PUT(String(state));  //Send the request
    http.end();  //Close connection
  }
}

boolean Connection::updateAck(){
  if(updateHttpCode == 204){
    return true;
  }
  return false;
}

/*
void Connection::debug(String msg){
  if(DEBUG){
    Serial.println(msg);
  }
}
*/
/*
String Connection::getCell(int line, int column){
    if(wifiMulti.run() == WL_CONNECTED){
        int httpCode; 

        //getting north status
        String url = BASEURL + String(line) + String(column) + "/value";
        http.begin(url);
        httpCode = http.GET();
        return http.getString();
    }
  
}
*/