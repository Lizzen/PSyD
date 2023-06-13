/*1.(1.5 puntos) Codificar en C una aplicaci�n que por encuesta peri�dica retransmita por la UART0
de la placa de prototipado S3CEV40 los caracteres recibidos por ese mismo interfaz. El
microcontrolador S3C44B0X deber� configurarse para que su timer-0 interrumpa cada 20 ms. La
aplicaci�n constar� de 2 hebras que usar�n un mailbox para comunicarse: una hebra en foreground
(isr_timer0) encargada de leer el estado de la UART0 y, en caso de detectar la recepci�n de un
car�cter, lo lea de la UART0 y lo comunique a la otra hebra; y una hebra en background (main)
encargada de inicializar el sistema, instalar la hebra en foreground como RTI del timer-0 e
indefinidamente esperar el aviso de la otra hebra para retransmitir por la UART0 los caracteres
recibidos. T�ngase en cuenta que, para el correcto funcionamiento de la aplicaci�n, ninguna de las
hebras debe llamar a la funci�n uart0_getchar. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <timers.h>
#include <uart.h>

void timer0_isr( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile char ch;

void main(void){
    sys_init();
    uart0_init();
    timers_init();

    flag = FALSE;
    timer0_open_ms( timer0_isr, 20, TIMER_INTERVAL ); 	//interrumpe cada 20ms

    while( 1 )
              if( flag )
              {
            	  uart0_putchar(ch);
                  flag = FALSE;
              }
}

void timer0_isr( void )
{
	 if( UFSTAT0 & 0xF )
	    {
	        ch = URXH0;
	        flag = TRUE;
	    }

    I_ISPC = BIT_TIMER0;
}

/*2.(1 punto) Codificar en C una funci�n que espere la presi�n y depresi�n de una tecla del keypad de
la placa de prototipado S3CEV40 mientras sea falso el valor del flag booleano cuya direcci�n se
pasa como argumento. Si la funci�n finaliza por presi�n y depresi�n de una tecla devolver� su
scancode, si lo hace porque el flag vale cierto devolver� 0xff. Esta funci�n permite que la lectura
del keypad por encuesta continua no bloquee la respuesta del programa invocante al evento externo
se�alizado por el flag. El prototipo de la funci�n ser�: */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <keypad.h>

uint8 keypad_getchar_flag( volatile boolean *flag )
{
    uint8 scancode;

    while( (PDATG & 0x2) && !*flag );
    if( *flag ) return 0xff;
    sw_delay_ms( KEYPAD_KEYDOWN_DELAY  );

    scancode = keypad_scan();

    while( !(PDATG & 0x2) && !*flag );
    if( *flag ) return 0xff;
    sw_delay_ms( KEYPAD_KEYUP_DELAY );

    return scancode;
}

/*3.(0.75 puntos) Codificar en C una funci�n que permita reproducir por encuesta (polling), a trav�s
del controlador de bus IIS del microcontrolador S3C44B0X, el sonido est�reo almacenado en un
buffer de tama�o en bytes dado a partir de un instante determinado (indicado en d�cimas de
segundo). El audio est� almacenado en memoria en el formato habitual: alternando las muestras
del canal izquierdo y del derecho a raz�n de 16 bits/muestra y 16000 muestras/segundo. El
prototipo de la funci�n ser�:  */

#include <common_types.h>
#include <iis.h>

void iis_playFrom( int16 *buffer, uint32 length, uint32 decsegs )
{
    uint32 i;
    int16 ch1, ch2;

    for( i=decsegs*1600*2; i<length/2; )
    {
        ch1 = buffer[i++];
        ch2 = buffer[i++];
        iis_putSample( ch1, ch2 );
    }
}

/*
void iis_playFrom( int16 *buffer, uint32 length, uint32 decsegs )
{
    iis_play( buffer+decsegs*1600*2, length, OFF );
}
*/

/*4.(0.75 puntos) Es posible reducir la frecuencia efectiva de muestreo de un sistema de audio, sin
reducir su frecuencia nominal, haciendo un cribado de las muestras transmitidas. El cribado m�s
elemental consiste en tomar, en cada canal, la media aritm�tica de una serie de muestras
consecutivas. Se pide codificar en C una funci�n para realizar por encuesta (pooling) el cribado de
1 de cada 2 muestras recibidas a trav�s del controlador de bus IIS del microcontrolador S3C44B0X.
El prototipo de la misma ser�: */

#include <common_types.h>
#include <iis.h>

inline void iis_getSampleHalfRate( int16 *ch0, int16 *ch1 )
{
    while( (IISFCON & 0x0F) < 2 );
    *ch0 = IISFIF;
    *ch1 = IISFIF;
    while( (IISFCON & 0x0F) < 2 );
    *ch0 = (IISFIF + *ch0) >> 1;
    *ch1 = (IISFIF + *ch1) >> 1;

}

/*
inline void iis_getSampleHalfRate( int16 *ch0, int16 *ch1 )
{
    int16 ch0aux, ch1aux;

    iis_getSample( &ch0aux, &ch1aux );
    iis_getSample( ch0, ch1 );
    *ch0 = ( ch0aux + *ch0 ) >> 1;
    *ch1 = ( ch1aux + *ch1 ) >> 1;
}
 */




