#include <s3c44b0x.h>
#include <uart.h>

void uart0_init( void )
{
    UFCON0 = 0x1;
    UMCON0 = 0x0;
    ULCON0 = 0x3;
    UBRDIV0 = 0x22;
    UCON0 = 0x5;
}

void uart0_putchar( char ch )
{
    while((UFSTAT0 & (1 << 9)));
    UTXH0 = ch;
}        

char uart0_getchar( void )
{
    while(!(UFSTAT0 & 0xf));
    return URXH0;
}

void uart0_puts( char *s )
{
    while (*s != '\0'){
    	uart0_putchar(*s);
    	*++s;
    }
}

void uart0_putint( int32 i )
{
    char buf[16 + 1];
    char *p = buf + 16;
    uint8 c;
    boolean negativo = FALSE;

    *p = '\0';

    if (i < 0){
        negativo = TRUE;
        i *= (-1);
    }

    do {
        c = i % 10;
        *--p = '0' + c;
        i /= 10;
    } while( i );

    if (negativo){ *--p = '-';}

    uart0_puts( p );
}

void uart0_puthex( uint32 i )
{
    char buf[8 + 1];
    char *p = buf + 8;
    uint8 c;

    *p = '\0';

    do {
        c = i & 0xf;
        if( c < 10 )
            *--p = '0' + c;
        else
            *--p = 'a' + c - 10;
        i = i >> 4;
    } while( i );

    uart0_puts( p );
}

void uart0_gets( char *s )
{
	char aux;
	aux = uart0_getchar();

    while (aux != '\n'){
    	*s = aux;
    	*++s;
    	aux = uart0_getchar();
    }
    *s  = '\0';
}

int32 uart0_getint( void )
{
	int32 entero = 0;
	char cadena[256], *p;
    uart0_gets(cadena);
    boolean negativo = FALSE;

    p = cadena;

    if (*p == '-'){
    	negativo = TRUE;
    	++p;
    }

    while (*p != '\0'){
    	entero = *p++ - '0' + entero * 10;
    }

    if (negativo){entero *= (-1);}

    return entero;
}

uint32 uart0_gethex( void )
{
	uint32 entero = 0;
	char cadena[256], *p;
    uart0_gets(cadena);

    p = cadena;

    while (*p != '\0'){
    	if (*p >= '0' && *p <= '9'){
    		entero = *p++ - '0' + entero * 16;
    	}
    	else if (*p >= 'A' && *p <= 'F'){
    		entero = *p++ - 'A' + entero * 16 + 10;
    	}
    	else if (*p >= 'a' && *p <= 'f'){
    		entero = *p++ - 'a' + entero * 16 + 10;
    	}
    }

    return entero;
}
