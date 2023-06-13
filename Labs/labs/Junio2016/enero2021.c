/*1.(1 punto) En una aplicación se desea tener la posibilidad de mostrar en el display 7 segmentos de
la placa de prototipado S3CEV40 una cuenta de segundos descendente desde un valor dado (9
como máximo). Esta cuenta debe ser autónoma para que no bloquee la ejecución de la aplicación,
por lo que deberá implementarse por interrupción. Para ello, deberá codificarse en C una función
que configurará el RTC para que interrumpa una vez por segundo y que instalará como rutina de
tratamiento otra función (que también deberá codificarse) encargada de realizar el cambio del dígito
mostrado en display. La cuenta comenzará desde el valor que se pase como argumento a la primera
de las funciones mencionadas. Si el argumento es 0, la cuenta deberá cesar devolviendo el valor al
que se ha llegado y que se está mostrando en ese momento. El prototipo de la función será: */

#include <s3c44b0x.h>
#include <common_types.h>
#include <segs.h>
#include <rtc.h>

void isr_rtc( void ) __attribute__ ((interrupt ("IRQ")));

volatile uint8 counter;

uint8 segs_countdown( uint8 initVal )
{
    if( initVal > 0 && initVal <= 9 ) {
        counter = initVal;
        segs_putchar( counter );
        rtc_open( isr_rtc, 127 );
    } else if( initVal == 0 )
        rtc_close();
    return counter;
}

void isr_rtc( void )
{
    segs_putchar( --counter );
    if( !counter )
        rtc_close();
    I_ISPC = BIT_TICK;
}

/*2.(1 punto) Tomando como referencia la función keypad_scan()del BSP desarrollado en los
laboratorios, codificar en C una función que indique todas las teclas que están presionadas
simultáneamente en el keypad de la placa de prototipado S3CEV40. Para ello, devolverá un valor
de 16 bits que valdrá 1 en aquellos bits cuya posición coincida con las teclas presionadas y 0 en el
resto de bits. Por ejemplo, si el teclado tiene las teclas 0, 2, 9 y E presionadas, la llamada a esta
función devolverá 0x4205 (0100001000000101 en binario). El prototipo de la función será: */

#include <s3c44b0x.h>
#include <s3cev40.h>

uint16 keypad_fullscan( void )
{
    uint8 aux;
    uint16 scancode;

    scancode = 0;

    // lee fila 1
    aux = *( KEYPAD_ADDR + 0x1c );    // mascara de scan: 0b00011100
    if( (aux & 0x0f) != 0x0f )        // ¿hay pulsación en fila 1?
    {
        if( (aux & 0x8) == 0 )        // ¿hay pulsación en columna 1?
            scancode |= (1<<0);
        if( (aux & 0x4) == 0 )        // ¿hay pulsación en columna 2?
            scancode |= (1<<1);
        if( (aux & 0x2) == 0 )        // ¿hay pulsación en columna 3?
            scancode |= (1<<2);
        if( (aux & 0x1) == 0 )        // ¿hay pulsación en columna 4?
            scancode |= (1<<3);
    }

    // lee fila 2
    aux = *( KEYPAD_ADDR + 0x1a );    // mascara de scan: 0b00011010
    if( (aux & 0x0f) != 0x0f )        // ¿hay pulsación en fila 2?
    {
        if( (aux & 0x8) == 0 )        // ¿hay pulsación en columna 1?
            scancode |= (1<<4);
        if( (aux & 0x4) == 0 )        // ¿hay pulsación en columna 2?
            scancode |= (1<<5);
        if( (aux & 0x2) == 0 )        // ¿hay pulsación en columna 3?
            scancode |= (1<<6);
        if( (aux & 0x1) == 0 )        // ¿hay pulsación en columna 4?
            scancode |= (1<<7);
    }

    // lee fila 3
    aux = *( KEYPAD_ADDR + 0x16 );    // mascara de scan: 0b00010110
    if( (aux & 0x0f) != 0x0f )        // ¿hay pulsación en fila 3?
    {
        if( (aux & 0x8) == 0 )        // ¿hay pulsación en columna 1?
            scancode |= (1<<8);
        if( (aux & 0x4) == 0 )        // ¿hay pulsación en columna 2?
            scancode |= (1<<9);
        if( (aux & 0x2) == 0 )        // ¿hay pulsación en columna 3?
            scancode |= (1<<10);
        if( (aux & 0x1) == 0 )        // ¿hay pulsación en columna 4?
            scancode |= (1<<11);
    }

    //
    aux = *( KEYPAD_ADDR + 0xe );     // mascara de scan: 0b00001110
    if( (aux & 0x0f) != 0x0f )        // ¿hay pulsación en fila 4?
    {
        if( (aux & 0x8) == 0 )        // ¿hay pulsación en columna 1?
            scancode |= (1<<12);
        if( (aux & 0x4) == 0 )        // ¿hay pulsación en columna 2?
            scancode |= (1<<13);
        if( (aux & 0x2) == 0 )        // ¿hay pulsación en columna 3?
            scancode |= (1<<14);
        if( (aux & 0x1) == 0 )        // ¿hay pulsación en columna 4?
            scancode |= (1<<15);
    }

    return scancode;
}


/*3.. (1 punto) Codificar en C una función que programe la hora y fecha de la alarma del RTC, habilite
y desenmascare las interrupciones por alarma de este dispositivo e instale la RTI encargada de su
tratamiento. La hora y fecha se pasa a través de la estructura de tipo rtc_time_t utilizada en los
laboratorios de la asignatura. El prototipo de la función será: */

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <rtc.h>

void rtc_openalarm( void (*isr)(void), rtc_time_t *rtc_time )
{
    RTCCON |= 1;            // activar R/W de registros

    ALMYEAR = ((rtc_time->year/10) << 4) + rtc_time->year%10;
    ALMMON  = ((rtc_time->mon/10) << 4)  + rtc_time->mon%10;
    ALMDAY  = ((rtc_time->mday/10) << 4) + rtc_time->mday%10;
    ALMHOUR = ((rtc_time->hour/10) << 4) + rtc_time->hour%10;
    ALMMIN  = ((rtc_time->min/10) << 4)  + rtc_time->min%10;
    ALMSEC  = ((rtc_time->sec/10) << 4)  + rtc_time->sec%10;

    RTCCON &= ~(1);            // desactivar R/W de registros

    pISR_RTC = (uint32) isr;                 // Instala la rutina de tratamiento
    I_ISPC   = BIT_RTC;                      // Borra el bit de interrupción pendiente
    INTMSK  &= ~( BIT_GLOBAL | BIT_RTC );    // Desenmascara globalmente las interrupciones y específicamente la interrupción por alarma
    RTCALM   = 0x7f;                         // Habilita la alarma
}

/*4.(1 punto) Codificar en C una función que realice un volcado de memoria por DMA por la UART0
de la placa de prototipado S3CEV40. Para ello deberá utilizarse el controlador de DMA BDMA0
y se tendrá en cuenta que las transferencias a la UART0 deben ser de 8 bits. La función tomará
como argumentos la dirección inicial y final de la región de memoria a volcar. El prototipo de la
función será: */

#include <s3c44b0x.h>
#include <common_types.h>

void uart0_memDump( uint32 addrIni, uint32 addrEnd )
{
    uint16 last_UCON0;

    BDISRC0 = (0 << 30) | (1 << 28) | addrIni;                 // datos de 8b, dirección post-incrementada
    BDIDES0 = (1 << 30) | (3 << 28) | (uint32) &UTXH0;         // M2IO, dirección fija
    BDCON0  = 0;                                               // enable DMA request
    BDICNT0 = (2 << 30) | (1 << 26) | (addrEnd-addrIni+1);     // DMA request: UART0, handshake mode, unit transfer, on-the fly, polling mode, no-autoreload
    BDICNT0 |= (1 << 20);                                      // Enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
    last_UCON0 = UCON0;
    UCON0 = (2 << 2) | (0);                                    // Tx DMA, Rx disable
    while( BDCCNT0 & 0xFFFFF );
    UCON0 = last_UCON0;
}
