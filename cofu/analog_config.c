/* Este módulo contendrá el código para
 * la configuración del canal analógico.
 * analogico_confif.c
 *
 */


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>

//#include "mcuio.h"
//#include "miniprintf.h"

/*********************************************************************
* Prototipos de funciones.
*********************************************************************/
void adc_init(void);		//-> Inicialización del ADC.

/*********************************************************************
 * Programa para la inicialización del ADC
 *********************************************************************/
void
adc_init(void)
{

       rcc_clock_setup_in_hse_8mhz_out_72mhz();        // Use this for "blue pill"

        rcc_periph_clock_enable(RCC_GPIOA);		// Enable GPIOA for ADC
	gpio_set_mode(GPIOA,
		GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_ANALOG,
		GPIO0/*|GPIO1*/);				// PA0 & PA1

	// Initialize ADC:
	rcc_peripheral_enable_clock(&RCC_APB2ENR,RCC_APB2ENR_ADC1EN);
	adc_power_off(ADC1);
	rcc_peripheral_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_peripheral_clear_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV6);	// Set. 12MHz, Max. 14MHz
	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);		// Independent mode
	adc_disable_scan_mode(ADC1);
	adc_set_right_aligned(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_sample_time(ADC1,ADC_CHANNEL_TEMP,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_sample_time(ADC1,ADC_CHANNEL_VREF,ADC_SMPR_SMP_239DOT5CYC);
	adc_enable_temperature_sensor();
	adc_power_on(ADC1);
	adc_reset_calibration(ADC1);
	adc_calibrate_async(ADC1);
	while ( adc_is_calibrating(ADC1) );

//  std_set_device(mcu_usb);
//  usb_start(1,1);

}

// End main.c
