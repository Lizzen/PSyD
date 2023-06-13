// Ejercicio 3
#include <s3c44b0x.h>
#include <common_types.h>
#include <lcd.h>

void memcpy_dma( uint8 *dest, uint8 *src, uint16 n )
{

	ZDISRC0 = (0 << 30) | (1 << 28) |  (uint32) src;            					// datos de 8b, dirección post-incrementada
	ZDIDES0 = (2 << 30) | (1 << 28) |  (uint32) dest;            				// recomendada, dirección post-incrementada
	ZDICNT0 = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | n;    	// whole service, unit transfer, polling mode, no autoreload
	ZDICNT0 |= (1 << 20);                              					// Enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
	ZDCON0  = 1;                                          				// start DMA
	while( ZDCCNT0 & 0xFFFFF );
}


// Ejercicio 4

#include <s3c44b0x.h>
#include <common_types.h>
#include <lcd.h>

void lcd_vflip(void){
	uint16 y;

	for(y = 0; y < LCD_HEIGHT; ++y){
		memcpy_dma(lcd_buffer + (y*(LCD_WIDTH/2)), lcd_buffer + ((LCD_HEIGHT - y)*(LCD_WIDTH/2)),  (LCD_WIDTH/2));
	}
}

//Ejercicio 1

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <timers.h>
#include <uart.h>

#define MAXLEN       (256)

void timer0_isr( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile char ch;
volatile char cadena[MAXLEN];

void main(void){
	sys_init();
	uart0_init();
	timers_init();

	flag = FALSE;
	timer0_open_ms( timer0_isr, 20, TIMER_INTERVAL ); 	//interrumpe cada 20ms

	while( 1 )
		if( flag )
		{
			uart0_puts(cadena);
			flag = FALSE;
		}
}

void timer0_isr( void )
{
	if( UFSTAT0 & 0xF )
	{
		char aux;
		aux = URXH0;

		while (aux != '\n'){
			*cadena = aux;
			*++cadena;
			aux = URXH0;
		}
		*cadena  = '\0';
		flag = TRUE;
	}

	I_ISPC = BIT_TIMER0;
}

// Ejercicio 2

#include <s3c44b0x.h>
#include <common_types.h>
#include <lcd.h>

void lcd_putWindow(uint8 *bmp, uint16 xleft, uint16 yup, uint16 xright, uint16 ydown){
	uint16 yAux, xAux;
	uint32 headerSize;
	uint16 lenght, width;
	uint16 offsetSrc, offsetDst;

	headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);

	bmp = bmp + headerSize;

	width = 1;

	offsetDst = yup*LCD_WIDTH/2;
	offsetSrc = yup*LCD_WIDTH/2;
	for (xAux = xleft; xAux <= xright; ++xAux){
		lcd_buffer[offsetDst + xAux] = ~bmp[offsetSrc + xAux];
	}

	offsetDst = ydown*LCD_WIDTH/2;
	offsetSrc = ydown*LCD_WIDTH/2;
	for (xAux = xright; xAux >= xleft; --yAux){
		lcd_buffer[offsetDst + xAux] = ~bmp[offsetSrc + xAux];
	}

	for (yAux = yup + 1; yAux < ydown; ++yAux){
		offsetDst = yAux*LCD_WIDTH/2;
		offsetSrc = yAux*LCD_WIDTH/2;
		lcd_buffer[offsetDst + xleft] = ~bmp[offsetSrc + xleft];
		lcd_buffer[offsetDst + xright] = ~bmp[offsetSrc + xright];
	}
}

