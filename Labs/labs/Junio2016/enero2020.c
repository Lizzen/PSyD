/*1.(1.5 puntos) Codificar en C una aplicaci�n que por interrupci�n retransmita por la UART0 de la
placa de prototipado S3CEV40 los caracteres recibidos por ese mismo interfaz. La aplicaci�n
constar� de 2 hebras que usar�n un mailbox para comunicarse: una hebra en foreground
(uart0_isrRX) encargada de leer el car�cter recibido de la UART0 y comunicarlo a la otra hebra;
y una hebra en background (main) encargada de inicializar el sistema, instalar la hebra en
foreground como RTI por recepci�n de la UART0 e indefinidamente esperar el aviso de la otra
hebra para retransmitir por la UART0 los caracteres recibidos. */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>

void uart0_isrRX( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile char ch;

void main( void )
{
    sys_init();
    uart0_init();
    UFCON0 &= ~1;    // Desactiva las FIFOs

    flag = FALSE;
    pISR_URXD0 = (uint32) uart0_isrRX;
    I_ISPC = BIT_URXD0;
    INTMSK &= ~( BIT_GLOBAL | BIT_URXD0 );
    while( 1 )
        if( flag )
        {
            while( !(UTRSTAT0 & (1 << 1)) );
            UTXH0 = ch;
            flag = FALSE;
        }
}

void uart0_isrRX( void )
{
    ch = URXH0;
    flag = TRUE;
    I_ISPC = BIT_URXD0;
}

/*2.(1 punto) En una aplicaci�n se desea tener la posibilidad de hacer parpadear el led izquierdo de la
placa de prototipado S3CEV40 a una frecuencia dada. Este parpadeo debe ser aut�nomo para que
no bloquee la ejecuci�n de la aplicaci�n, por lo que deber� implementarse por interrupci�n. Para
que sea posible controlar el parpadeo, deber� codificarse en C una funci�n que configurar� el
timer0 para que interrumpa a la frecuencia que corresponda e instalar� como rutina de tratamiento
otra funci�n (que tambi�n deber� codificarse) encargada de conmutar el led izquierdo. Si el
argumento es 0, el parpadeo deber� cesar. El prototipo de la funci�n ser�: */

#include <s3c44b0x.h>
#include <common_types.h>
#include <leds.h>
#include <timers.h>

void isr_timer0( void ) __attribute__ ((interrupt ("IRQ")));

void left_led_blink( uint8 Hz )
{
    if( Hz )
        timer0_open_tick( isr_timer0, 2*Hz );
    else
        timer0_close();
}

void isr_timer0( void )
{
    led_toggle( LEFT_LED );
    I_ISPC = BIT_TIMER0;
}


/*3.(1 punto) Tomando como referencia la funci�n keypad_scan()del BSP desarrollado en los
laboratorios, codificar en C una funci�n que indique el n�mero de teclas que est�n presionadas
simult�neamente en el keypad de la placa de prototipado S3CEV40. El prototipo de la funci�n ser�: */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <keypad.h>

uint8 keypad_keycount( void )
{
    uint8 aux;
    uint8 count = 0;

    /* lee fila 1 */
    aux = *( KEYPAD_ADDR + 0x1c );    // mascara de scan: 0b00011100
    if( (aux & 0x0f) != 0x0f )        // �hay pulsaci�n en fila 1?
    {
        if( (aux & 0x8) == 0 )        // �hay pulsaci�n en columna 1?
            count++;
        if( (aux & 0x4) == 0 )        // �hay pulsaci�n en columna 2?
            count++;
        if( (aux & 0x2) == 0 )        // �hay pulsaci�n en columna 3?
            count++;
        if( (aux & 0x1) == 0 )        // �hay pulsaci�n en columna 4?
            count++;
    }

    /* lee fila 2 */
    aux = *( KEYPAD_ADDR + 0x1a );    // mascara de scan: 0b00011010
    if( (aux & 0x0f) != 0x0f )        // �hay pulsaci�n en fila 2?
    {
        if( (aux & 0x8) == 0 )        // �hay pulsaci�n en columna 1?
            count++;
        if( (aux & 0x4) == 0 )        // �hay pulsaci�n en columna 2?
            count++;
        if( (aux & 0x2) == 0 )        // �hay pulsaci�n en columna 3?
            count++;
        if( (aux & 0x1) == 0 )        // �hay pulsaci�n en columna 4?
            count++;
    }

    /* lee fila 3 */
    aux = *( KEYPAD_ADDR + 0x16 );     // mascara de scan: 0b00010110
    if( (aux & 0x0f) != 0x0f )        // �hay pulsaci�n en fila 3?
    {
        if( (aux & 0x8) == 0 )        // �hay pulsaci�n en columna 1?
            count++;
        if( (aux & 0x4) == 0 )        // �hay pulsaci�n en columna 2?
            count++;
        if( (aux & 0x2) == 0 )        // �hay pulsaci�n en columna 3?
            count++;
        if( (aux & 0x1) == 0 )        // �hay pulsaci�n en columna 4?
            count++;
    }

    /* lee fila 4 */
    aux = *( KEYPAD_ADDR + 0xe );    // mascara de scan: 0b00001110
    if( (aux & 0x0f) != 0x0f )        // �hay pulsaci�n en fila 4?
    {
        if( (aux & 0x8) == 0 )        // �hay pulsaci�n en columna 1?
            count++;
        if( (aux & 0x4) == 0 )        // �hay pulsaci�n en columna 2?
            count++;
        if( (aux & 0x2) == 0 )        // �hay pulsaci�n en columna 3?
            count++;
        if( (aux & 0x1) == 0 )        // �hay pulsaci�n en columna 4?
            count++;
    }

    return count;
}

/*4.(0.5 punto) Codificar en C una funci�n que dibuje sobre el LCD de la placa de prototipado
S3CEV40 un rect�ngulo relleno del color indicado cuya esquina superior izquierda est� en el pixel
(xleft,yup) y cuya esquina inferior derecha est� en el p�xel (xright, ydown). El prototipo de la
funci�n ser�: */

#include <common_types.h>
#include <lcd.h>

void lcd_fill_box( uint16 xleft, uint16 yup, uint16 xright, uint16 ydown, uint8 color )
{
    uint16 yAux;

    for( yAux=yup; yAux<=ydown; yAux++ )
        lcd_draw_hline( xleft, xright, yAux, color, 1 );

}

