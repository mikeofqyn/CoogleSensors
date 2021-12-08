#pragma once
#ifndef _CS_STATIC_IPS_H
#define _CS_STATIC_IPS_H
/*
 * Table of MAC addresses and corresponding static IP configurartions
 * ==================================================================
 * 
 * Calling cs_get_default_static_ip() will return a pointer to a
 * csStaticConfigEntry struct, whose member can directly be used to
 * call WiFi.config() or CoogleIOT::setStaticAddress()
 * 
 * If there is no static configuration for tha MAC address given
 * then NULL is returned.
 * 
 * This version uses a block of consecutive addresses which must be
 * reserved in the router. Starting address is given by the symbol
 * CS_FIRST_ADDR 
 *
 **/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "THIS LIBRARY IS FOR ESP32/ESP8266 DEVICES"
#endif

typedef struct CS_STATIC_CONFIG_s {
    char mac_addr[18];      // device MAC address (null terminatd with semicolons)
    IPAddress  address;   // assigned IP address
    IPAddress  gateway;   // assigned default gateway
    IPAddress  subnet;    // subnet mask 
    IPAddress  dns1;      // DNS server 1
    IPAddress  dns2;      // DNS server 2
} csStaticConfigEntry;

// Make address using default network  192.168.1.0/24

#define CS_MKADDR(n) IPAddress(192,168,1,n) 

const IPAddress CS_MASK = IPAddress(255,255,255,0);
const IPAddress CS_GW = CS_MKADDR(1);

// Google default DNS Servers
const IPAddress CS_DNS_1 = IPAddress(8,8,8,8);
const IPAddress CS_DNS_2 = IPAddress(8,8,4,4);

// Create table entry  (see cpp file for implementation)
#define CS_FIRST_ADDR 180  // Addresses set aside in router from this onwards. Be careful not to overflow

#define CS_MKENTRY(mac,offset) { mac, CS_MKADDR(offset + CS_FIRST_ADDR), CS_GW, CS_MASK, CS_DNS_1, CS_DNS_2 }

csStaticConfigEntry* cs_GetStaticConfig(const char *mac_str);

 csStaticConfigEntry* cs_GetStaticConfig(String mac);

 csStaticConfigEntry* cs_GetStaticConfig();

#endif //  _CS_STATIC_IPS_H  