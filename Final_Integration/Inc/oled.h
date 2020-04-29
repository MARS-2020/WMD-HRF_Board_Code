/*
 * oled.h
 *
 *  Created on: Apr 17, 2020
 *      Author: Morgan
 */

#ifndef __OLED_H_
#define __OLED_H_
//#include "fonts.h"
//#define contrast 0x81
	void turnOnScreen();
	void setupScreen();
	void sendCMD(uint8_t *cmd, uint16_t size);
	void sendDATA(uint8_t *data, uint16_t size);
	void sendString(char *string, uint8_t header);
	void clearScreen();
	void updateScreen(char* hr, char* spo2, char* distance, char* direction, char* user);
	void setUserName(char* userName);
	void sendSOS();
	void clearSOS();
	void user1Info(uint8_t hr, uint8_t spo2);
	void displayActiveHR(void);
	void removeActiveHR(void);
#endif
