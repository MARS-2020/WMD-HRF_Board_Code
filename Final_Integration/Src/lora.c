/*
 * lora.c
 *
 *  Created on: Apr 14, 2020
 *      Author: Lucas Pfeiffer
 */

#include "lora.h"
#include "main.h"

uint8_t send[] = "Jacob";
uint8_t send1[] = "Austin";
uint8_t send2[] = "Morgan";
uint8_t send3[] = "Lucas";
uint8_t headerTo = 255;
uint8_t headerFrom = 255;
uint8_t headerID = 0;
uint8_t headerFlags = 0;

uint8_t receive[80]; //receive data buffer

extern SPI_HandleTypeDef hspi1;



//Function for writing to a register
void writeReg(uint8_t addr, uint8_t value)
{
	uint8_t reg = addr | 0x80;
	uint8_t val = value;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET); //pull NSS low to start frame
	HAL_SPI_Transmit(&hspi1, &reg, (uint16_t)sizeof(reg), 1000);
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_SPI_Transmit(&hspi1, &val, (uint16_t)sizeof(val), 1000);
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET); //pull NSS high to end frame
}

//Function for reading from a register
uint8_t readReg(uint8_t addr)
{
	uint8_t reg = addr & ~0x80;
	uint8_t data = 0;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET); //pull NSS low to start frame
	//HAL_SPI_TransmitReceive(&hspi1, &reg, &data, 1, 1000);
	HAL_SPI_Transmit(&hspi1, &reg, sizeof(reg), 1000); //send a read command from that address
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_SPI_Receive(&hspi1, &data, sizeof(data), 1000);
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET); //pull NSS high to end frame
	return data;
}

//Function for reading from FIFO
void readFIFO(uint8_t buff[], uint16_t size)
{
	uint8_t reg = RH_RF95_REG_00_FIFO & ~0x80;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET); //pull NSS low to start frame
	HAL_SPI_Transmit(&hspi1, &reg, sizeof(reg), 1000); //send a read command from that address
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_SPI_Receive(&hspi1, buff, size, 1000);
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET); //pull NSS high to end frame
}

//Function for reading from a register
void receiveData()
{
	writeReg(RH_RF95_REG_01_OP_MODE, 0x01);
	writeReg(RH_RF95_REG_12_IRQ_FLAGS, 0xFF);

	if (readReg(RH_RF95_REG_12_IRQ_FLAGS) == 0x00)
	{
		writeReg(RH_RF95_REG_0D_FIFO_ADDR_PTR, readReg(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR)); //fifo addr ptr = fifo rx current addr
		uint8_t bytesLimit = readReg(RH_RF95_REG_13_RX_NB_BYTES);
		//HAL_Delay(10);
		readFIFO(receive, (uint16_t) bytesLimit);
		writeReg(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0x00);
	}

	writeReg(RH_RF95_REG_01_OP_MODE, 0x05);
	writeReg(RH_RF95_REG_40_DIO_MAPPING1, 0x00);
}

//Function to burst write (primarily for FIFO)
void writeReg_Burst(uint8_t addr, uint8_t data[], uint8_t length)
{
	uint8_t reg = addr | 0x80;
	uint8_t val = 0;
	if (length >= 1)
	{
		HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET); //pull NSS low to start frame
		HAL_SPI_Transmit(&hspi1, &reg, (uint16_t)sizeof(reg), 1000);
		while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
		for(int i = 0; i <= (length - 1); i++)
		{
			val = data[i];
			HAL_SPI_Transmit(&hspi1, &val, (uint16_t)sizeof(val), 1000);
			while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
		}

		HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET); //pull NSS high to end frame
	}
}

void sendPacket(uint8_t data[], uint8_t size)
{
	writeReg(RH_RF95_REG_01_OP_MODE, 0x01); //STDBY
	writeReg(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0x00); //fifo addr pointer

	//set headers
	writeReg(RH_RF95_REG_00_FIFO, headerTo); //header TO
	writeReg(RH_RF95_REG_00_FIFO, headerFrom); //header FROM
	writeReg(RH_RF95_REG_00_FIFO, headerID); //header ID
	writeReg(RH_RF95_REG_00_FIFO, headerFlags); //header FLAGS

	//uint8_t size = (sizeof(&data)/sizeof(data[0]));
	//uint8_t size = sizeof(*send);

	//write message data to fifo
	writeReg_Burst(RH_RF95_REG_00_FIFO, data, size);

	//set payload length
	writeReg(RH_RF95_REG_22_PAYLOAD_LENGTH, size + RH_RF95_HEADER_LEN);

	//HAL_Delay(10); //delay some time

	writeReg(RH_RF95_REG_12_IRQ_FLAGS, 0xFF); //clear txdone
	writeReg(RH_RF95_REG_01_OP_MODE, 0x03); //TX Mode
	writeReg(RH_RF95_REG_40_DIO_MAPPING1, 0x40); //DIO0

	//HAL_Delay(10);
	while(readReg(RH_RF95_REG_12_IRQ_FLAGS) != 0x08);

	writeReg(RH_RF95_REG_01_OP_MODE, 0x01); //STDBY
	writeReg(RH_RF95_REG_12_IRQ_FLAGS, 0xFF); //clear txdone
	//HAL_Delay(10);
}

void LORA_INIT(void)
{
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//initialization
	writeReg(RH_RF95_REG_01_OP_MODE, 0x80); //long range mode
	//readReg(RH_RF95_REG_01_OP_MODE);
	writeReg(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0x00); //tx base addr to 0
	writeReg(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0x00); //rx base addr to 0
	writeReg(RH_RF95_REG_1D_MODEM_CONFIG1, 0x72); //coding rate and modem config
	writeReg(RH_RF95_REG_1E_MODEM_CONFIG2, 0x74); //rxpayloadcrc and spreading factor
	writeReg(RH_RF95_REG_26_MODEM_CONFIG3, 0x04); //LNA gain
	writeReg(RH_RF95_REG_20_PREAMBLE_MSB, 0x00); //preamble MSB
	writeReg(RH_RF95_REG_21_PREAMBLE_LSB, 0x08); //premamble LSB
	writeReg(RH_RF95_REG_06_FRF_MSB, 0x6C); //freq msb
	writeReg(RH_RF95_REG_07_FRF_MID, 0x80); //freq mid
	writeReg(RH_RF95_REG_08_FRF_LSB, 0x00); //freq lsb
	writeReg(RH_RF95_REG_4D_PA_DAC, 0x04); //padac
	writeReg(RH_RF95_REG_09_PA_CONFIG, 0x88); //output power and PA_BOOST

	//set frequency to 915MHz
	writeReg(RH_RF95_REG_06_FRF_MSB, 0xE4); //freq msb
	writeReg(RH_RF95_REG_07_FRF_MID, 0xC0); //freq mid
	writeReg(RH_RF95_REG_08_FRF_LSB, 0x00); //freq lsb

	//set power
	writeReg(RH_RF95_REG_4D_PA_DAC, 0x07); //padac
	writeReg(RH_RF95_REG_09_PA_CONFIG, 0x8F); //output power and PA_BOOST

	//set up for rx
	writeReg(RH_RF95_REG_01_OP_MODE, 0x05);
	writeReg(RH_RF95_REG_40_DIO_MAPPING1, 0x00);
}

uint8_t valid(uint8_t interrupts)
{
	if(interrupts != 0x50) //0x50 means only RXDone and ValidHeader are set
	{
		return 0;
	}
	return 1;
}
