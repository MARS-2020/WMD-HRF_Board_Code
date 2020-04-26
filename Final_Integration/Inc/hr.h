
#ifndef HR_H_
#define HR_H_

void HR_APP_MODE(void); //Put component in application mode
void HR_MFIO_SET(void); //set up MFIO as an interrupt
uint8_t HR_INIT(void); //Start Algorithm
uint8_t HR_READ(uint8_t * receive_data);
uint8_t HR_SHUTDOWN(void);
void pause_sensor(void);
void unpause_sensor(void);
uint8_t write_cmd(uint8_t arr[], uint8_t size);
void read_cmd(uint8_t arr[], uint8_t size, uint8_t * received_buff);
uint8_t fill_buff(uint8_t arr[], uint8_t size, uint8_t * receive_buff);
void start(void);
void stop(void);
void shut30101(void);
void start30101(void);

#endif /* HR_H_ */
