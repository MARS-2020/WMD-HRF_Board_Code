#include "main.h"
#include "hr.h"


extern I2C_HandleTypeDef hi2c1;

extern uint16_t heartrate;
extern uint8_t  HR_conf;
extern uint16_t SPO2;
extern uint8_t  alg_state;
extern uint8_t  alg_status;



void HR_APP_MODE()
{
	  //HAL_Delay(10);
	  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_RESET); //set MFIO LOW
	  HAL_Delay(10);
	  HAL_GPIO_WritePin(HR_RESET_GPIO_Port, HR_RESET_Pin, GPIO_PIN_RESET); //set RSTN low for 10ms
	  HAL_Delay(3);
	  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_SET); //set MFIO high while RSTN is low
	  //GPIOB->BSRR = HR_MFIO_Pin;
	  HAL_Delay(5);
	  HAL_GPIO_WritePin(GPIOB, HR_RESET_Pin, GPIO_PIN_SET); //return RSTN to its high state
	  HAL_Delay(1000);	//wait 1 second (in app mode after 50ms) - will have to replace with timer later

	  //GPIOB->MODER &= ~(GPIO_MODER_MODE4); //Set MFIO to be input so it can act as an interrupt
}




void HR_MFIO_SET()
{
	/*
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_SET);


	  GPIO_InitStruct.Pin = HR_MFIO_Pin;
	  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(HR_MFIO_GPIO_Port, &GPIO_InitStruct);
	*/

	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = HR_MFIO_Pin;
	  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(HR_MFIO_GPIO_Port, &GPIO_InitStruct);

	  //enable the interrupt
	  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}





uint8_t HR_INIT()
{
	//1.1 - configure default spo2 alg values
	//uint8_t arr_1_1[15] = {0x50, 0x02, 0x0B, 0x00, 0x02, 0x6F, 0x60, 0xFF, 0xCB, 0x1D, 0x12, 0x00, 0xAB, 0xF3, 0x7B};
	uint16_t writeAddr = 0xAA;
	uint16_t readAddr = 0xAB;
	//HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_1_1, sizeof(arr_1_1), 1000);
	uint8_t receive_buff = -1;
	//HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
	//if(receive_buff != 0x00)
	//{
	//	return 1;
	//}


	//1.2 - set output mode to sensor
	uint8_t arr_1_2[3] = {0x10, 0x00, 0x03};
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_1_2, sizeof(arr_1_2), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	receive_buff = -3;
	//HAL_Delay(20);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_buff != 0x00)
	{
		return 1;
	}

	//1.3 - Set sensor hub interrupt threshold
	uint8_t arr_1_3[3] = {0x10, 0x01, 0x02};
	HAL_I2C_Master_Transmit(&hi2c1, 0xAA, arr_1_3, sizeof(arr_1_3), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	receive_buff = -1;
	//HAL_Delay(20);
	HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_buff != 0x00)
	{
		return 1;
	}

	//1.4 - Enable AGC
	uint8_t arr_1_4[3] = {0x52, 0x00, 0x01};
	HAL_I2C_Master_Transmit(&hi2c1, 0xAA, arr_1_4, sizeof(arr_1_4), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	receive_buff = -1;
	//HAL_Delay(20);

	HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}

	if(receive_buff != 0x00)
	{
		return 1;
	}

	//1.6 - Enable AFE
	uint8_t arr_1_6[3] = {0x44, 0x03, 0x01};
	HAL_I2C_Master_Transmit(&hi2c1, 0xAA, arr_1_6, sizeof(arr_1_6), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	receive_buff = -1;
	//HAL_Delay(100);
	HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_buff != 0x00)
	{
		return 1;
	}

	//1.7 - Enable HR/SpO2 Algorithm
	uint8_t arr_1_7[3] = {0x52, 0x02, 0x01};
	HAL_I2C_Master_Transmit(&hi2c1, 0xAA, arr_1_7, sizeof(arr_1_7), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	receive_buff = -1;
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_buff != 0x00)
	{
		return 1;
	}


	return 0;
}






uint8_t HR_READ(uint8_t * receive_data)
{

	//2.1 - Data finished when bit3 of AA0000 is full (DATARDYINT)
	uint8_t arr_2_1[2] = {0x00, 0x00};
	uint16_t writeAddr = 0xAA;
	uint16_t readAddr = 0xAB;
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_2_1, sizeof(arr_2_1), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	uint8_t receive_hub[2];
	HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_hub, sizeof(receive_hub), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	if(receive_hub[0] != 0x00)   //failed read
	{
		return 1;
	}

	/*
	while(receive_hub[1] != 0x08)
	{
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_2_1, sizeof(arr_2_1), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_hub, sizeof(receive_hub), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
*/

	if(receive_hub[1] == 0x08)
	{
		//2.2 - get number of samples in FIFO
		uint8_t arr_2_2[2] = {0x12, 0x00};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_2_2, sizeof(arr_2_2), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_hub, sizeof(receive_hub), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		if(receive_hub[0] != 0x00)	//failed read
		{
			return 1;
		}

		//2.3 - read all samples from FIFO
		uint8_t arr_2_3[2] = {0x12, 0x01};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_2_3, sizeof(arr_2_3), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_data, 22, 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);

		/*
		if(receive_data[18] == 0x03)
		{
			uint16_t heartrate  = ((((uint16_t) receive_data[13]) << 8) | (receive_data[14])) / 10;
			uint8_t  HR_conf    =  receive_data[15];
			uint16_t spo2  = ((((uint16_t) receive_data[16]) << 8) | (receive_data[17])) / 10;
			uint8_t  alg_state  =  receive_data[18];
			uint8_t  alg_status =  receive_data[19];

			return 0;
		}
*/
		heartrate = 0;
		SPO2 = 0;
		alg_state = 0;

		heartrate  = ((((uint16_t) receive_data[13]) << 8) | (receive_data[14])) / 10;
		HR_conf    =  receive_data[15];
		SPO2  = ((((uint16_t) receive_data[16]) << 8) | (receive_data[17])) / 10;
		alg_state  =  receive_data[18];
		alg_status =  receive_data[19];

		if((heartrate > 50) && (SPO2 > 50))
		{
			asm("nop");
		}

		return 1;
	}

	return 1;
}







uint8_t HR_SHUTDOWN()
{
	/*
	//3.1 - Disable the AFE
	uint8_t arr_3_1[3] = {0x44, 0x03, 0x00};
	uint16_t writeAddr = 0xAA;
	uint16_t readAddr = 0xAB;
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_3_1, sizeof(arr_3_1), 1000);
	uint8_t receive_off = -1;
	HAL_Delay(100);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_off, sizeof(receive_off), 1000);
	while(receive_off == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_off, sizeof(receive_off), 1000);
	}
	if(receive_off != 0x00)   //failed read
	{
		return 1;
	}

	//3.3 - Disable the Algorithm
	uint8_t arr_3_3[3] = {0x52, 0x02, 0x00};
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_3_3, sizeof(arr_3_3), 1000);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_off, sizeof(receive_off), 1000);
	while(receive_off == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_off, sizeof(receive_off), 1000);
	}
	if(receive_off != 0x00)   //failed read
	{
		return 1;
	}
*/

	//SOFT RESET SENSOR

	uint16_t writeAddr = 0xAA;
	uint16_t readAddr = 0xAB;
	uint8_t receive_off = -1;
	uint8_t arr_x_x[4] = {0x40, 0x03, 0x09, 0x40};
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_x_x, sizeof(arr_x_x), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	//HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_off, sizeof(receive_off), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_off == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_off, sizeof(receive_off), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_off != 0x00)   //failed read
	{
		return 1;
	}

	//soft reset agc

	uint8_t arr_x_y[3] = {0x01, 0x00, 0x02};
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_x_y, sizeof(arr_x_y), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	//HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_off, sizeof(receive_off), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_off == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_off, sizeof(receive_off), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	if(receive_off != 0x00)   //failed read
	{
		return 1;
	}



	return 0;
}


void pauseSensor()
{
	//1.1 - configure default spo2 alg values
		uint16_t writeAddr = 0xAA;
		uint16_t readAddr = 0xAB;
		uint8_t receive_buff = -1;

		//1.2 - set output mode to sensor
		//uint8_t arr_1_2[3] = {0x10, 0x00, 0x04};
		uint8_t arr_1_2[4] = {0x40, 0x03, 0x09, 0x80};
		//uint8_t arr_1_2[3] = {0x44, 0x03, 0x00};
		//uint8_t arr_1_2[4] = {0x40, 0x03, 0x09, 0x40};
		//uint8_t arr_1_2[3] = {0x01, 0x00, 0x02};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_1_2, sizeof(arr_1_2), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		receive_buff = -3;
		//HAL_Delay(20);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		while(receive_buff == 0xFE)
		{
			HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
			while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		}
		//HAL_Delay(40);
}

void unpauseSensor()
{
	//1.1 - configure default spo2 alg values
		uint16_t writeAddr = 0xAA;
		uint16_t readAddr = 0xAB;
		uint8_t receive_buff = -1;

		//1.2 - set output mode to sensor
		//uint8_t arr_1_2[3] = {0x10, 0x00, 0x03};
		//uint8_t arr_1_2[3] = {0x44, 0x03, 0x01};
		//uint8_t arr_1_2[3] = {0x10, 0x00, 0x03};
		uint8_t arr_1_2[3] = {0x01, 0x00, 0x02};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_1_2, sizeof(arr_1_2), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		receive_buff = -3;
		//HAL_Delay(20);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		while(receive_buff == 0xFE)
		{
			HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
			while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		}

		uint8_t arr_1_3[3] = {0x44, 0x03, 0x01};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_1_3, sizeof(arr_1_3), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		receive_buff = -3;
		//HAL_Delay(20);
		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		while(receive_buff == 0xFE)
		{
			HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
			while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		}
		//HAL_Delay(40);
}
