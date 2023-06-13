/*
 * Junio2016.c
 *
 *  Created on: 18/01/2023
 *      Author: DOSTATIC
 */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>
#include <rtc.h>

/*1. (1.5 puntos) Codificar en C una aplicaci�n que env�e un texto de aviso por la UART-0 cada vez
que transcurra una hora. El microntrolador S3C44B0X deber� configurarse para que su RTC
interrumpa una vez por segundo. La aplicaci�n constar� de 2 hebras: una hebra en foreground
(isr_tick) encargada de actualizar los segundos transcurridos desde el �ltimo aviso; y una hebra
en background (main) encargada de inicializar el sistema, instalar la hebra en foreground como
RTI del RTC e indefinidamente enviar el mensaje de aviso cada hora. Ambas hebras usar�n un
flag para sincronizarse. */

void isr_tick( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;

void main( void )
{
    sys_init();
    uart0_init();
    rtc_init();

    flag = FALSE;
    rtc_open( isr_tick, 127 );

    while( 1 )
        if( flag )
        {
            uart0_puts( "Ha transcurrido una hora.\n" );
            flag = FALSE;
        }
}

void isr_tick( void )
{
    static uint16 numSecs = 0;

    if( ++numSecs == 3600 )
    {
        numSecs = 0;
        flag = TRUE;
    }

    I_ISPC = BIT_TICK;
}

/*2.(0.5 puntos) Codificar en C una funci�n que calcule en punto fijo con representaci�n Q8.8 el
per�metro de una circunferencia dado su radio. Se supondr� que los valores del argumento y del
resultado se expresan en la misma unidad de medida. El prototipo de la funci�n ser�: */

#include <fix_types.h>

#define QM 8
typedef uint16 ufix16;
ufix16 circumference( ufix16 radius )
{
    const ufix16 doublePi = TOFIX( ufix16, 2*3.141592654, QM );

    return FMUL( doublePi, radius, QM );
}

/*3.(1 punto) Codificar en C una funci�n que, dada la direcci�n del registro de configuraci�n de uno
de los puertos del controlador del pines de E/S del microntrolador S3C44B0X, env�e por la
UART-0 el modo (entrada, salida o interno) en que se haya configurado cada uno de los pines de
dicho puerto. El prototipo de la funci�n ser�:

Visualizar� en un terminal serie conectado a la UART-0 una informaci�n similar a la siguiente:
CONFIGURACION DEL PUERTO A:
Pin 0: salida
Pin 1: interno
Pin 2: interno
Pin 3: interno
Pin 4: interno
Pin 5: interno
Pin 6: interno
Pin 7: interno
Pin 8: salida
Pin 9: salida
No ser� necesario implementar al completo la funci�n, bastar� codificar la estructura general de
la misma as� como el detalle del c�digo necesario para la visualizaci�n de la configuraci�n de los
puertos A y C. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <uart.h>

void uart0_portDump( uint32 portConfAddr )
{
    uint32 pConf;
    uint8 i;

    switch( portConfAddr )
    {
        case (uint32)&PCONA:
            uart0_puts( "CONFIGURACION DEL PUERTO A\n" );
            pConf = PCONA;
            for( i=0; i<10; i++ )
            {
                uart0_puts( "Pin " );
                uart0_putint( i );
                switch( pConf & 0x1 )
                {
                    case 0:
                        uart0_puts( ": salida\n" );
                        break;
                    default:
                        uart0_puts( ": interno\n" );
                        break;
                }
                pConf >>= 1;
            }
            break;
        case (uint32)&PCONB:
            break;
        case (uint32)&PCONC:
            uart0_puts( "CONFIGURACION DEL PUERTO C\n" );
            pConf = PCONC;
            for( i=0; i<16; i++ )
            {
                uart0_puts( "Pin " );
                uart0_putint( i );
                switch( pConf & 0x3 )
                {
                    case 0:
                        uart0_puts( ": entrada\n" );
                        break;
                    case 1:
                        uart0_puts( ": salida\n" );
                        break;
                    default:
                        uart0_puts( ": interno\n" );
                        break;
                }
                pConf >>= 2;
            }
            break;
        case (uint32)&PCOND:
            break;
        case (uint32)&PCONE:
            break;
        case (uint32)&PCONF:
            break;
        case (uint32)&PCONG:
            break;
        default:
               uart0_puts( "Direccion de puerto erronea\n" );
            break;
    }

}

