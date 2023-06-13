#include <s3c44b0x.h>
#include <s3cev40.h>
#include <rtc.h>

extern void isr_TICK_dummy( void );

void rtc_init( void )
{
    TICNT   = 0x80; //10000000
    RTCALM  = 0x0;
    RTCRST  = 0x0;
        
    RTCCON  = 0x0;
    
    BCDYEAR = 0x13;
    BCDMON  = 0x1;
    BCDDAY  = 0x1;
    BCDDATE = 0x3;
    BCDHOUR = 0x0;
    BCDMIN  = 0x0;
    BCDSEC  = 0x0;

    ALMYEAR = 0x0;
    ALMMON  = 0x0;
    ALMDAY  = 0x0;
    ALMHOUR = 0x0;
    ALMMIN  = 0x0;
    ALMSEC  = 0x0;

    RTCCON &= ~(1 << 0);
}

void rtc_puttime( rtc_time_t *rtc_time )
{
    RTCCON |= (1 << 0);
    
    BCDYEAR = (((rtc_time->year)/10) << 4) | ((rtc_time->year)%10);
    BCDMON  = (((rtc_time->mon)/10) << 4) | ((rtc_time->mon)%10);
    BCDDAY  = (((rtc_time->mday)/10) << 4) | ((rtc_time->mday)%10);
    BCDDATE = (((rtc_time->wday)/10) << 4) | ((rtc_time->wday)%10);
    BCDHOUR = (((rtc_time->hour)/10) << 4) | ((rtc_time->hour)%10);
    BCDMIN  = (((rtc_time->min)/10) << 4) | ((rtc_time->min)%10);
    BCDSEC  = (((rtc_time->sec)/10) << 4) | ((rtc_time->sec)%10);
        
    RTCCON &= ~(1 << 0);
}

void rtc_gettime( rtc_time_t *rtc_time )
{
    RTCCON |= (1 << 0);
    
    rtc_time->year = (BCDYEAR & 0xF) + (BCDYEAR >> 4) *10;
    rtc_time->mon  = (BCDMON & 0xF) + (BCDMON >> 4) *10;
    rtc_time->mday = (BCDDAY & 0xF) + (BCDDAY >> 4) *10;
    rtc_time->wday = (BCDDATE & 0xF) + (BCDDATE >> 4) *10;
    rtc_time->hour = (BCDHOUR & 0xF) + (BCDHOUR >> 4) *10;
    rtc_time->min  = (BCDMIN & 0xF) + (BCDMIN >> 4) *10;
    rtc_time->sec  = (BCDSEC & 0xF) + (BCDSEC >> 4) *10;
    if( ! rtc_time->sec ){
        rtc_time->year = (BCDYEAR & 0xF) + (BCDYEAR >> 4) *10;
        rtc_time->mon  = (BCDMON & 0xF) + (BCDMON >> 4) *10;
        rtc_time->mday = (BCDDATE & 0xF) + (BCDDATE >> 4) *10;
        rtc_time->wday = (BCDDAY & 0xF) + (BCDDAY >> 4) *10;
        rtc_time->hour = (BCDHOUR & 0xF) + (BCDHOUR >> 4) *10;
        rtc_time->min  = (BCDMIN & 0xF) + (BCDMIN >> 4) *10;
        rtc_time->sec  = (BCDSEC & 0xF) + (BCDSEC >> 4) *10;
    };

    RTCCON &= ~(1 << 0);
}


void rtc_open( void (*isr)(void), uint8 tick_count )
{
    pISR_TICK = (uint32) isr;
    I_ISPC    = BIT_TICK;
    INTMSK   &= ~(BIT_GLOBAL | BIT_TICK);
    TICNT     = 0x80 | tick_count;
}

void rtc_close( void )
{
    TICNT     = 0x0;
    INTMSK   |= BIT_TICK;
    pISR_TICK = isr_TICK_dummy;
}
