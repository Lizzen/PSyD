/*
 * enero2022.c
 *
 *  Created on: 18/01/2023
 *      Author: DOSTATIC
 */

/*1.(1 punto) En una arquitectura tipo cola de funciones se desean ejecutar cooperativamente 3 tareas
periódicas Task1(), Task2()y Task3()con periodos respectivos de activación de 10, 20 y 30 ms.
Codifique en C una aplicación que esté formada por 2 hebras: una hebra en foreground encargada
de encolar periódicamente las tareas en los instantes requeridos; y una hebra en background (main)
encargada de inicializar el sistema, instalar la hebra en foreground como RTI por interrupción
periódica del timer0 e indefinidamente ejecutar las tareas que se vayan encolando. Supónganse
implementadas tanto las tareas, como las funciones para la gestión de la cola de funciones usadas
en los laboratorios. */

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <timers.h>

#define TICKS_PER_SEC   (1000)

/* Declaración de fifo de punteros a funciones */

#define BUFFER_LEN   (512)

typedef void (*pf_t)(void);

typedef struct fifo {
    uint16 head;
    uint16 tail;
    uint16 size;
    pf_t buffer[BUFFER_LEN];
} fifo_t;

void fifo_init( void );
void fifo_enqueue( pf_t pf );
pf_t fifo_dequeue( void );
boolean fifo_is_empty( void );
boolean fifo_is_full( void );

/* Declaración de recursos */

volatile fifo_t fifo;

/* Declaración de tareas */

void Task1( void );
void Task2( void );
void Task3( void );

/* Declaración de RTI */

void isr_tick( void ) __attribute__ ((interrupt ("IRQ")));

void main( void )
{
    pf_t pf;

    sys_init();
    timers_init();

    fifo_init();
    timer0_open_tick( isr_tick, TICKS_PER_SEC );

    while( 1 )
    {
        while( !fifo_is_empty() )
        {
            pf = fifo_dequeue();
            (*pf)();
        }
    }
}

void isr_tick( void )
{
    static uint8 cont10ticks = 10;
    static uint8 cont20ticks = 20;
    static uint8 cont30ticks = 30;

    if( !(--cont10ticks) )
    {
        cont10ticks = 10;
        fifo_enqueue( Task1 );
    }
    if( !(--cont20ticks) )
    {
        cont20ticks = 20;
        fifo_enqueue( Task2 );
    }
    if( !(--cont30ticks) )
    {
        cont30ticks = 30;
        fifo_enqueue( Task3 );
    }
    I_ISPC = BIT_TIMER0;
}

/*2.(0.5 puntos) Codificar en C una función que, usando una fuente 8×16, escriba sobre el LCD de la
placa de prototipado S3CEV40 un carácter a partir del pixel (x,y) al revés. Es decir, para que pudiera
leerse correctamente al hacer un giro de 180º en la pantalla. El prototipo de la función será: */

#include <common_types.h>
#include <lcd.h>

extern uint8 font[];

void lcd_putchar_upsidedown( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16; line++ )
        for( row=0; row<8; row++ )
            if( bitmap[line] & (0x80 >> row) )
                lcd_putpixel( x+7-row, y+15-line, color );
            else
                lcd_putpixel( x+7-row, y+15-line, WHITE );
}

/*3.(1 punto) Codificar en C una función que realice o restaure una captura de la pantalla LCD de la
placa de prototipado S3CEV40 utilizando el controlador de DMA ZDMA0. La función tendrá dos
argumentos: el primero indicará si debe realizarse la captura (valor 1) o debe restaurarse (valor 0);
el segundo será puntero al buffer en donde debe almacenarse o está almacenada la captura. El
prototipo de la función será: */

#include <s3c44b0x.h>
#include <common_types.h>
#include <lcd.h>

extern uint8 lcd_buffer[];

void lcd_snapshot( uint8 action, uint8 *capture )
{
    uint32 srcAddr, dstAddr;

    if( action ){
        srcAddr = (uint32) lcd_buffer;
        dstAddr = (uint32) capture;
    } else {
        srcAddr = (uint32) capture;
        dstAddr = (uint32) lcd_buffer;
    }
    ZDISRC0 = (2 << 30) | (1 << 28) | srcAddr;            // datos de 32b, dirección post-incrementada
    ZDIDES0 = (2 << 30) | (1 << 28) | dstAddr;            // recomendada, dirección post-incrementada
    ZDICNT0 = (2 << 28) | (1 << 26) | LCD_BUFFER_SIZE;    // whole service, unit transfer, polling mode, no autoreload
    ZDICNT0 |= (1 << 20);                                 // Enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
    ZDCON0  = 1;                                          // start DMA
}

/*4.(1.5 puntos) En una aplicación se desea tener la posibilidad de generar un zumbido a través del
audio códec de la placa de prototipado S3CEV40. El zumbido se implementará enviando por el
interfaz IIS las muestras correspondientes a una onda cuadrada de máxima amplitud y de frecuencia
dada. Esta generación de muestras debe ser autónoma para que no bloquee la ejecución de la
aplicación, por lo que deberá implementarse por interrupción. Para que sea posible controlar el
envío de muestras, deberá codificarse en C una función que configurará el timer0 para que
interrumpa 16000 veces por segundo (la frecuencia de muestreo del interfaz IIS) e instalará como
rutina de tratamiento otra función (que también deberá codificarse) encargada de enviar en cada
momento la muestra que corresponda para generar la onda cuadrada deseada, es decir, muestras
que alternaran entre MAX_INT16 y -MAX_INT16 cada 16000/(2*freqHz) muestras. Si el
argumento es 0, el zumbido deberá cesar. El prototipo de la función será:  */


#include <s3c44b0x.h>
#include <common_types.h>
#include <iis.h>
#include <timers.h>

#define FS (16000)

void isr_timer0( void ) __attribute__ ((interrupt ("IRQ")));

uint16 counter, toggleWave;

void iss_buzz( uint16 freqHz )
{
    if( freqHz ) {
        counter = 0;
        toggleWave = FS / (2*freqHz);
        timer0_open_tick( isr_timer0, FS );
    }
    else
        timer0_close();
}

void isr_timer0( void )
{
    static int16 sample = MAX_INT16;

    if( ++counter == toggleWave ){
        counter = 0;
        sample = -sample;
    }
    iis_putSample( sample, sample );
    I_ISPC = BIT_TIMER0;
}

