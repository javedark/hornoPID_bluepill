/* Master_horno_pidof. El programa es para un microcontrolador. Realiza el control
 * de un horno bajo dos modos a elección del usuario: Control ON-OFF ó Control PID.
 * En ambos modos se ajustan parámetros de funcionamiento.
 *
 * Ing. MasterJER. 02]/12/2020
 *
 */

 //*****Librerias requeridas*****

 #include "FreeRTOS.h"					//FreeRTOS.
 #include "task.h"							//Uso de tareas
 #include "semphr.h"
 #include "includes/ugui.h"
 #include "includes/miniprintf.h"			//Para la impresión de texto.

 #include <libopencm3/cm3/cortex.h>
 #include <libopencm3/stm32/rcc.h>
 #include <libopencm3/stm32/gpio.h>
 #include <libopencm3/cm3/nvic.h>
 #include <libopencm3/stm32/spi.h>
 #include <libopencm3/stm32/adc.h>

 #include "includes/parauso_ugui.h"		//Config. de ugui.
 #include "includes/oled_fun.h"				//Funciones para el uso de la OLED.
 #include "includes/spioled_config.h"	//Config. para la pantalla OLED.
 #include "includes/mis_img.h"				//Mis imagenes, usan for para los retardos.
 #include "includes/analog_config.h"	//Config. para el uso del canal analógico.
 #include "includes/controles.h"			//Algoritmos de control. Master_Oled_Menu

//--> Definiciones.

#define SELECT GPIOA,GPIO4		//Master_Oled_Menu
#define LED 	 GPIOC,GPIO13		//Master_Oled_Menu
#define SET 	 GPIOA,GPIO5		//Master_Oled_Menu

//--> Handles.

static SemaphoreHandle_t h_mutex;
static TaskHandle_t xTarea1 = NULL, xTarea2 = NULL, xTarea3 = NULL;

//--> Variables globales.

static int lim_inf; // Límite inferior del modo ON-OFF.Master_Oled_Menu
static int lim_sup; // Límite superior del modo ON-OFF.Master_Oled_Menu
static int8_t gan_P;   // Ganancia proporcional.
static int8_t gan_I;   // Ganancia integral.
static int8_t gan_D;   // Ganancia derivativa.

//--> Mutex lock y mutex unlock.

static void
mutex_lock(void){
  xSemaphoreTake(h_mutex,portMAX_DELAY);
}

static void
mutex_unlock(void){
  xSemaphoreGive(h_mutex);
}

//--> Prototipos de funciones.

//Para la impresión de datos en la oled con mutexes.
void imp_oled_mutexes(void);
//Para el mapeo del valor analógico.Master_horno
long mapeo(long x, long in_min, long in_max, long out_min, long out_max);
//Para la lectura del canal analógico.Master_horno
static uint16_t read_adc(uint8_t channel);
//Para determinar si el botón fue presionado.Master_Oled_Menu
bool	press(uint32_t PUERTO,uint16_t PIN);
//Parámetros del modo de control ON-OFF
void param_control_onoff(void);
//Parámetros del modo de control PID
void param_control_pid(void);
//Para la lectura de temperatura.
int temp(void);
//Muestra la temperatura en oled.
void imp_temp(void);

//--> Definición de funciones.

//Función para la impresión de datos en oled usando mutexes.
void imp_oled_mutexes(void){
  mutex_lock();
  mapa_to_oled();
  mutex_unlock();
}
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
  UG_FontSelect(&FONT_8X12);
	UG_PutString(91,15,buf);
	imp_oled_mutexes();
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
//F. Para establecer los parámetros del modo de control ON_OFF
void param_control_onoff(void){
	// Límite inferior.
	limpia_seccion();
  char buf[16];
  UG_FontSelect(&FONT_5X8);
  UG_PutString(3,6,"Establece el");
  UG_PutString(3,16,"Limite INF:");
  UG_PutString(3,47,"grados cent.");
  imp_oled_mutexes();
  for(int i = 0; i <= 36; i = i+2){
    mini_snprintf(buf,sizeof buf, "%d",i);
    UG_FontSelect(&FONT_8X12);
    UG_PutString(30,30,buf);
    imp_oled_mutexes();
    while(press(SELECT) == pdFALSE){
      if(press(SET)){lim_inf = i; i = 37; break;}
    }
  }
	// Límite superior.
	limpia_seccion();
	UG_FontSelect(&FONT_5X8);
	UG_PutString(3,6,"Establece el");
	UG_PutString(3,16,"Limite SUP:");
	UG_PutString(3,47,"grados cent.");
	imp_oled_mutexes();
	for(int i = lim_inf + 2; i <= 36; i = i +2){
		mini_snprintf(buf,sizeof buf, "%d",i);
		UG_FontSelect(&FONT_8X12);
		UG_PutString(30,30,buf);
		imp_oled_mutexes();
		while(press(SELECT) == pdFALSE){
			if(press(SET)){lim_sup = i; i = 37; break;}
		}
	}

	// Pantalla muestra modo de control ON-OFF.
	limpia_seccion();
	UG_FontSelect(&FONT_8X8);
	UG_PutString(3,6,"*ON-OFF");
	UG_PutString(3,22,"L.SUP:");
	UG_PutString(3,44,"L.INF:");
	mini_snprintf(buf,sizeof buf,"%d",lim_sup);
	UG_PutString(55,22,buf);
	mini_snprintf(buf,sizeof buf,"%d",lim_inf);
	UG_PutString(55,44,buf);
	imp_oled_mutexes();
}

//F. para establecer los parámetros del modo de contro PIDvoid
param_control_pid(void){
	// Ganancia proporcional.
	limpia_seccion();
  char buf[16];
  UG_FontSelect(&FONT_5X8);
  UG_PutString(3,6,"Establece la");
  UG_PutString(3,16,"Ganancia PROP:");
  imp_oled_mutexes();
  for(int i = 0; i <= 30; i = i+2){
    mini_snprintf(buf,sizeof buf, "%d",i);
    UG_FontSelect(&FONT_8X12);
    UG_PutString(30,30,buf);
    imp_oled_mutexes();
    while(press(SELECT) == pdFALSE){
      if(press(SET)){gan_P = i; i = 30; break;}
    }
  }
	// Ganancia integrativa
	limpia_seccion();
	UG_FontSelect(&FONT_5X8);
	UG_PutString(3,6,"Establece la");
	UG_PutString(3,16,"Ganancia INT:");
	imp_oled_mutexes();
	for(int i = 0 ; i <= 30; i = i +2){
		mini_snprintf(buf,sizeof buf, "%d",i);
		UG_FontSelect(&FONT_8X12);
		UG_PutString(30,30,buf);
		imp_oled_mutexes();
		while(press(SELECT) == pdFALSE){
			if(press(SET)){gan_I = i; i = 30; break;}
		}
	}
  // Ganancia derivativa
  limpia_seccion();
  UG_FontSelect(&FONT_5X8);
  UG_PutString(3,6,"Establece la");
  UG_PutString(3,16,"Ganancia DER:");
  imp_oled_mutexes();
  for(int i = 0 ; i <= 30; i = i +2){
    mini_snprintf(buf,sizeof buf, "%d",i);
    UG_FontSelect(&FONT_8X12);
    UG_PutString(30,30,buf);
    imp_oled_mutexes();
    while(press(SELECT) == pdFALSE){
      if(press(SET)){gan_D = i; i = 30; break;}
    }
  }

	// Pantalla muestra modo de control PID
	limpia_seccion();
	UG_FontSelect(&FONT_8X8);
	UG_PutString(3,6,"**PID**");
	UG_PutString(3,22,"G.P.:");
	UG_PutString(3,38,"G.I.:");
  UG_PutString(3,50,"G.D.:");
	mini_snprintf(buf,sizeof buf,"%d",gan_P);
	UG_PutString(55,22,buf);
	mini_snprintf(buf,sizeof buf,"%d",gan_I);
	UG_PutString(55,38,buf);
  mini_snprintf(buf,sizeof buf,"%d",gan_D);
	UG_PutString(55,50,buf);
	imp_oled_mutexes();
}


//--> Tarea 1
static void
tarea1(void *args) {
	(void)args;
  mi_marca();	//Dibuja mi marda por 5 seg.
	frame();	//Dibuja recuadro y espera 2ms.
  opciones();
  vTaskDelay(pdMS_TO_TICKS(100));
  xTaskNotifyGive(xTarea2);
  for(;;){
	   imp_temp();
	    vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//--> Tarea 2
 static void
 tarea2(void *args){
   (void)args;
   uint8_t sel_opc = 0, ejec_opc = 0;
   for(;;){
     ulTaskNotifyTake(pdTRUE,portMAX_DELAY); //Espera a la notificación de tarea1.
     // Selección de opciones
     while (ejec_opc == 0) {
       switch (sel_opc) {
         case 0:
         UG_FillCircle(15,48,6,pen_to_ug(0));
         UG_FillCircle(15,28,6,pen_to_ug(1));
         UG_DrawCircle(15,48,6,pen_to_ug(1));
         imp_oled_mutexes();
         while(1){
           if(press(SELECT)){sel_opc = 1; break;}
           if(press(SET)){ejec_opc = 1; break;}
         } break;

         case 1:
         UG_FillCircle(15,28,6,pen_to_ug(0));
         UG_DrawCircle(15,28,6,pen_to_ug(1));
         UG_FillCircle(15,48,6,pen_to_ug(1));
         imp_oled_mutexes();
         while(1){
           if(press(SELECT)){sel_opc = 0; break;}
           if(press(SET)){ejec_opc = 2; break;}
         } break;
       }
     }

     // Ejecución de opciones.
     switch (ejec_opc) {
       case 1:
       param_control_onoff();
       xTaskNotifyGive(xTarea3);
       gpio_clear(LED);
//       while(1);
       break;

       case 2:
       param_control_pid();
       gpio_clear(LED);
//       while(1);
       break;
     }   vTaskSuspend(xTarea2);
   }// Cierre del for
 }// Final de la tarea 2

//--> Tarea 3
static void
tarea3(void *args){
  (void)args;
  for(;;){
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    while(1){
      if(temp() > lim_sup){
         gpio_clear(GPIOB,GPIO0);
         UG_FillFrame(90,44,126,62,pen_to_ug(0));
         imp_oled_mutexes();
       }
    vTaskDelay(pdMS_TO_TICKS(30));
      if(temp() < lim_inf) {
        gpio_set(GPIOB,GPIO0);
        UG_FillFrame(90,44,126,62,pen_to_ug(1));
        imp_oled_mutexes();
      }
    vTaskDelay(pdMS_TO_TICKS(30));
    }
  }
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

  rcc_periph_clock_enable(RCC_GPIOB); //Conf. de la salida a la lámpara.
  gpio_set_mode(GPIOB,
    GPIO_MODE_OUTPUT_2_MHZ,
    GPIO_CNF_OUTPUT_PUSHPULL,
    GPIO0);

  gpio_set(GPIOB,GPIO0);

	spi_oled_init();	//Conf. e inicialización de la OLED.
	adc_init();	//Init. del periférico ADC.




  h_mutex = xSemaphoreCreateMutex();
	//--> Se encarda solo de mostrar la temperatura en oled cada 0.5s.
	xTaskCreate(tarea1,"IMP_TEMP",100,NULL,configMAX_PRIORITIES-1,&xTarea1);
	//--> Se encarga de obtener el modo de control y sus parámetros.
	xTaskCreate(tarea2,"M_C_PRTOS",100,NULL,configMAX_PRIORITIES-2,&xTarea2);
  //--> Se encarga de ejecutar el modo de control ON-ON_OFF
  xTaskCreate(tarea3,"M_C_OnOff",100,NULL,configMAX_PRIORITIES-3,&xTarea3);

	vTaskStartScheduler();
	for (;;)
		;
	return 0;
}

// End
