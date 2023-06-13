/*#include <s3c44b0x.h>
#include <s3cev40.h>
#include <timers.h>
#include <iic.h>

void iic_init( void )
{
    IICADD = 0x0;
    IICCON = 0xAF;
    IICSTAT = 0x10;
}

void iic_start( uint8 mode, uint8 byte )
{
    IICDS   = ...;
    IICSTAT = (mode << 3) | (1 << 5) | (1 << 4);
    IICCON &= (0 << 4);
    while( ... );
}


void iic_putByte( uint8 byte )
{
    IICDS   = ...;
    IICCON &= ...;
    while( ... );    
}

uint8 iic_getByte( uint8 ack )
{
    IICCON  = ...;
    IICCON &= ...;
    while( ... );
    return ...;
}

void iic_stop( uint16 ms )
{
    IICSTAT &= ...;
    IICCON  &= ...;
    sw_delay_ms( ms );
}*/
