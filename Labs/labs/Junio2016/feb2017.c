/*1.(1.5 puntos) Codificar en C una aplicaci�n que env�e por la UART-0 de la placa de prototipado
S3CEV40 las teclas pulsadas en su keypad. El microcontrolador S3C44B0X deber� configurarse
para que su timer-0 interrumpa cada 20 ms. La aplicaci�n constar� de 2 hebras que usar�n un
mailbox para comunicarse: una hebra en foreground (isr_timer0) encargada de escanear el
keypad y de enviar a la otra hebra el resultado del escaneo siempre que este sea v�lido y haya
cambiado desde su �ltima lectura; y una hebra en background (main) encargada de inicializar el
sistema, instalar la hebra en foreground como RTI del timer-0 e indefinidamente esperar la
recepci�n de mensajes para visualizarlos. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <timers.h>
#include <keypad.h>
#include <uart.h>

void timer0_isr( void ) __attribute__ ((interrupt ("IRQ")));

volatile boolean flag;
volatile uint8 scanCode;

void main(void){
    sys_init();
    uart0_init();
    timers_init();
    keypad_init();

    flag = FALSE;
    timer0_open_ms( timer0_isr, 20, TIMER_INTERVAL ); 	//interrumpe cada 20ms

    while( 1 )
           if( flag )
           {
               uart0_puts( "\nScancode: 0x" );
               uart0_puthex( scanCode );
               flag = FALSE;
           }
}

void timer0_isr( void )
{
    static uint8 lastScanCode = 255;

    scanCode = keypad_scan();
    if( scanCode != lastScanCode && scanCode != KEYPAD_FAILURE )
    {
        lastScanCode = scanCode;
        flag = TRUE;
    }

    I_ISPC = BIT_TIMER0;
}

/*2.(1 punto) Codificar en C una funci�n que convierta un n�mero real representado en punto fijo
Q(16-n).n (es decir, representado con 16 bits de los cuales n son decimales) al mismo n�mero
representado en formato Q(16-m).m (es decir, representado con 16 bits de los cuales m son
decimales). La funci�n deber� saturar el resultado cuando el n�mero representado en la notaci�n
origen exceda el mayor n�mero representable de la notaci�n destino. El prototipo de la funci�n
ser�: */

#include <common_types.h>
#include <fix_types.h>

fix16 fix_convert( fix16 number, uint8 n, uint8 m )
{
    fix32 aux;

    if( n >= m )
        return number >> (n-m);
    else {
        aux = number << (m-n);
        if( aux > MAX_INT16 )
            return MAX_INT16;
        else if ( aux < MIN_INT16 )
            return MIN_INT16;
    }
}

/*3.(1 punto) Codificar en C una funci�n que, usando una fuente 8�16, escriba sobre el LCD de la
placa de prototipado S3CEV40 un car�cter a partir del pixel (x,y) con el color y el grado de
aumento indicado. El prototipo de la funci�n ser�:

Como orientaci�n, t�ngase en cuenta que la llamada a esta funci�n con size valiendo 1 ser�
equivalente a la llamada a la funci�n lcd_putchar, as� como la llamada a esta funci�n con size
valiendo 2 ser� equivalente a la llamada a la funci�n lcd_putchar_x2.
 */

#include <common_types.h>
#include <lcd.h>

extern uint8 font[];

void lcd_putchar_xsize( uint16 x, uint16 y, uint8 color, char ch, uint8 size )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16*size; line++)
        for( row=0; row<8*size; row++)
            if( bitmap[line/size] & (0x80 >> row/size) )
                lcd_putpixel( x+row, y+line, color );
            else
                lcd_putpixel( x+row, y+line, WHITE );
}

/*4.(0.5 puntos) Suponiendo que la frecuencia del oscilador externo conectado a la entrada de reloj
del microcontrolador S3C44B0X es 8 MHz, codificar en C una funci�n que configure el gestor
de reloj del sistema para que en modo normal genere la frecuencia de reloj indicada como
argumento (expresada en unidades de MHz). Para hacer la configuraci�n, la funci�n deber�
chequear que la frecuencia solicitada est� dentro del rango admitido por el controlador del PLL.
El prototipo de la funci�n ser�:
void set_mclk( uint8 numMHz ); */

#include <s3c44b0x.h>
#include <common_types.h>

void set_mclk( uint8 numMHz )
{
    if( numMHz >= 20 && numMHz <= 66 )
        PLLCON = ((numMHz-8) << 12) | (2 << 4) | (1);
}
