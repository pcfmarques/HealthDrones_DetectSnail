#ifndef Connection_h
#define Connection_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>


class Connection {
    public:
        Connection(const char* ssid, const char* passwd, String ip, String port);
        void connectWifi(); 
        boolean connectedWifi(); // connectWifiAck()
        //myLocation -> getNeighbours(lin,column)
        // coordinatesFiware -> getNeighboursAck()
        // coordinatesFiware -> neighbours()
        void myLocation(int line, int column);
        boolean coordinatesFiwareAck(); 
        int *coordinatesFiware();  
        void updateFiware(int line, int column, int state); // updateState
        boolean updateAck(); // updateStateAck()

    private:
        WiFiMulti wifiMulti;
        HTTPClient http;
        const char* SSID;
        const char* PASSWD;
        String IP;
        String PORT;
        String BASEURL;
        int neighbors[4];
        int updateHttpCode;
        boolean coordinatesAck;
};

#endif