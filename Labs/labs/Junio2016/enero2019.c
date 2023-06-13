/*1.(1.5 puntos) Codificar en C una aplicación que por interrupción periódica realice un loopback de
audio, es decir, retransmita a través del interfaz IIS de la placa de prototipado S3CEV40 las
muestras recibidas por ese mismo interfaz. El microcontrolador S3C44B0X deberá configurarse
para que su timer-0 interrumpa 16000 veces por segundo (equivalente a una frecuencia de muestreo
de 16 KHz). La aplicación constará de 2 hebras que usarán un mailbox para comunicarse: una hebra
en foreground (isr_timer0) encargada de leer dos muestras (una por canal) y de comunicarlas a
la otra hebra; y una hebra en background (main) encargada de inicializar el sistema, inicializar el
audio códec, instalar la hebra en foreground como RTI del timer-0 e indefinidamente esperar el
aviso de la otra hebra para retransmitir las muestras recibidas. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <timers.h>
#include <uda1341ts.h>
#include <iis.h>

void isr_timer0( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile int16 ch0, ch1;

void main( void )
{
    sys_init();
    timers_init();
    uda1341ts_init();
    iis_init( IIS_POLLING );

    flag = FALSE;
    timer0_open_tick( isr_timer0, 16000 );
    while( 1 )
        if( flag )
        {
            IISFIF = ch0;
            IISFIF = ch1;
            flag = FALSE;
        }
}

void isr_timer0( void )
{

    ch0 = IISFIF;
    ch1 = IISFIF;
    flag = TRUE;
    I_ISPC = BIT_TIMER0;
}

/*2.(1.5 punto) Codificar en C una función que indique el tipo de interacción del usuario con el
pulsador de la placa de prototipado S3CEV40 indicado en el argumento. Esta función devolverá 1
si el usuario ha realizado una pulsación corta (inferior a 1 segundo entre presión y depresión) y
devolverá 0 en cuanto haya transcurrido 1 segundo desde la presión del pulsador, sin necesidad de
esperar su depresión. El prototipo de la función será: */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <timers.h>
#include <pbs.h>

uint8 pbs_behavior( uint8 scancode )
{
    while( PDATG & scancode );
       timer3_start_timeout( 10000 );

       sw_delay_ms( PB_KEYDOWN_DELAY );

    while( !(PDATG & scancode) && !timer3_timeout() );
           if( timer3_timeout() ) return 0;
    sw_delay_ms( PB_KEYUP_DELAY );
    return 1;
}

/*3.(0.5 punto) Codificar en C una función que, usando una fuente 8×16, escriba sobre el LCD de la
placa de prototipado S3CEV40 un carácter a partir del pixel (x,y) con el color indicado pero con el
doble de anchura que la que se obtendría si se llamase a la funcion lcd_putchar. El prototipo de
la función será: */

#include <common_types.h>
#include <lcd.h>

extern uint8 font[];

void lcd_putchar_wider( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16; line++)
        for( row=0; row<16; row++)
            if( bitmap[line] & (0x80 >> row/2) )
                lcd_putpixel( x+row, y+line, color );
            else
                lcd_putpixel( x+row, y+line, WHITE );
}

/*4.(0.5 puntos) Suponiendo que la frecuencia de reloj del microcontrolador S3C44B0X es 64 MHz,
codificar en C una función que establezca la velocidad de comunicación de la UART-0 (en baudios)
que se indica como argumento. El prototipo de la función será: */

#include <s3c44b0x.h>
#include <common_types.h>
#include <uart.h>

#define MCLK ((uint32)64000000U)

void uart0_setBaudRate( uint16 baudRate )
{
    UBRDIV0 = ((MCLK >> 4) / baudRate) - 1;
//     UBRDIV0 = (MCLK / (baudRate << 4)) - 1;     // (MCLK/(baudRate*16)) - 1

}
