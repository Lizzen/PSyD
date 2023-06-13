/*1.(1.5 puntos) Codificar en C una aplicación que por interrupción muestre por el display 7 segmentos
de la placa de prototipado S3CEV40 el dígito hexadecimal asociado a cada tecla que se pulse en el
del keypad y a la vez envíe por la UART-0 el código ASCII correspondiente a dicho dígito. La
aplicación constará de 2 hebras que usarán un mailbox para comunicarse: una hebra en foreground
(keypad_isr) encargada de leer el keypad y de enviar a la otra hebra el resultado siempre que este
sea válido; y una hebra en background (main) encargada de inicializar el sistema, instalar la hebra
en foreground como RTI del keypad e indefinidamente esperar la recepción de mensajes para
visualizarlos y transmitirlos. */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>
#include <segs.h>
#include <keypad.h>

void keypad_isr( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile char ch;

void main( void )
{
    sys_init();
    uart0_init();
    segs_init();
    keypad_init();

    flag = FALSE;
    keypad_open( keypad_isr );
    while( 1 )
        if( flag )
        {
            segs_putchar( ch );
            ch += ( ch < 10 ? '0' : 'a'-10 );
            uart0_putchar( ch );
            flag = FALSE;
        }

}

void keypad_isr( void )
{
    ch = keypad_getchar();
    if( ch != KEYPAD_FAILURE )
        flag = TRUE;
    I_ISPC = BIT_KEYPAD;
}

/*2.(1.5 puntos) Suponiendo que la frecuencia del oscilador externo conectado a la entrada de reloj del
microcontrolador S3C44B0X es 8 MHz, codificar en C una función que envíe por la UART-0 el
modo (NORMAL o SLOW) y la frecuencia de reloj a la que está funcionando el microcontrolador.
El prototipo de la función será:

Así, una llamada a dicha función visualizará en un terminal serie conectado a la UART-0 una
información similar a la siguiente:
El microcontrolador está en modo ... a una frecuencia de ... Hz. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <uart.h>

#define Fin ((uint32)8000000U)

#define CLK_NORMAL (0)
#define CLK_SLOW   (1)

void uart0_putClockFreq( void )
{
    uint8 slow_bit, slow_val;
    uint8 mdiv, pdiv, sdiv;

    slow_bit = ( CLKSLOW & (1 << 4) ) >> 4;
    slow_val = CLKSLOW & 0xf;

    mdiv = (PLLCON & (0xff << 12) ) >> 12;
    pdiv = (PLLCON & (0x3f << 4) ) >> 4;
    sdiv = PLLCON & 0x3;

    if( slow_bit == CLK_SLOW ) {
        uart0_puts( "El microcontrolador está en modo SLOW a una frecuencia de " );
        if( slow_val )
            uart0_putint( Fin / (2*slow_val) );                   // Ver pag. 40 del tema 1
        else
            uart0_putint( Fin );
        uart0_puts( " hercios\n" );
    } else {
        uart0_puts( "El microcontrolador está en modo NORMAL a una frecuencia de " );
        uart0_putint( ((mdiv + 8)*Fin) / ((pdiv+2) << sdiv) );    // Ver pag. 38 del tema 1
        uart0_puts( " hercios\n" );
    }
}

/*3.(0.5 puntos) Codificar en C una función que borre la pantalla LCD (rellenando con 0 el
lcd_buffer) de la placa de prototipado S3CEV40 utilizando el controlador de DMA ZDMA0. El
prototipo de la función será: */

#include <lcd.h>
extern uint8 lcd_buffer[];

void lcd_clearDMA( void )
{
    uint32 zero;

    zero    = 0;
    ZDISRC0 = (2 << 30) | (3 << 28) | (uint32) &zero;         // datos de 32b, dirección fija
    ZDIDES0 = (2 << 30) | (1 << 28) | (uint32) lcd_buffer;    // recomendada, dirección post-incrementada
    ZDICNT0 = (2 << 28) | (1 << 26) | LCD_BUFFER_SIZE;        // whole service, unit transfer, polling mode, no autoreload
    ZDICNT0 |= (1 << 20);                                     // Enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
    ZDCON0  = 1;                                              // start DMA
}

/*4.(0.5 puntos) Codificar en C una función que, usando una fuente 8×16, escriba sobre el LCD de la
placa de prototipado S3CEV40 un carácter a partir del pixel (x,y) en vídeo invertido. Es decir, dado
el mapa de bits del carácter, los foreground pixels deberán mostrarse en blanco mientras que los
background pixels deberán mostrarse en el color indicado. */


#include <common_types.h>
#include <lcd.h>

extern uint8 font[];
extern uint8 lcd_buffer[];

void lcd_putchar_inverted( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16; line++ )
        for( row=0; row<8; row++ )
            if( bitmap[line] & (0x80 >> row) )
                lcd_putpixel( x+row, y+line, WHITE );
            else
                lcd_putpixel( x+row, y+line, color );
}
