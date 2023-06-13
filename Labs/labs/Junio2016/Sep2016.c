/*1.(2 puntos) Codificar en C una aplicación que conmute el estado del led izquierdo de la placa de
prototipado Embest S3CEV40 cada vez que detecte la presión del pulsador izquierdo.
Inicialmente el led estará apagado.
La aplicación constará de 2 hebras que usarán un flag para comunicarse: una hebra en foreground
(pbs_rti) encargada de activar el flag a cada presión o depresión del pulsador (incluyendo
rebotes); y una hebra en background (main) encargada de inicializar el sistema, instalar la hebra
en foreground como RTI del pulsador e indefinidamente hacer la conmutación del led a cada
presión del pulsador filtrando sus rebotes.*/
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <system.h>
#include <leds.h>
#include <pbs.h>
#include <timers.h>

void pbs_rti( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;

void main( void )
{
    sys_init();
    leds_init();
    pbs_init();
    timers_init();

    flag = FALSE;
    PCONG  &= ~(3<<14);                           // Pulsador derecho: Desconectado del controlador de interrupciones
    EXTINT  = (EXTINT & ~(0xf<<24)) | (7<<24);    // Pulsador izquierdo: Both edge triggered
    pbs_open( pbs_rti );

    while( 1 )
        if( flag )
        {
            led_toggle( LEFT_LED );
            sw_delay_ms( PB_KEYDOWN_DELAY );
            while( pb_status( PB_LEFT ) );
            sw_delay_ms( PB_KEYUP_DELAY );
            flag = FALSE;
        }
}

void pbs_rti( void )
{
    flag = TRUE;
    EXTINTPND = BIT_LEFTPB;
    I_ISPC = BIT_PB;
}

/*2.(0.5 puntos) Codificar en C una función que calcule en punto fijo con representación Q8.8 el
área de un círculo dado su perímetro. Se supondrá que los valores del argumento y del resultado
se expresan en la misma unidad de medida. El prototipo de la función será:

Recuérdese que: Área = pi*(r)^2 / Perímetro = 2pi*r / pi = 3.141592654*/


#include <fix_types.h>

#define QM 8
typedef uint16 ufix16;
ufix16 area( ufix16 perimeter )
{
    const ufix16 quadruplePi = TOFIX( ufix16, 4*3.141592654, QM );

    return FDIV( FMUL( perimeter, perimeter, QM ), quadruplePi, QM );
}

/*3.(0.5 puntos) Codificar en C una función que envíe por la UART-0 la velocidad exacta de
transmisión en baudios de dicho interfaz suponiendo que la frecuencia de reloj del
microcontrolador S3C44B0X es de 64 MHz. El prototipo de la función será:

Así, una llamada a dicha función visualizará en un terminal serie conectado a la UART-0 una
información similar a la siguiente:
La velocidad de transmisión exacta de esta UART es de ... baudios.
Para los cálculos solo se permite usar aritmética entera. */

#include <s3c44b0x.h>
#include <uart.h>

#define MCLK ((uint32)64000000U)

void uart0_putBaudRate( void )
{
    uart0_puts( "La velocidad exacta de transmisión de esta UART es de " );
    uart0_putint( (MCLK/(UBRDIV0 + 1)) >> 4 );
    uart0_puts( " baudios.\n" );
}
