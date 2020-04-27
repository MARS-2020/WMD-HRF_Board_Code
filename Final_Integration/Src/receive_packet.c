#include <stdio.h>
#include <string.h>
#include "receive_packet.h"
#include "oled.h"
#include "lora.h"

uint8_t device_number = '1';
uint8_t user_set[] = "User Data Set!\r\n";
extern uint8_t NAME_RECEIVED;
extern uint8_t LORA_RECEIVED;

void parse_packet(uint8_t* buffer) {
    if(buffer[2] != device_number) return;

    if(buffer[0] == 'z') { //User Packet
        char userName[15];
        memcpy(&userName, &buffer[4], 15);
        setUserName(userName);
        NAME_RECEIVED = 1;
        //sendPacket(user_set, sizeof(user_set));
    }
    
    if(buffer[0] == 'x') { //Info Packet
        char hr[4];
        char spo2[4];
        char distance[6];
        hr[3] = '\0';
        spo2[3] = '\0';
        char user[1];
        user[0] = '2';
        memcpy(&hr, &buffer[4], 3);
        memcpy(&spo2, &buffer[8], 3);
        memcpy(&distance, &buffer[12], 4);
        memcpy(&distance[4], &buffer[17], 2);
        updateScreen(hr, spo2, distance, user);
        LORA_RECEIVED = 1;
    }
}




