/*
 * Julio2018.c
 *
 *  Created on: 18/01/2023
 *      Author: DOSTATIC
 */

/*1.(1.5 puntos) Codificar en C una aplicaci�n que por encuesta peri�dica transmita por la UART0 de
la placa de prototipado S3CEV40 el c�digo ASCII del d�gito hexadecimal asociado a cada tecla
que se pulse en el del keypad. El microcontrolador S3C44B0X deber� configurarse para que su
timer-0 interrumpa cada 20 ms. La aplicaci�n constar� de 2 hebras que usar�n un mailbox para
comunicarse: una hebra en background (main) encargada de inicializar el sistema, instalar la hebra
en foreground como RTI del timer-0 y comunicar, cada vez que lea una nueva tecla del keypad, el
correspondiente c�digo a la otra hebra; y una hebra en foreground (isr_timer0) que en caso de
que la UART0 est� lista para transmitir y haya disponible un nuevo c�digo, lo env�e por la UART0.
T�ngase en cuenta que, para el correcto funcionamiento de la aplicaci�n, ninguna de las hebras
debe llamar a la funci�n uart0_putchar. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>
#include <timers.h>
#include <keypad.h>

void timer0_isr( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile char ch;

void main( void )
{
    sys_init();
    uart0_init();
    timers_init();
    keypad_init();

    flag = FALSE;
    timer0_open_ms( timer0_isr, 20, TIMER_INTERVAL );
    while( 1 )
    {
        ch = keypad_getchar();
        ch += ( ch < 10 ? '0' : 'a'-10 );
        flag = TRUE;
    }
}

void timer0_isr( void )
{
    if( flag && !(UFSTAT0 & (1 << 9)) )
    {
        UTXH0 = ch;
        flag = FALSE;
    }

    I_ISPC = BIT_TIMER0;
}

/*2.(1.5 punto) Codificar en C una funci�n que sume dos n�meros reales, a y b, representados en punto
fijo en formatos Q(16-an).an y Q(16-bn).bn respectivamente (es decir, representados ambos con
16 bits de los cuales en a, an bits son decimales y en b, lo son bn bits). El resultado deber� estar
representado en el mismo formato que el argumento con mayor n�mero de bits decimales y deber�
saturarse cuando sea mayor que el m�ximo n�mero representable en dicha notaci�n. El prototipo
de la funci�n ser�: */

#include <common_types.h>
#include <fix_types.h>

fix16 fix_add( fix16 a, uint8 an, fix16 b, uint8 bn )
{
    uint8 rm;
    fix32 a32, b32, r32;

    if( an >= bn ) {
        rm = an;
        a32 = a;
        b32 = b << (an-bn);
    }
    else {
        rm = bn;
        a32 = a << (bn-an);
        b32 = b;
    }

    r32 = a32 + b32;

    if( r32 > MAX_INT16 )
        return MAX_INT16;
    else if ( r32 < MIN_INT16 )
        return MIN_INT16;
    else
        return r32;

}

/*3.(0.5 punto) El bus IIS transmite audio en est�reo alternando en serie las muestras del canal
izquierdo y del derecho. Si se desea recibir audio en mono, las muestras transmitidas por uno de
los canales ser desechadas. Si se desea enviar audio en mono, las mismas muestras deben
transmitirse por ambos canales. Se pide codificar en C dos funciones para transmitir audio en mono
por pooling a trav�s del controlador de bus IIS del microcontrolador S3C44B0X. Los prototipos
de las funciones ser�n: */

#include <s3c44b0x.h>
#include <common_types.h>
#include <iis.h>

inline void iis_putSampleMono( int16 sample )
{
    while( ((IISFCON & 0xF0) >> 4) > 6 );
    IISFIF = sample;
    IISFIF = sample;
}


inline int16 iis_getSampleMono( void )
{
    int16 trash;

    while( (IISFCON & 0x0F) < 2 );
    trash = IISFIF;
    return IISFIF;
}

/*

inline void iis_putSampleMono( int16 sample )
{
    iis_putSample( sample, sample );
}

inline int16 iis_getSampleMono( void )
{
    int16 trash, sample;

    iis_getSample( &trash, &sample );
    return sample;
}

*/

/*4.(0.5 puntos) Codificar en C una funci�n que, usando una fuente 8�16, escriba sobre el LCD de la
placa de prototipado S3CEV40 un car�cter a partir del pixel (x,y). Dado el mapa de bits del car�cter,
los foreground pixels deber�n mostrarse en el color indicado (sobreescribiendo los valores de los
correspondientes p�xeles en pantalla), pero los background pixels deber�n ser considerados
transparentes, de modo que permanezcan inalterados los valores de los correspondientes p�xeles en
pantalla. */

#include <common_types.h>
#include <lcd.h>

extern uint8 font[];

void lcd_putchar_transparent( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16; line++ )
        for( row=0; row<8; row++ )
            if( bitmap[line] & (0x80 >> row) )
                lcd_putpixel( x+row, y+line, color );
}
