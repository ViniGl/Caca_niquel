/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define MOTOR_PIO           PIOD
#define MOTOR_PIO_ID        ID_PIOD
#define MOTOR_PIO_IDX       22u
#define MOTOR_PIO_IDX_MASK  (1u << MOTOR_PIO_IDX)

#define DRIVER_PIO   PIOD
#define DRIVER_PIO_ID	ID_PIOD
#define DRIVER_PIO_IDX	20u
#define DRIVER_PIO_IDX_MASK (1u << DRIVER_PIO_IDX)

#define ENC1_PIO PIOA
#define ENC1_ID ID_PIOA
#define ENC1_IDX 4
#define ENC1_MASK (1 << ENC1_IDX)

#define BUZZ_PIO PIOB
#define BUZZ_PIO_ID ID_PIOB
#define BUZZ_IDX 2u
#define BUZZ_MASK (1u<< BUZZ_IDX)

#define LED_PIO           PIOC
#define LED_PIO_ID        ID_PIOC
#define LED_PIO_IDX       8u
#define LED_PIO_IDX_MASK  (1u << LED_PIO_IDX)

#define BUT_PIO   PIOA
#define BUT_PIO_ID	ID_PIOA
#define BUT_PIO_IDX	11u
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/


#define YEAR        2019
#define MOUNTH      1
#define DAY         1
#define WEEK        1
#define HOUR        0
#define MINUTE      0
#define SECOND      0

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/



/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);
void RTC_init(void);


/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

volatile uint8_t flag = 1;
volatile uint32_t counter = 0;
volatile bool turned = true;
volatile bool losing_money = false;


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void RTC_Handler(void)
{	unsigned int hora, minuto, segundo;
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);

			losing_money = false;
		
	}
	/*if (losing_money){
		pio_set(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
		pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
	} else {
		pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
		pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
	}*/
	

	
	pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
	pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
	
		
	
	//rtc_get_time(RTC, &hora,&minuto,&segundo);
	//rtc_set_time_alarm(RTC,1,hora,1,minuto,1,segundo+1);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}


void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MOUNTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_ALREN);

}

void count_rpm(void){ 
	counter +=1;
	
}

void betting(void){
	losing_money = true;
}

// Função de inicialização do uC
void init(void)
{
	// Initialize the board clock
	sysclk_init();
	
	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	pmc_enable_periph_clk(MOTOR_PIO);
	pmc_enable_periph_clk(DRIVER_PIO_ID);
	//pmc_enable_periph_clk(LED_PIO_ID);
	
	pmc_enable_periph_clk(BUT_PIO_ID);
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT_PIO_ID,BUT_PIO_IDX_MASK,1);

	pio_set_output(MOTOR_PIO, MOTOR_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(DRIVER_PIO, DRIVER_PIO_IDX_MASK, 0, 0, 0);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_PIO_IDX_MASK,PIO_DEFAULT);
		
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	betting);
	
	pio_enable_interrupt(BUT_PIO, BUT_PIO_IDX_MASK);
	NVIC_EnableIRQ(BUT_PIO_ID);
  
  	pio_configure(ENC1_PIO, PIO_INPUT, ENC1_MASK, PIO_DEFAULT);
	pio_handler_set(ENC1_PIO, ENC1_ID , ENC1_MASK,	PIO_IT_FALL_EDGE,	count_rpm);
	pio_enable_interrupt(ENC1_PIO, ENC1_MASK);
	NVIC_EnableIRQ(ENC1_ID);
}

void rolling(){
	//pio_set(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
	//pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
	
	rtc_set_date_alarm(RTC, 1, MOUNTH, 1, DAY);
	rtc_set_time_alarm(RTC,1, HOUR, 1, MINUTE, 1, SECOND+5);
}



/**
 * \brief Configure the console UART.
 */
static void configure_console(void)
{
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
	const usart_serial_options_t uart_serial_options = {
		.baudrate   = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits   = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();
  configure_console();


  pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
  pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
   
  
  /** Configura RTC */
 // RTC_init();

  /* configura alarme do RTC */
  //rtc_set_date_alarm(RTC, 1, MOUNTH, 1, DAY);
  //rtc_set_time_alarm(RTC,1, HOUR, 1, MINUTE, 1, SECOND+1);
  

  while(1){
	  if (losing_money){
		//  rolling();
		if(counter < 11){
		  pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
		  pio_set(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
		  delay_ms(10);
		  pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
		  pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
		  delay_ms(10);

		}else{
			pio_clear(MOTOR_PIO,MOTOR_PIO_IDX_MASK);
			pio_clear(DRIVER_PIO,DRIVER_PIO_IDX_MASK);
			delay_s(1);
			counter = 0;
		}
	  }	  
  }
  return 0;
}
