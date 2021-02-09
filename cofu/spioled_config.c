/* Demo program for OLED 128x64 SSD1306 controller
 * Warren Gay   Sun Dec 17 22:49:14 2017
 *
 * Important!  	You must have a pullup resistor on the NSS
 * 	       	line in order that the NSS (/CS) SPI output
 *		functions correctly as a chip select. The
 *		SPI peripheral configures NSS pin as an
 *		open drain output.
 *
 * OLED		4-Wire SPI
 *
 * PINS:
 *	PC13	LED
 *	PA15	/CS (NSS, with 10k pullup)
 *	PB3	SCK
 *	PB5	MOSI (MISO not used)
 *	PB10	D/C
 *	PB11	/Reset
 */



#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


//-> Prototipos de funciones
void spi_oled_init(void);

//--> Función de inicio del SPI.
void
spi_oled_init(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz();	// Blue pill

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_SPI1);

	// Put SPI1 on PB5/PB4/PB3/PA15
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF, // Optional
										AFIO_MAPR_SPI1_REMAP);

	// PB10 -> D/C, PB11 -> RES
	gpio_set_mode(GPIOB,
		GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		GPIO10|GPIO11);
	// Activate OLED Reset line
	gpio_clear(GPIOB,GPIO11);

	// PB5=MOSI, PB3=SCK
	gpio_set_mode(GPIOB,
          GPIO_MODE_OUTPUT_50_MHZ,
        	GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
          GPIO5|GPIO3);
	// NSS=PA15
	gpio_set_mode(GPIOA,
          GPIO_MODE_OUTPUT_50_MHZ,
        	GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
          GPIO15);

	spi_reset(SPI1);
	spi_init_master(	SPI1,
          SPI_CR1_BAUDRATE_FPCLK_DIV_256,
          SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1,
	        SPI_CR1_DFF_8BIT,
	        SPI_CR1_MSBFIRST);

	spi_disable_software_slave_management(SPI1);
	spi_enable_ss_output(SPI1);

	gpio_toggle(GPIOC,GPIO13);

}

// End
