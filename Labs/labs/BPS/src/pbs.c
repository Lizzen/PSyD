#include <s3c44b0x.h>
#include <s3cev40.h>
#include <pbs.h>
#include <timers.h>

extern void isr_PB_dummy( void );

void pbs_init( void )
{
    timers_init();
}

uint8 pb_scan( void )
{
    if((PB_LEFT & PDATG) == PB_UP)
        return PB_LEFT;
    else if((PB_RIGHT & PDATG) == PB_UP)
        return PB_RIGHT;
    else
        return PB_FAILURE;
}

uint8 pb_pressed( void )
{
	if (((PDATG & PB_LEFT) == PB_UP) || ((PDATG & PB_RIGHT) == PB_UP)){
		return '1';
	}

	return 0;
}

uint8 pb_getchar( void )
{
	uint8 scancode;

	while(!pb_pressed());

	sw_delay_ms(PB_KEYDOWN_DELAY);
	scancode = pb_scan();

	while((PDATG & scancode) == 0){
		sw_delay_ms( PB_KEYUP_DELAY );
	}

	return scancode;
}

uint8 pb_getchartime( uint16 *ms )
{
    uint8 scancode;
    
    //while((PDATG & (1 << 6)) && (PDATG & (1 << 7)));
    while(!pb_pressed());
    timer3_start();
    sw_delay_ms( PB_KEYDOWN_DELAY );
    
    scancode = pb_scan();
    
    while((PDATG & scancode) == PB_UP);
    *ms = timer3_stop() / 10;
    sw_delay_ms( PB_KEYUP_DELAY );

    return scancode;
}

uint8 pb_timeout_getchar( uint16 ms )
{
    uint8 scancode;

    timer3_start_timeout(10*ms);

    while(!timer3_timeout() && !pb_pressed());
    if (timer3_timeout()){
    	return PB_TIMEOUT;
    }
    else {
    	sw_delay_ms( PB_KEYDOWN_DELAY );

    	scancode = pb_scan();
    	while(timer3_timeout() && ((PDATG & scancode) == PB_UP));
    	    sw_delay_ms( PB_KEYUP_DELAY );

    	return scancode;
    }
}

void pbs_open( void (*isr)(void) )
{
    pISR_PB   = (uint32) isr;
    EXTINTPND = 0xF;
    I_ISPC    = BIT_EINT4567;
    INTMSK   &= ~(BIT_GLOBAL | BIT_EINT4567);
}

void pbs_close( void )
{
    INTMSK  |= BIT_EINT4567;
    pISR_PB  = (uint32) isr_PB_dummy;
}
