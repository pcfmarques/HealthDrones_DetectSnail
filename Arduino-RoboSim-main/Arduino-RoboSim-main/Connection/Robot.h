#ifndef Robot_h
#define Robot_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

class Robot {
    public:
        Robot(const char* ssid, const char* passwd, String ip, String port);
        Robot(const char* ssid, const char* passwd, String ip, String port, int line, int column);
        Robot(const char* ssid, const char* passwd, String ip, String port, int line, int column, int orientation);
        void connect();
        boolean isConnected();
        void setLocation(int line, int column);
        int *getNeighbours();
        void updateLocation(int line, int column, int state);
        boolean isUpdated();
        int getNorthStatus();
        int getEastStatus();
        int getSouthStatus();
        int getWestStatus();
	    void goToNorth();
        void goToEast();
        void goToSouth();
        void goToWest();
        void setOrientation(int orientation);
        int getFrontStatus();
        int getLeftStatus();
        int getRightStatus();
        int getBackStatus();
        void turnRight();
        void turnLeft();
        void goFoward(int steps);
        void goBackward(int steps);
        
    private:
        WiFiMulti wifiMulti;
        HTTPClient http;
        const char* SSID;
        const char* PASSWD;
        String IP;
        String PORT;
        String BASEURL;
        int line;
        int column;
        int neighbors[4];
        int updateHttpCode; 
};

#endif