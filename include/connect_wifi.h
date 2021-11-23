#ifndef CWIFI_H_   /* Include guard */
#define CWIFI_H_

#include <WiFi.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/* Serial */
#define BAUD 9600

/* Cases */
#define ROOT 0
#define IP_WIDTH 16
#define MAC_WIDTH 30
#define NUM_BANANAS 2

#define MAC_A "A4:CF:12:54:DD:D4"
#define MAC_B "3C:71:BF:EA:B6:A8"
#define MAC_C "3C:71:BF:EA:B6:A8"

#define IP_A "172.20.10.1"
#define IP_B "172.20.10.2"
#define IP_C "172.20.10.3"

typedef uint8_t node;
/*node A = 0, B = 1, C = 2;*/

typedef struct {
    node A;
    node B;
    node C;
} route_t;

typedef struct {
    uint8_t ip_id;
    char macaddr[MAC_WIDTH];
    node monkey;
    node banana[NUM_BANANAS];
    route_t *route;
} node_t;



/**
 * Establish a WiFi Connection
 * return: true is success, false in case of timeout (15sec)
 * */
boolean ConnectWifi(const char* ssid, const char* pass);

/**
 * Sets network settings
 * */
boolean SetAddress(IPAddress ip, IPAddress gateway, IPAddress subnet);

#endif