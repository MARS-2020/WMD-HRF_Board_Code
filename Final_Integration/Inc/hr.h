
#ifndef HR_H_
#define HR_H_

#include "main.h"

void HR_APP_MODE(void); //Put component in application mode
void HR_MFIO_SET(void); //set up MFIO as an interrupt
uint8_t HR_INIT(void); //Start Algorithm
uint8_t HR_READ(uint8_t * receive_data);
uint8_t HR_SHUTDOWN(void);
void pauseSensor(void);
void unpauseSensor(void);

#endif /* HR_H_ */
