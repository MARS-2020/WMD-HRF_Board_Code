#include "main.h"
#include "hr.h"


extern I2C_HandleTypeDef hi2c1;
extern uint8_t HR_FLAG;

extern uint16_t heartrate;
extern uint8_t  HR_conf;
extern uint16_t SPO2;
extern uint8_t  alg_state;
extern uint8_t  alg_status;


uint8_t  read_flag = 0;

uint16_t writeAddr = 0xAA;
uint16_t readAddr = 0xAB;

uint8_t MAX_BUFF = 100;
uint8_t num_samp = -1;

uint8_t sampleCnt = 0;


void HR_APP_MODE()
{
	  //HAL_Delay(10);
	  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_RESET); //set MFIO LOW
	  HAL_Delay(10);
	  HAL_GPIO_WritePin(HR_RESET_GPIO_Port, HR_RESET_Pin, GPIO_PIN_RESET); //set RSTN low for 10ms
	  HAL_Delay(3);
	  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_SET); //set MFIO high while RSTN is low
	  HAL_Delay(5);
	  HAL_GPIO_WritePin(GPIOB, HR_RESET_Pin, GPIO_PIN_SET); //return RSTN to its high state
	  HAL_Delay(1000);	//wait 1 second (in app mode after 50ms) - will have to replace with timer later
}




void HR_MFIO_SET()
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  //HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_SET);

	  GPIO_InitStruct.Pin = HR_MFIO_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  //GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
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


	//1.2 - set output mode to sensor
	uint8_t arr_1_2[3] = {0x10, 0x00, 0x02};
	if(write_cmd(arr_1_2, sizeof(arr_1_2)) != 0x00)
	{
		return 1;
	}


	//1.3 - Set sensor hub interrupt threshold
	uint8_t arr_1_3[3] = {0x10, 0x01, 0x05};
	if(write_cmd(arr_1_3, sizeof(arr_1_3)) != 0x00)
	{
		return 1;
	}

	//1.4 - Enable AGC
	uint8_t arr_1_4[3] = {0x52, 0x00, 0x01};
	if(write_cmd(arr_1_4, sizeof(arr_1_4)) != 0x00)
	{
		return 1;
	}
	HAL_Delay(20);

	//1.6 - Enable AFE
	uint8_t arr_1_6[3] = {0x44, 0x03, 0x01};
	if(write_cmd(arr_1_6, sizeof(arr_1_6)) != 0x00)
	{
		return 1;
	}
	HAL_Delay(40);


	//1.7 - Enable HR/SpO2 Algorithm
	uint8_t arr_1_7[3] = {0x52, 0x02, 0x01};
	if(write_cmd(arr_1_7, sizeof(arr_1_7)) != 0x00)
	{
		return 1;
	}
	HAL_Delay(40);

/*
	//1.9 - READ ALGO SAMPLE AVERAGE RATE
	uint8_t arr_1_9[3] = {0x51, 0x00, 0x03};
	uint8_t received[2] = {-3, -3};
	read_cmd(arr_1_9, sizeof(arr_1_9), received);
	if(received[0] != 0x00)
	{
		return 1;
	}
*/


/*
	//1.10 - CHANGE ALGORITHM RATE
	uint8_t arr_1_10[4] = {0x50, 0x00, 0x03, 0x0A};
	uint8_t received2[2] = {-3, -3};
	read_cmd(arr_1_10, sizeof(arr_1_10), received2);
	if(received2[0] != 0x00)
	{
		return 1;
	}


	//1.9 - READ ALGO SAMPLE AVERAGE RATE
	uint8_t arr_1_x[3] = {0x51, 0x00, 0x03};
	uint8_t received3[2] = {-3, -3};
	read_cmd(arr_1_x, sizeof(arr_1_x), received3);
	if(received3[0] != 0x00)
	{
		return 1;
	}

	uint8_t samp = received3[1];

*/

	return 0;
}




uint8_t HR_READ(uint8_t * data)
{

	//2.1 - Data finished when bit3 of AA0000 is full (DATARDYINT)
	uint8_t arr_2_1[2] = {0x00, 0x00};
	uint8_t received[2] = {-3, -3};
	read_cmd(arr_2_1, sizeof(arr_2_1), received);
	if(received[0] != 0x00)
	{
		return 1;
	}


	if(received[1] == 0x08)
	{
		//2.2 - get number of samples in FIFO
		uint8_t arr_2_2[2] = {0x12, 0x00};
		uint8_t received[2] = {-3, -3};
		read_cmd(arr_2_2, sizeof(arr_2_2), received);
		if(received[0] != 0x00)
		{
			return 1;
		}

		num_samp = received[1];

		//2.3 - read all samples from FIFO
		uint8_t arr_2_3[2] = {0x12, 0x01};
		if(!fill_buff(arr_2_3, sizeof(arr_2_3), data))
		{
			return 1;
		}

		/*
		uint8_t arr_chk[2] = {0x12, 0x00};
		uint8_t received4[2] = {-3, -3};
		read_cmd(arr_chk, sizeof(arr_chk), received4);
		if(received4[0] != 0x00)
		{
			return 1;
		}
		*/

		heartrate  = ((((uint16_t) data[1]) << 8) | (data[2])) / 10;
		HR_conf    =  data[3];
		SPO2  = ((((uint16_t) data[4]) << 8) | (data[5])) / 10;
		alg_state  =  data[6];

		//read_flag = 0;
		if(((heartrate > 50) && (heartrate < 255)) && ((SPO2 > 50) && (SPO2 < 101)))
		{
			//stop();

			//read_flag = 3;
			//sampleCnt++;
			//if(sampleCnt > 10)
			//{
				//sampleCnt = 0;
				HR_FLAG = 1;
				shut30101();
			//}
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



void stop()
{


	uint8_t arr_stop2[3] = {0x52, 0x02, 0x00};
	if(write_cmd(arr_stop2, sizeof(arr_stop2)) != 0x00)
	{
		return;
	}
	//HAL_Delay(40);

	uint8_t arr_stop1[3] = {0x44, 0x03, 0x00};
	if(write_cmd(arr_stop1, sizeof(arr_stop1)) != 0x00)
	{
		return;
	}
	//HAL_Delay(40);

	return;
}

void start()
{
	uint8_t arr_start1[3] = {0x44, 0x03, 0x01};
	if(write_cmd(arr_start1, sizeof(arr_start1)) != 0x00)
	{
		return;
	}
	//HAL_Delay(40);

	uint8_t arr_start2[3] = {0x52, 0x02, 0x01};
	if(write_cmd(arr_start2, sizeof(arr_start2)) != 0x00)
	{
		return;
	}
	//HAL_Delay(40);

	return;
}



void shut30101()
{
	uint8_t arr_shut[3] = {0x41, 0x03, 0x09};
	uint8_t received[2] = {-3, -3};
	read_cmd(arr_shut, sizeof(arr_shut), received);
	if(received[0] != 0x00)
	{
		return;
	}

	received[1] = received[1]  | (1<<7);

	uint8_t arr_shut2[4] = {0x40, 0x03, 0x09, received[1]};
	if(write_cmd(arr_shut2, sizeof(arr_shut2)) != 0x00)
	{
		return;
	}
}



void start30101(void)
{
	uint8_t arr_shut[3] = {0x41, 0x03, 0x09};
	uint8_t received[2] = {-3, -3};
	read_cmd(arr_shut, sizeof(arr_shut), received);
	if(received[0] != 0x00)
	{
		return;
	}

	received[1] = (received[1]  & (~(1<<7)));

	uint8_t arr_shut2[4] = {0x40, 0x03, 0x09, received[1]};
	if(write_cmd(arr_shut2, sizeof(arr_shut2)) != 0x00)
	{
		return;
	}


	//ENABLE ALGORITHM
	uint8_t arr_1_7[3] = {0x52, 0x02, 0x01};
	if(write_cmd(arr_1_7, sizeof(arr_1_7)) != 0x00)
	{
		return;
	}
}




void pause_sensor()
{

		uint8_t receive_buff = -1;

		  heartrate = 0;
		  SPO2 = 0;

		//1.2 - set output mode to sensor
		uint8_t arr_pause[3] = {0x10, 0x00, 0x04};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_pause, sizeof(arr_pause), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		receive_buff = -3;

		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		while(receive_buff == 0xFE)
		{
			HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
			while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		}
		if(receive_buff != 0x00)
		{
			return;
		}
}


void unpause_sensor()
{

		uint8_t receive_buff = -1;

		//1.2 - set output mode to sensor
		uint8_t arr_unpause[3] = {0x10, 0x00, 0x03};
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr_unpause, sizeof(arr_unpause), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		receive_buff = -3;

		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		while(receive_buff == 0xFE)
		{
			HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
			while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		}
		if(receive_buff != 0x00)
		{
			return;
		}
}



uint8_t write_cmd(uint8_t arr[], uint8_t size)
{
	uint8_t receive_buff = -3;

	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr, size, 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff, sizeof(receive_buff), 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, &receive_buff, sizeof(receive_buff), 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}

	return receive_buff;
}


void read_cmd(uint8_t arr[], uint8_t size, uint8_t * receive_buff)
{
	HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr, size, 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_buff, 2, 1000);
	while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	while(receive_buff[0] == 0xFE)
	{
		HAL_I2C_Master_Receive(&hi2c1, 0xAB, receive_buff, 2, 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
}


uint8_t fill_buff(uint8_t arr[], uint8_t size, uint8_t * receive_buff)
{

	//for(int i = 0; i < num_samp; i++)
	//{
		HAL_I2C_Master_Transmit(&hi2c1, writeAddr, arr, size, 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);

		HAL_I2C_Master_Receive(&hi2c1, readAddr, receive_buff, 31, 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
		if(receive_buff[0] != 0)
		{
			return 0;
		}

		return 1;
	//}


	/*
	for(int i = 0; i < MAX_BUFF; i++)
	{
		HAL_I2C_Master_Receive(&hi2c1, readAddr, &receive_buff[i], 1, 1000);
		while(((&hi2c1) -> State) != HAL_I2C_STATE_READY);
	}
	*/
}
