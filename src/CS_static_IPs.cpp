#include "CS_static_IPs.h"

static csStaticConfigEntry CS_STATIC_CONFIG[] =  {
    CS_MKENTRY("BC:FF:4D:2A:C1:DF",  0),  // 192.168.1.180
    CS_MKENTRY("BC:FF:4D:2A:54:8F",  1),  // 192.168.1.181
    CS_MKENTRY("BC:FF:4D:29:94:8D",  2),  // ...
    CS_MKENTRY("BC:FF:4D:2A:C1:E8",  3),
    CS_MKENTRY("84:F3:EB:4B:FD:97",  4), 
    CS_MKENTRY("BC:FF:4D:2A:6B:55",  5),
    CS_MKENTRY("BC:FF:4D:2A:90:42",  6),
    CS_MKENTRY("BC:FF:4D:2A:68:03",  7),
    CS_MKENTRY("BC:FF:4D:2B:7A:06",  8),   
    CS_MKENTRY("44:17:93:0D:B6:C5",  9),
    CS_MKENTRY("EC:FA:BC:C5:11:D6", 10)    // ESP-12 HUB
};

const unsigned int CS_NUM_ENTRIES = sizeof(CS_STATIC_CONFIG)/sizeof(csStaticConfigEntry);

csStaticConfigEntry* cs_GetStaticConfig(const char *mac_str) {
    if (mac_str) {
        for (int i=0; i<CS_NUM_ENTRIES; i++) {
            if (!strcmp(mac_str, CS_STATIC_CONFIG[i].mac_addr)) {
                return &(CS_STATIC_CONFIG[i]);
            }
        }
    }
    return NULL;
}

csStaticConfigEntry* cs_GetStaticConfig(String mac) {
    return cs_GetStaticConfig(mac.c_str());
}

csStaticConfigEntry* cs_GetStaticConfig() {
    return cs_GetStaticConfig(WiFi.macAddress());
}

