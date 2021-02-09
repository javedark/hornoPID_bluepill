/* Master_horno_pidof. El programa es para un microcontrolador. Realiza el control
 * de un horno bajo dos modos a elección del usuario: Control ON-OFF ó Control PID.
 * En ambos modos se ajustan parámetros de funcionamiento.
 *
 * Ing. MasterJER. 02]/12/2020
 *
 */

#include "FreeRTOS.h"					//FreeRTOS.
#include "task.h"							//Uso de tareas
#include "ugui.h"							//Uso de ugui.Master_horno
#include "miniprintf.h"				//Para la impresión de texto."""
#include "parauso_ugui.h"			//Config. de ugui."""
#include "oled_fun.h"					//Para manejar la pantalla oled."""
#include "spioled_config.h"		//Config. spi para la pantalla oled."""
#include "analog_config.h"		//Config. del canal analógico."""
#include "mis_img.h"					//Mis imagenes para cargar."""
#include "controles.h"				//Algoritmos de control. Master_Oled_Menu

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

//--> Definiciones.

#define SELECT GPIOA,GPIO4		//Master_Oled_Menu
#define LED 	 GPIOC,GPIO13		//Master_Oled_Menu
#define SET 	 GPIOA,GPIO5		//Master_Oled_Menu

//--> Variables globales.

static int8_t lim_inf; // Límite inferior del modo ON-OFF.Master_Oled_Menu
static int8_t lim_sup; // Límite superior del modo ON-OFF.Master_Oled_Menu

//--> Prototipos de funciones.

//Para el mapeo del valor analógico.Master_horno
long mapeo(long x, long in_min, long in_max, long out_min, long out_max);
//Para la lectura del canal analógico.Master_horno
static uint16_t read_adc(uint8_t channel);
//Para determinar si el botón fue presionado.Master_Oled_Menu
bool	press(uint32_t PUERTO,uint16_t PIN);
//Modo de control on-off.Master_Oled_Menu.
void modo_control_onoff(void);
//Selección de modo de control.
void sel_opc(void);
//Para la lectura de temperatura.
int temp(void);
//Muestra la temperatura en oled.
void imp_temp(void);

//--> Definición de funciones.

//Función de mapeo para el canal analógico.
long
mapeo(long x,long in_min, long in_max, long out_min, long out_max){
return (x - in_min)*(out_max - out_min)/(in_max - in_min) + out_min;
}
//F. Lectura del canal analógico.
static uint16_t
read_adc(uint8_t channel) {
	adc_set_sample_time(ADC1,channel,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_regular_sequence(ADC1,1,&channel);
	adc_start_conversion_direct(ADC1);
	while ( !adc_eoc(ADC1) )
		taskYIELD();
	return adc_read_regular(ADC1);
}
//F. para la obtención de la temperatura.
int temp(void){
	float volts;	//Para almacenar el voltaje obtenido en canal 0.
	int		temperatura;	//Valor que regresa la función, es la temperatura.

	volts = read_adc(0)*330/4095;
	temperatura = 20.211 + (87.813 * volts)/100;

	return	temperatura;
}
//F. Para mostrar la temperatura en oled.
void imp_temp(void){
	char buf[16];

	mini_snprintf(buf,sizeof buf,"%d",temp());
	UG_PutString(5,19,buf);
	mapa_to_oled();
}
//F. Para saber si un botón fue presionado.
bool	press(uint32_t PUERTO,uint16_t PIN){
	if(gpio_get(PUERTO,PIN) != 0x0000) vTaskDelay(pdMS_TO_TICKS(180));
	if(gpio_get(PUERTO,PIN) != 0x0000) {
		while(gpio_get(PUERTO,PIN) != 0x0000);
		return pdTRUE;
	} else {
		return pdFALSE;
	}
}
//F. Para desplegar información del modo de control.
/*void modo_control_onoff(void){
	// Límite inferior.
	limpia_seccion();
  char buf[16];
  UG_FontSelect(&FONT_5X8);
  UG_PutString(3,6,"Establece el");
  UG_PutString(3,16,"Limite INF:");
  UG_PutString(3,47,"grados cent.");
  mapa_to_oled();
  for(int i = 0; i <= 30; i = i+2){
    mini_snprintf(buf,sizeof buf, "%d",i);
    UG_FontSelect(&FONT_8X12);
    UG_PutString(30,30,buf);
    mapa_to_oled();
    while(press(SELECT) == pdFALSE){
      if(press(SET)){lim_inf = i; i = 30; break;}
    }
  }
	// Límite superior.
	limpia_seccion();
	UG_FontSelect(&FONT_5X8);
	UG_PutString(3,6,"Establece el");
	UG_PutString(3,16,"Limite SUP:");
	UG_PutString(3,47,"grados cent.");
	mapa_to_oled();
	for(int i = lim_inf + 2; i <= 30; i = i +2){
		mini_snprintf(buf,sizeof buf, "%d",i);
		UG_FontSelect(&FONT_8X12);
		UG_PutString(30,30,buf);
		mapa_to_oled();
		while(press(SELECT) == pdFALSE){
			if(press(SET)){lim_sup = i; i = 30; break;}
		}
	}

	// Pantalla muestra modo de control ON-OFF.
	limpia_seccion();
	UG_FontSelect(&FONT_8X8);
	UG_PutString(3,6,"*ON-OFF");
	UG_PutString(3,22,"L.SUP:");
	UG_PutString(3,44,"L.INF:");
	mapa_to_oled();
	mini_snprintf(buf,sizeof buf,"%d",lim_sup);
	UG_PutString(55,22,buf);
	mapa_to_oled();
	mini_snprintf(buf,sizeof buf,"%d",lim_inf);
	UG_PutString(55,44,buf);
	mapa_to_oled();
}*/
//F. Para la selección del modo de control.
/*void sel_opc(void){
uint8_t	sel_opc = 0, ejec_opc = 0;
mi_marca();
frame();
opciones();

for (;;) {
	// Selección de opciones
	while (ejec_opc == 0) {
		switch (sel_opc) {
			case 0:
			UG_FillCircle(15,48,6,pen_to_ug(0));
			UG_FillCircle(15,28,6,pen_to_ug(1));
			UG_DrawCircle(15,48,6,pen_to_ug(1));
			mapa_to_oled();
			while(1){
				if(press(SELECT)){sel_opc = 1; break;}
				if(press(SET)){ejec_opc = 1; break;}
			} break;

			case 1:
			UG_FillCircle(15,28,6,pen_to_ug(0));
			UG_DrawCircle(15,28,6,pen_to_ug(1));
			UG_FillCircle(15,48,6,pen_to_ug(1));
			mapa_to_oled();
			while(1){
				if(press(SELECT)){sel_opc = 0; break;}
				if(press(SET)){ejec_opc = 2; break;}
			} break;
		}
	}

	// Ejecución de opciones.
	switch (ejec_opc) {
		case 1:
		modo_control_onoff();
		gpio_clear(LED);
		while(1);
		break;

		case 2:
		gpio_clear(LED);
		while(1);
		break;
	}
}// Cierre del for
}*/


//--> Tarea 1
static void
tarea1(void *args) {
	(void)args;

	imp_temp();
	vTaskDelay(pdMS_TO_TICKS(500));
}

int
main(void) {
	rcc_clock_setup_in_hse_8mhz_out_72mhz();	//for blue pill

	rcc_periph_clock_enable(RCC_GPIOC);	//Conf. de led en tarjeta.
	gpio_set_mode(GPIOC,
		GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		GPIO13);

	rcc_periph_clock_enable(RCC_GPIOA);	//Conf. del sel y set.
	gpio_set_mode(GPIOA,
		GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_FLOAT,
		GPIO4|GPIO5);

	spi_oled_init();	//Conf. e inicialización de la OLED.
	adc_init();	//Init. del periférico ADC.

	mi_marca();	//Dibuja mi marda por 5 seg.
	frame();	//Dibuja recuadro y espera 2ms.

	//--> Se encarda solo de mostrar la temperatura en oled cada 0.5s.
	xTaskCreate(tarea1,"IMP_TEMP",100,NULL,configMAX_PRIORITIES-1,NULL);
	//--> Se encarga de obtener el modo de control y sus parámetros.
	xTaskCreate(tarea2,"M_C_PRTOS",100.NULL,configMAX_PRIORITIES-1,NULL);

	vTaskStartScheduler();
	for (;;)
		;
	return 0;
}

// End
