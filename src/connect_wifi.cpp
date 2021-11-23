#include "connect_wifi.h"


boolean ConnectWifi(const char* ssid, const char* pass) {

    uint32_t time_conter = 0;

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);

      time_conter += 1000;
      if (time_conter > 15000) 
        return false;

      Serial.println("Establishing connection to WiFi...");
    }
    
    Serial.print("Connected to network: ");
    Serial.print(WiFi.localIP());
    Serial.print(" | ");
    Serial.println(WiFi.macAddress());

    return true;
}

boolean SetAddress(IPAddress ip, IPAddress gateway, IPAddress subnet) {

    bool result = WiFi.config(ip, gateway, subnet);

    if (!result) {
      Serial.println("Node was not set correctly.");
      return false;
    }

    Serial.print("Node is well set: ");
    Serial.println(WiFi.localIP());
    return true;

}

