#include <s3c44b0x.h>
#include <s3cev40.h>
#include <timers.h>
#include <adc.h>
#include <lcd.h>
#include <ts.h>

#define PX_ERROR    (5)

static uint16 Vxmin = 0;
static uint16 Vxmax = 0;
static uint16 Vymin = 0;
static uint16 Vymax = 0;

static uint8 state;

extern void isr_TS_dummy( void );

static void ts_scan( uint16 *Vx, uint16 *Vy );
static void ts_calibrate( void );
static void ts_sample2coord( uint16 Vx, uint16 Vy, uint16 *x, uint16 *y );

void ts_init( void )
{
    timers_init();  
    lcd_init();
    adc_init();
    PDATE |= ((1<<7) | (1<<5) | (1<<4));   // (Y-, X-, Y+, X+) = (GND, Z, Z, Z)
    PDATE &= ~(1 << 6);
    sw_delay_ms( 1 );
    ts_on();
    ts_calibrate();
    ts_off();
}

void ts_on( void )
{
    adc_on();
    state = TS_ON;
}

void ts_off( void )
{
	adc_off();
	state = TS_OFF;
}

uint8 ts_status( void )
{
    return state;
}

uint8 ts_pressed( void )
{
    if ((PDATG & (1 << 2)) == 0){ // True si el valor del puerto 2 de G es 0
    	return '1';
    }

    return 0;
}

static void ts_calibrate( void )
{
    uint16 x, y;
    
    lcd_on();
    do {    
        lcd_clear();
        lcd_puts(16, 20, BLACK, "Calibrando LCD: ");
    	lcd_putpixel(0,0,BLACK);
    	lcd_putpixel(0,1,BLACK);
    	lcd_putpixel(0,2,BLACK);
    	lcd_putpixel(0,3,BLACK);
    	lcd_putpixel(0,4,BLACK);
    	lcd_putpixel(1,0,BLACK);
    	lcd_putpixel(1,1,BLACK);
    	lcd_putpixel(1,2,BLACK);
    	lcd_putpixel(1,3,BLACK);
    	lcd_putpixel(1,4,BLACK);
    	lcd_putpixel(2,0,BLACK);
    	lcd_putpixel(2,1,BLACK);
    	lcd_putpixel(2,2,BLACK);
    	lcd_putpixel(2,3,BLACK);
    	lcd_putpixel(2,4,BLACK);
    	lcd_putpixel(3,0,BLACK);
    	lcd_putpixel(3,1,BLACK);
    	lcd_putpixel(3,2,BLACK);
    	lcd_putpixel(3,3,BLACK);
    	lcd_putpixel(3,4,BLACK);
    	lcd_putpixel(4,0,BLACK);
    	lcd_putpixel(4,1,BLACK);
    	lcd_putpixel(4,2,BLACK);
    	lcd_putpixel(4,3,BLACK);
    	lcd_putpixel(4,4,BLACK);

        while(!ts_pressed());
        sw_delay_ms( TS_DOWN_DELAY );
        ts_scan( &Vxmin, &Vymax );
        while(ts_pressed());
        sw_delay_ms( TS_UP_DELAY );

        lcd_putpixel(319,239, BLACK);
        lcd_putpixel(318,239, BLACK);
        lcd_putpixel(317,239, BLACK);
        lcd_putpixel(316,239, BLACK);
        lcd_putpixel(315,239, BLACK);
        lcd_putpixel(319,238, BLACK);
        lcd_putpixel(318,238, BLACK);
        lcd_putpixel(317,238, BLACK);
        lcd_putpixel(316,238, BLACK);
        lcd_putpixel(315,238, BLACK);
        lcd_putpixel(319,237, BLACK);
        lcd_putpixel(318,237, BLACK);
        lcd_putpixel(317,237, BLACK);
        lcd_putpixel(316,237, BLACK);
        lcd_putpixel(315,237, BLACK);
        lcd_putpixel(319,236, BLACK);
        lcd_putpixel(318,236, BLACK);
        lcd_putpixel(317,236, BLACK);
        lcd_putpixel(316,236, BLACK);
        lcd_putpixel(315,236, BLACK);
        lcd_putpixel(319,235, BLACK);
        lcd_putpixel(318,235, BLACK);
        lcd_putpixel(317,235, BLACK);
        lcd_putpixel(316,235, BLACK);
        lcd_putpixel(315,235, BLACK);
           
        while(!ts_pressed());
        sw_delay_ms( TS_DOWN_DELAY );
        ts_scan( &Vxmax, &Vymin );
        while(ts_pressed());
        sw_delay_ms( TS_UP_DELAY );
    
        lcd_putpixel(160,120, BLACK);
        lcd_putpixel(159,120, BLACK);
        lcd_putpixel(158,120, BLACK);
        lcd_putpixel(161,120,BLACK);
        lcd_putpixel(162,120, BLACK);
        lcd_putpixel(160,121, BLACK);
        lcd_putpixel(159,121, BLACK);
        lcd_putpixel(158,121, BLACK);
        lcd_putpixel(161,121,BLACK);
        lcd_putpixel(162,121, BLACK);
        lcd_putpixel(160,122, BLACK);
        lcd_putpixel(159,122, BLACK);
        lcd_putpixel(158,122, BLACK);
        lcd_putpixel(161,122,BLACK);
        lcd_putpixel(162,122, BLACK);
        lcd_putpixel(160,119, BLACK);
        lcd_putpixel(159,119, BLACK);
        lcd_putpixel(158,119, BLACK);
        lcd_putpixel(161,119,BLACK);
        lcd_putpixel(162,119, BLACK);
        lcd_putpixel(160,118, BLACK);
        lcd_putpixel(159,118, BLACK);
        lcd_putpixel(158,118, BLACK);
        lcd_putpixel(161,118,BLACK);
        lcd_putpixel(162,118, BLACK);

        ts_getpos( &x, &y );      
    
    } while( (x > LCD_WIDTH/2+PX_ERROR) || (x < LCD_WIDTH/2-PX_ERROR) || (y > LCD_HEIGHT/2+PX_ERROR) || (y < LCD_HEIGHT/2-PX_ERROR) );
    lcd_clear();
}

void ts_getpos( uint16 *x, uint16 *y )
{
	uint16 Vx = 0;
	uint16 Vy = 0;

    while(!ts_pressed());
    sw_delay_ms( TS_DOWN_DELAY );
    ts_scan( &Vx, &Vy );
    while(ts_pressed());
    sw_delay_ms( TS_UP_DELAY );

    ts_sample2coord(Vx, Vy, x, y);
}

void ts_getpostime( uint16 *x, uint16 *y, uint16 *ms )
{
	uint16 Vx = 0;
	uint16 Vy = 0;

    while(!ts_pressed());
    timer3_start();
    sw_delay_ms( TS_DOWN_DELAY );
    ts_scan( &Vx, &Vy );

    while(ts_pressed());
    *ms = timer3_stop() /10;
    sw_delay_ms( TS_UP_DELAY );

    ts_sample2coord(Vx, Vy, x, y);
}

uint8 ts_timeout_getpos( uint16 *x, uint16 *y, uint16 ms )
{
	uint16 Vx = 0;
	uint16 Vy = 0;

	timer3_start_timeout(10*ms);
	while (!timer3_timeout() && !ts_pressed());
	if (timer3_timeout()){
		return TS_TIMEOUT;
	}
	else{
		sw_delay_ms( TS_DOWN_DELAY );
		ts_scan( &Vx, &Vy );

		while(timer3_timeout() && ts_pressed());
		sw_delay_ms( TS_UP_DELAY );
		ts_sample2coord(Vx, Vy, x, y);

		return TS_OK; // No deja devolver ts_sample2coord(es funcion void) asi que pense que debe devolver o TS_TIMEOUT o TS_OK como si fuera un booleano
	}
}

static void ts_scan( uint16 *Vx, uint16 *Vy )
{
	PDATE |= ((1<<6) | (1<<5));     // (Y-, X-, Y+, X+) = (Z, GND, Z, Vdd )
	PDATE &= ~((1 << 7) | (1 << 4));
    *Vx = adc_getSample(ADC_AIN1);
    
    PDATE |= ((1<<7) | (1<<4));     // (Y-, X-, Y+, X+) = (GND, Z, Vdd, Z)
    PDATE &= ~((1 << 6) | (1 << 5));
    *Vy = adc_getSample(ADC_AIN0);
    
    PDATE |= (1<<7) | (1<<5) | (1<<4);   // (Y-, X-, Y+, X+) = (GND, Z, Z, Z)
    PDATE &= ~(1 << 6);
    sw_delay_ms( 1 );
}

static void ts_sample2coord( uint16 Vx, uint16 Vy, uint16 *x, uint16 *y )
{
    if( Vx < Vxmin )
        *x = 0;
    else if( Vx > Vxmax )
        *x = LCD_WIDTH-1;
    else 
        *x = (LCD_WIDTH*(Vx-Vxmin)) / (Vxmax-Vxmin);

    if( Vy < Vymin )
        *y = LCD_HEIGHT-1;
    else if( Vy > Vymax )
        *y = 0;
    else
        *y = (LCD_HEIGHT -1) - (LCD_HEIGHT*(Vy-Vymin) / (Vymax-Vymin));
}

void ts_open( void (*isr)(void) )
{
	pISR_TS = (uint32)isr;
	I_ISPC = BIT_EINT4567;
	INTMSK   &= ~(BIT_GLOBAL | BIT_EINT4567);
}

void ts_close( void )
{
	INTMSK   |= BIT_EINT4567;
	pISR_ADC = (uint32)isr_TS_dummy;
}
