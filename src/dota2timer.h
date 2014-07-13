/*
 * dota2timer.h
 *
 *  Created on: 13/07/2014
 *      Author: nicolas
 */

#ifndef DOTA2TIMER_H_
#define DOTA2TIMER_H_

#define ROSHAN_ALIVE 100
#define ROSHAN_DEAD 0

/*#define ROSHAN_RESPAWN_TIME_LOWER 480
#define ROSHAN_RESPAWN_TIME_UPPER 660*/


#define ROSHAN_RESPAWN_TIME_LOWER 10
#define ROSHAN_RESPAWN_TIME_UPPER 20

// Devuelve un string con el estado de Roshan.
void get_string_for_roshan(unsigned int roshan_status, char* s);

// Helper para obtener los segundos transcurridos.
int seconds(void);

#endif /* DOTA2TIMER_H_ */
