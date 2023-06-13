#include <s3c44b0x.h>
#include <l3.h>
#include <leds.h>

#define SHORT_DELAY    { int8 j; for( j=0; j<4; j++ ); }
/* L3CLOCK -> 5
 * L3MODE -> 4
 * L3DATA -> 9
 */


void L3_init( void )
{
    PDATB |= ((1 << 9) | (1 << 5) | (1 << 4));
}

void L3_putByte( uint8 byte, uint8 mode )
{
    uint8 i;
    uint8 rled, lled;
    
    rled = !led_status( RIGHT_LED );
    lled = !led_status( LEFT_LED );    
   
    PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (mode << 4);
    SHORT_DELAY;

    for( i=0; i<8; i++ )
    {
        PDATB = (rled << 10) | (lled << 9) | (0 << 5) | (mode << 4);
        PDATA = (((byte >> i) & 0x1) << 9);
        SHORT_DELAY;    
        PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (mode << 4);;
        SHORT_DELAY;
    }
    PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (1 << 4);   
}
