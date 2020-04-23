/*
 * oled.c
 *
 *  Created on: Apr 17, 2020
 *      Author: Morgan
 */


#include "main.h"
#include "oled.h"
#include "fonts.h"
extern SPI_HandleTypeDef hspi2;

extern char HR_Valid[4];
extern char SPO2_Valid[4];

uint8_t isSelfSetup = 1;
uint8_t isOtherSetup = 1;
char hr[]="000";
char spo2[] = {'0','0','0'};
char distance[]="N000";
char user[]={'1'};
char hrnew[] = "50";
char spo2new[] = "89";
char fixnew[] = "0";

void turnOnScreen(){
HAL_GPIO_WritePin(oled_RES_GPIO_Port, oled_RES_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(oled_NSS_GPIO_Port, oled_NSS_Pin, GPIO_PIN_SET);

	uint8_t turnOn[] = {0xA8, 0x3F, 0xD3, 0x00, 0x20,0x10, 0xAF, 0xAC};// 0xAF}; //need to change
	uint8_t orientation[]={0xC8, 0xA1};
	 sendCMD(turnOn, (uint16_t)sizeof(turnOn));
	  sendCMD(orientation, (uint16_t)sizeof(orientation));


	  sendDATA(MARSBMP, (uint16_t)sizeof(MARSBMP));
}

void setupScreen(){


	uint8_t page[] = {0x22, 0x00,0x00};

	uint8_t col[]= {0x21, 0x00, 0x7F};
	clearScreen();
	sendCMD(page,(uint16_t)sizeof(page));

	sendCMD(col, (uint16_t)sizeof(col));
	char* message = "   @*@    %          ";
	sendString(message,0x00);
	page[1]=0x01;
	page[2]=0x01;
	col[1]=0x00;
	col[2]=0x7F;
	sendCMD(page,(uint16_t)sizeof(page));
	sendCMD(col, (uint16_t)sizeof(col));
	message="                     ";
	sendString(message,0x01);



	message = "TOTO ";
	page[1]=0x02;
	page[2]=0x02;

	col[1]=0x00;
	col[2]=0x7F;
	sendCMD(page,(uint16_t)sizeof(page));
	sendCMD(col, (uint16_t)sizeof(col));
	sendString(message,0x00);

	page[1]=0x03;
	page[2]=0x03;
	col[1]=0x00;
	col[2]=0x7F;
	message = "   @*@    %     @M     ";
	sendCMD(page,(uint16_t)sizeof(page));
	sendCMD(col, (uint16_t)sizeof(col));
	sendString(message,0x00);
}

void sendCMD(uint8_t *cmd, uint16_t size) {
	//set dc low
	HAL_GPIO_WritePin(oled_DC_GPIO_Port,  oled_DC_Pin, GPIO_PIN_RESET);
	//set CS low

	HAL_GPIO_WritePin(oled_NSS_GPIO_Port,  oled_NSS_Pin, GPIO_PIN_RESET);
	//send cmd
	HAL_SPI_Transmit(&hspi2, cmd, size, 1000);
	//set CS high

	HAL_GPIO_WritePin(oled_NSS_GPIO_Port,  oled_NSS_Pin, GPIO_PIN_SET);

}

void sendDATA(uint8_t *data, uint16_t size) {
	//send and go through buffer
	//set dc high

	HAL_GPIO_WritePin(oled_DC_GPIO_Port, oled_DC_Pin, GPIO_PIN_SET);
	//set CS low

	HAL_GPIO_WritePin(oled_NSS_GPIO_Port,  oled_NSS_Pin, GPIO_PIN_RESET);
	//sendData
	//for(int i=0; i<dataSize;i++);
	HAL_SPI_Transmit(&hspi2, data, size, 1000);
	//set CS high
	HAL_GPIO_WritePin(oled_NSS_GPIO_Port,  oled_NSS_Pin, GPIO_PIN_SET);
	//set dc high

	//HAL_GPIO_WritePin(oled_DC_GPIO_Port, oled_DC_Pin, GPIO_PIN_RESET);
}

void clearScreen(){
	for (int i=0; i<1024; i++){
		  sendDATA(space, (uint16_t)sizeof(space));
	  }
}

void sendString(char *string, uint8_t header){

	for(int i =0; string[i]!='\0'; i++){
		uint8_t letter[6];
		uint16_t wordSize = (uint16_t)sizeof(letter);
		//IF STRING I LETTER

		for(int j =0; j<6; j++){
			wordSize = (uint16_t)sizeof(letter);
			if (string[i]>='A' && string[i] <= 'Z'){
				letter[j] = fonts[(string[i]-'A')*6+j];
			}
			else if(string[i] >= '0' && string[i] <= '9'){
				letter[j] = fonts[(string[i]-'0'+26)*6+j];
			}
			else if(string[i]=='%'){
				letter[j] = fonts[36*6+j];
			}
			else if(string[i]==':'){
				letter[j]=fonts[39*6+j];
				wordSize = 2;

			}
			else if(string[i]=='.'){
				letter[j]=fonts[39*6+2+j];
				wordSize = 2;

			}
			else if(string[i]==' '){
				letter[j] = fonts[38*6+j];
				//wordSize=2;
			}
			else if(string[i]=='@'){
				letter[j] = fonts[38*6+j];
				wordSize=2;
			}
			else if(string[i]=='*'){
				letter[j] = fonts[37*6+j];
			}
			letter[j]=letter[j]|header;
		}
		sendDATA(letter, wordSize);
	}
}

void updateScreen(char* hr, char* spo2, char* distance, char* user){

	uint8_t page[] = {0x22, 0x00,0x00};

	uint8_t col[]= {0x21, 0x00, 0x7F};

	//hr col is 0-18
	//spo2 col is - 33-51
	//distance col is for
	if(user[0]=='1'){
		page[1]=0x00;
		page[2]=0x00;
		col[1]=0x00;
		col[2]=0x12;
		sendCMD(page,(uint16_t)sizeof(page));
		sendCMD(col, (uint16_t)sizeof(col));
		sendString(hr,0x00);
		col[1]=0x21;
		col[2]=0x32;
		sendCMD(page,(uint16_t)sizeof(page));

		sendCMD(col, (uint16_t)sizeof(col));
		sendString(spo2,0x00);
		col[1]=0x61;
		col[2]=0x79;
		sendCMD(col, (uint16_t)sizeof(col));
		sendString(distance,0x00);


	}
	if(user[0]=='2'){

		page[1]=0x03;
		page[2]=0x03;
		col[1]=0x00;
		col[2]=0x12;
		sendCMD(page,(uint16_t)sizeof(page));
		sendCMD(col, (uint16_t)sizeof(col));
		sendString(hr,0x00);
		col[1]=0x21;
		col[2]=0x32;
		sendCMD(col, (uint16_t)sizeof(col));
		sendString(spo2,0x00);
		col[1]=0x41;
		col[2]=0x59;
		sendCMD(col, (uint16_t)sizeof(col));
		sendString(distance,0x00);
	}
}


void testScreen()
{

	if(isSelfSetup)
		  		  {
		  			  setupScreen();
		  			  isSelfSetup = 0;

		  		  }

		  		  if(hr[2]=='9'){
		  			  if(hr[1]=='9'){
		  				  if (hr[0]=='9'){
		  					  hr[0]='0';
		  				  }
		  				  else{
		  					  hr[0]=hr[0]+1;
		  				  }
		  				  hr[1]='0';
		  			  }
		  			  else{
		  				  hr[1]=hr[1]+1;
		  			  }
		  			  hr[2]='0';
		  		  }
		  		  else{
		  			  hr[2]=hr[2]+1;
		  		  }

		  		  distance[1]=hr[0];
		  		  distance[2]=hr[1];
		  		  distance[3]=hr[2];
		  		  user[0]='1';
		  		  updateScreen(hr, hr, hr, user);
		  		  user[0]='2';
		  		  updateScreen(hr, hr, distance, user);

}

void convertArray(uint8_t hr, uint8_t spo2)
{
	if (hr >= 100)
	{
		HR_Valid[3] = '\0';
		HR_Valid[2] = (hr % 10) + '0';
		HR_Valid[1] = ((hr/10) % 10) + '0';
		HR_Valid[0] = ((hr/100) % 10) + '0';
	}
	else
	{
		HR_Valid[3] = '\0';
		HR_Valid[2] = (hr % 10) + '0';
		HR_Valid[1] = ((hr/10) % 10) + '0';
		HR_Valid[0] = ' ';
	}

	if (spo2 >= 100)
	{
		SPO2_Valid[3] = '\0';
		SPO2_Valid[2] = (spo2 % 10) + '0';
		SPO2_Valid[1] = ((spo2/10) % 10) + '0';
		SPO2_Valid[0] = ((spo2/100) % 10) + '0';
	}
	else
	{
		SPO2_Valid[3] = '\0';
		SPO2_Valid[2] = (spo2 % 10) + '0';
		SPO2_Valid[1] = ((spo2/10) % 10) + '0';
		SPO2_Valid[0] = ' ';
	}

	testScreen();

	/*

	if(hr[2]=='9'){
		if(hr[1]=='9'){
			if (hr[0]=='9'){
				hr[0]='0';
			}
			else{
				hr[0]=hr[0]+1;
			}
			hr[1]='0';
		}
		else{
			hr[1]=hr[1]+1;
		}
		hr[2]='0';
	}
	else{
		hr[2]=hr[2]+1;
	}
	*/

	return;
}
