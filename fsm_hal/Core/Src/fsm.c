/*
 * fsm.c
 *
 *  Created on: 14/10/2020
 *      Author: alram
 */
#include "main.h"
#include "fsm.h"
//#include "stm32l4xx_hal.h"
uint32_t tsw=20000;			//Tiempo limite para decidir los cambios de estado del led
uint32_t t_led=0;			//Tiempo de la ultima transición de estados led
uint32_t start=1;			//Bandera que permite indicar el estado en el que comienzan los controladores

uint32_t t_bot=0;			//Tiempo de la ultima detección del botón
uint32_t event=1;			//Bandera que indica que se presiono y libero el botón
uint32_t cont=0;			//# de veces que se ha presionado en botón
uint32_t dim=0;				//Bandera que indica si se dimeriza o no el led
uint32_t p;					//Almacena el estado del botón
static uint32_t con=0;		//Almacena el tiempo, aumenta cada milisegundo
enum states {OFF,DIM,BRIGHT} led_state;	//Estados del controlador led
enum events{tswm,tswp,NC} new_event;		//Eventos del controlador del led

void LedHandler(void){

		if (start==1){			//Cuando la funcion se ejecuta por primera vez:
			led_state=OFF;	//Se pone el estado en OFF
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);	//Se apaga el led
			start=0;			//Y se cambia la bandera para no volver a ejecutar esto
		}

		#define MAX_STATES 3
		#define MAX_EVENTS 3
		typedef void (*transition)();
		void apag(){	//Accion de apagar el led
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
			tsw=20000;	//Establece el tiempo limite en 20s
			led_state=OFF;
			t_led=HAL_GetTick();	//Guarda el tiempo actual
			dim=0;
		}
		void dimer(){	//Accion de dimerizar el led
			dim=1;
			led_state=DIM;
			t_led=HAL_GetTick();	//Guarda el tiempo actual
			tsw=4000;
		}
		void brill(){//Accion de encender el led
			dim=0;
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);//Se enciende el led
			led_state=BRIGHT;		//Se pasa al estado BRIGHT
			t_led=HAL_GetTick();
			tsw=4000;
		}
		void error(){dimerizar(dim);}//Si no ha cambiado nada, revise si debe dimerizar el led

		transition state_table[MAX_STATES][MAX_EVENTS] = {
				{dimer,brill,error}, //OFF
				{brill, apag, error},//DIM
				{apag, dimer, error}//BRIGHT
		};
				//tswm, tswp, NC
		if (event==0){	//Si se presiono el boton
			event=1;	//Limpie la bandera del boton
			con=HAL_GetTick();	//Lea el tiempo actual
			if(con<(t_led+tsw)){	//Si ha transcurrido menos de un tiempo limite entre
				  new_event=tswm;	//cada cambio de estado, presente el evento
			  }
			else{
				 new_event=tswp;
			 }
		}
		else{new_event=NC;}		//Nada cambio
		if ((new_event >= 0) && (new_event < MAX_EVENTS)	//El evento actual esta entre los eventos que se crearon
				&& (led_state >= 0) && (led_state < MAX_STATES)) {//El estado actual esta entre los eventos que se crearon
		/* call the transition function */
			state_table[led_state][new_event]();//Se ejecuta la tabla de estados usando el evento y estado actual
		}
		else {
		/* invalid event/state - handle appropriately */
	}
}

enum states2 {RELEASED,WAIT,DETECT} boton_state;	//Estados controlador boton
uint32_t volatile antire=0;			//Contador que indica cuando se cumplio el antirrebote
void BotonHandler(void){
	if (start==1){			//Cuando la funcion se ejecuta por primera vez:
		boton_state=WAIT;	//Se pone el estado en OFF
	}

	p=HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13);	//Se lee el estado del boton


	switch(boton_state){	//Cambio de estados boton
		case DETECT:		//Se detecto un touch
			switch (p){		//Si el boton esta presionado
			case(0):
				antire+=1;	//Aumente el contador en 1
				break;
			case (1):		//Si el boton no esta presionado
				if (antire>50){//Si el boton paso mas de 50 ms presionado (no es un rebote)
					boton_state=RELEASED;	//pase al estado liberado
				}else{boton_state=WAIT;}	//Si no pase al estado de espera
				break;
			}
			break;
		case RELEASED://El boton fue presionado y liberado
			event=0;	//Se envia el evento al controlador del led
			antire=0;	//Se reinicia la variable del antirrebote
			boton_state=WAIT;//Se pasa al estado de espera
			break;
		case WAIT:		//Espera a detectar un touch
			switch (p){
			case(0)://Si el boton es presionado pase al estado detect
				boton_state=DETECT;
				break;
			case (1)://Si no esta presionado no haga nada
				break;
			}
			break;
		default:

			break;

	}
}
void dimerizar(int dim){
	if(dim==1){//Si esta en estado dimerizado
		 HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);//Encienda y apague el led
	}

}


