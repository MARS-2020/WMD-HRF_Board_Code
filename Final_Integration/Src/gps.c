/*
 * gps.c
 *
 *  Created on: Apr 21, 2020
 *      Author: Lucas Pfeiffer
 */

#include "gps.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern uint8_t data[128];
extern UART_HandleTypeDef huart1;
extern char latitude[10];
extern char longitude[11];
extern char longDir[2];
extern char latDir[2];
extern char fix[2];
extern char sendMeasurements[27];

uint8_t GPS_BAUDRATE[] = "$PMTK251,9600*17\r\n";
uint8_t GPS_FIXRATE[] = "$PMTK300,1000,0,0,0,0*1C\r\n";
//uint8_t GPS_FIXRATE[] = "$PMTK300,10000,0,0,0,0*2C\r\n";
uint8_t GPS_GGA[] = "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
uint8_t GPS_1HZ[] = "$PMTK220,1000*1F\r\n";
uint8_t GPS_5HZ[] = "$PMTK220,200*2C\r\n";
uint8_t GPS_10HZ[] = "$PMTK220,100*2F\r\n";
uint8_t GPS_STANDBY[] = "$PMTK161,0*28\r\n";

void GPS_INIT(void)
{
	  HAL_UART_Transmit(&huart1, GPS_BAUDRATE, sizeof(GPS_BAUDRATE), 1000); //set baud rate
	  //HAL_UART_Transmit(&huart1, GPS_FIXRATE, sizeof(GPS_FIXRATE), 1000); //set fix rate
	  HAL_UART_Transmit(&huart1, GPS_1HZ, sizeof(GPS_1HZ), 1000); //set frequency
	  HAL_UART_Transmit(&huart1, GPS_GGA, sizeof(GPS_GGA), 1000); //set sentence format
}

uint8_t parseData(void)
{
	char string[7];
	int commaCnt = 0;
	int i = 0;
	int j = 0;

	memcpy(string, data, 6);

	if(!strcmp(string, "$GPGGA"))
	{
		i = 6; //since valid message type, start one character after "A"
		while(data[i] != '\n')
		{
			if (data[i] == ',')
			{
				commaCnt++; //increase number of commas found
				j = 0; //reset counter for parsing
			}
			else
			{
				if(commaCnt == 2) //latitude
				{
					latitude[j] = data[i];
					j++;
				}
				else if(commaCnt == 3) //latitude direction
				{
					latDir[j] = data[i];
					j++;
				}
				else if(commaCnt == 4) //longitude
				{
					longitude[j] = data[i];
					j++;
				}
				else if(commaCnt == 5) //longitude direction
				{
					longDir[j] = data[i];
					j++;
				}
				else if(commaCnt == 6)
				{
					fix[j] = data[i];
					j++;
				}
				else if (commaCnt >= 7)
				{
					break;
				}
			}
			i++; //increase element counter
		}

		if(fix[0] == '0') //no fix
		{
			return 0;
		}

	}
	else
	{
		return 0;
	}

	latitude[9] = '\0';
	latDir[1] = '\0';
	longitude[10] = '\0';
	longDir[1] = '\0';
	fix[1] = '\0';

	strcpy(sendMeasurements, latitude);
	strcat(sendMeasurements, ",");
	strcat(sendMeasurements, latDir);
	strcat(sendMeasurements, ",");
	strcat(sendMeasurements, longitude);
	strcat(sendMeasurements, ",");
	strcat(sendMeasurements, longDir);
	strcat(sendMeasurements, ",");
	strcat(sendMeasurements, fix);

	return 1;
}
