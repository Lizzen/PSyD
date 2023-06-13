/*cd C:\Users\DOSTATIC\Documents\PSyD-proyecto(2022-23)\bmp320x240
source load_bmp.txt*/
#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <segs.h>
#include <timers.h>
#include <lcd.h>
#include <keypad.h>

/* Número máximo de fotos distintas visualizables en el marco */

#define MAX_PHOTOS (40)

/* Direcciones en donde se encuentran cargados los BMPs tras la ejecución del comando "script load_bmp.script" */

#define ARBOL       ((uint8 *)0x0c210000)
#define PICACHU     ((uint8 *)0x0c220000)
#define MINIARBOL   ((uint8 *)0x0c280000)
#define MINIPICACHU ((uint8 *)0x0c288000)

/* Dimensiones de la pantalla para la realización de efectos */

#define LCD_COLS   (LCD_WIDTH/2)        // Para simplificar el procesamiento consideraremos una columna como la formada por 2 píxeles adyacentes (1 byte)
#define LCD_ROWS   (LCD_HEIGHT)

/* Sentidos de realización del efecto */

#define LEFT       (0)
#define RIGHT      (1)
#define UP         (2)
#define DOWN       (3)
#define NO_APPLY   (4)
#define HORIZONTAL (5)
#define VERTICAL   (6)

/* Declaración de funciones auxiliares */

void lcd_putBmp( uint8 *bmp, uint16 x, uint16 y, uint16 xsize, uint16 ysize );
void lcd_bmp2photo( uint8 *bmp, uint8 *photo );

void lcd_putColumn( uint16 xLcd, uint8 *photo, uint16 xPhoto );
void lcd_putRow( uint16 yLcd, uint8 *photo, uint16 yPhoto );    							// Deberá recodificarse en el proyecto final [Hecho]
void lcd_putPhoto( uint8 *photo );                              							// Deberá recodificarse en el proyecto final [Hecho]

void lcd_shift(uint8 sense, uint32 origen, uint32 destino, uint16 size);                    // Deberá completarse y ampliarse en el proyecto final [Hecho]

/*Declaración de funciones añadidas por mi*/
void lcd_putPixelColumn( uint16 xLcd, uint8 *photo, uint16 xPhoto, uint16 init, uint16 fin);

/* Declaración de efectos de transición entre fotos */

void efectoNulo( uint8 *photo, uint8 sense );
void efectoEmpuje( uint8 *photo, uint8 sense );                 // Deberá completarse en el proyecto final	 [Hecho]
void efectoBarrido( uint8 *photo, uint8 sense );                // Deberá implementarse en el proyecto final [Hecho]
void efectoRevelado( uint8 *photo, uint8 sense );               // Deberá implementarse en el proyecto final [Hecho]
void efectoCobertura( uint8 *photo, uint8 sense );              // Deberá implementarse en el proyecto final [Hecho]

// Otros posibles efectos a implementar

void efectoDivisionEntrante( uint8 *photo, uint8 sense );		// [Hecho]
void efectoDivisionSaliente( uint8 *photo, uint8 sense );		// [Hecho]
void efectoCuadradoEntrante( uint8 *photo, uint8 sense );		// [Sin terminar]
void efectoCuadradoSaliente( uint8 *photo, uint8 sense );		// [Sin terminar]
void efectoBarras( uint8 *photo, uint8 sense );					// [Hecho]
void efectoPeine( uint8 *photo, uint8 sense );					// [Sin terminar]
void efectoDisolver( uint8 *photo, uint8 sense );
void efectoFlash( uint8 *photo, uint8 sense );					// [Hecho]
void efectoAleatorio( uint8 *photo, uint8 sense );

// Efectos del power point
void efectoDesvanecer( uint8 *photo, uint8 sense );				// [Hecho]

/* Declaración de tipos */

typedef void (*pf_t)( uint8 *, uint8 );    // Tipo puntero a una función efecto con 2 argumentos (foto y sentido del efecto)

typedef struct                             // Estructura con toda la información (pack) relativa a la visualización de una foto, podrá ampliarse según convenga
{  
	uint8 photo[LCD_BUFFER_SIZE];          // Foto
	pf_t  effect;                          // Efecto de transición a aplicar para visualizarla
	uint8 sense;                           // Sentido del efecto a aplicar
	uint8 secs;                            // Segundos que debe estar visualizada
} pack_t;

typedef struct                             // Estructura conteniendo las fotos a visualizar, podrá ampliarse según convenga 
{
	uint8  numPacks;                       // Número de packs que contiene el album
	pack_t pack[MAX_PHOTOS];               // Array de fotos
} album_t;

/* Declaración del buffer de vídeo */

extern uint8 lcd_buffer[LCD_BUFFER_SIZE];

/*******************************************************************/

void main( void ){
	album_t album;
	uint16 i;
	uint16 modo;
	uint16 ticks;
	uint8 scancode;

	sys_init();
	segs_init();
	timers_init();
    keypad_init();
	lcd_init();

	lcd_clear();
	lcd_on();

	// Ejemplo de uso de las miniaturas (de este o distinto tamaño)

	lcd_putBmp( MINIARBOL, 20, 20, 128, 96 );       // ARBOL al 40% editada con Paint de Windows
	lcd_putBmp( MINIPICACHU, 168, 20, 128, 96 );    // PICACHU al 40% editada con Paint de Windows

	lcd_puts( 20, 130, BLACK, "Se usaran miniaturas para" );
	lcd_puts( 20, 146, BLACK, "configurar la transicion de fotos" );
	lcd_puts( 20, 162, BLACK, "Elige el modo de transición: " );
	lcd_puts( 20, 178, BLACK, "0-Con pausa, 1- Sin pausa " );
    ticks = 0;

    /* Se me ocurrió al principio hacerlo con el touchscreen, pero la pantalla tiene un rebote
     * y cuesta bastante que vaya bien, haciendolo más ineficiente que usando el keypad
     */
    while( ticks < 1)
    {
    	while (!keypad_pressed());
        switch( scancode = keypad_getchar() )
        {
            default:
            	if (scancode == KEYPAD_KEY0){
            		lcd_puts( 20, 194, BLACK, "Has seleccionado modo 0" );
            		modo = 0;
            		ticks++;
            	}
            	else if (scancode == KEYPAD_KEY1){
            		lcd_puts( 20, 194, BLACK, "Has seleccionado modo 1" );
            		modo = 1;
            	    ticks++;
            	}
                break;
        }
    }
	//sw_delay_s( 10 );

	// Creación del album de fotos

	i = 0;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoEmpuje;
	album.pack[i].sense = DOWN;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoEmpuje;
	album.pack[i].sense = LEFT;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoBarrido;
	album.pack[i].sense = UP;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoBarrido;
	album.pack[i].sense = RIGHT;
	i++;


	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoRevelado;
	album.pack[i].sense = DOWN;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoRevelado;
	album.pack[i].sense = LEFT;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoCobertura;
	album.pack[i].sense = UP;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoCobertura;
	album.pack[i].sense = RIGHT;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoDivisionEntrante;
	album.pack[i].sense = HORIZONTAL;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoDivisionEntrante;
	album.pack[i].sense = VERTICAL;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoDivisionSaliente;
	album.pack[i].sense = HORIZONTAL;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoDivisionSaliente;
	album.pack[i].sense = VERTICAL;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoBarras;
	album.pack[i].sense = HORIZONTAL;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoDesvanecer;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( ARBOL, album.pack[i].photo );
	album.pack[i].secs = 0;
	album.pack[i].effect = efectoNulo;
	album.pack[i].sense = NO_APPLY;
	i++;

	lcd_bmp2photo( PICACHU, album.pack[i].photo );
	album.pack[i].secs = 3;
	album.pack[i].effect = efectoFlash;
	album.pack[i].sense = NO_APPLY;
	i++;

	album.numPacks = i;
	// Carrusel de fotos demostrando que solo los efectos por DMA hacen transiciones aceptables

	i = 0;
	if (modo == 0){
		lcd_clear();
		lcd_puts( 20, 130, BLACK, "Presione 0 para comenzar: " );
	}
	while( 1 )
	{
		if (keypad_pressed()){
			scancode = keypad_getchar();
		}

		if (modo == 1 && scancode == KEYPAD_KEY0 && i % 2 == 0){
			modo = 0;
		}
		else if (modo == 0 && i % 2 == 0){
			scancode = keypad_getchar();
			while (!(scancode == KEYPAD_KEY0) && modo == 0){
				while (!keypad_pressed());
				scancode = keypad_getchar();
				if (keypad_getchar() == KEYPAD_KEY1){
					modo = 1;
				}
			}
		}

		(*album.pack[i].effect)( album.pack[i].photo, album.pack[i].sense );    // Ejecuta el efecto para visualizar la nueva foto
		sw_delay_s( album.pack[i].secs );                                       // Mantiene la foto el tiempo indicado
		i = ( i==album.numPacks-1 ? 0 : i+1 );                                  // Avanza circularmente a la siguiente foto del album
	}
}


/*******************************************************************/

/*
 ** Muestra un BMP de tamaño (xsize, ysize) píxeles en la posición (x,y) del LCD
 ** Esta función es una generalización de lcd_putWallpaper() ya que:
 **     lcd_putWallpaper( bmp ) = lcd_putBmp( bmp, 0, 0, LCD_WIDTH, LCD_HEIGHT )
 **
 ** NO puede hacerse por DMA porque requiere la manipulación de pixeles
 */

void lcd_putBmp( uint8 *bmp, uint16 x, uint16 y, uint16 xsize, uint16 ysize )
{
	uint32 headerSize;

	uint16 xSrc, ySrc, yDst;
	uint16 offsetSrc, offsetDst;

	headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);

	bmp = bmp + headerSize;

	for( ySrc=0, yDst=ysize-1; ySrc<ysize; ySrc++, yDst-- )
	{
		offsetDst = (yDst+y)*LCD_WIDTH/2+x/2;
		offsetSrc = ySrc*xsize/2;
		for( xSrc=0; xSrc<xsize/2; xSrc++ )
			lcd_buffer[offsetDst+xSrc] = ~bmp[offsetSrc+xSrc];
	}
}

/*
 ** Respecto al buffer de vídeo, el formato BMP tiene cabecera, las filas están volteadas y el color invertido,
 ** Esta función convierte los BMP en un array de pixeles directamente visualizable (foto) para facilitar su manipulación
 ** Es una adaptación de lcd_putWallpaper() que en lugar de copiar el BMP sobre el buffer de vídeo lo hace sobre el array photo
 **
 ** NO puede hacerse por DMA porque requiere la manipulación de pixeles.
 */

void lcd_bmp2photo( uint8 *bmp, uint8 *photo )
{
	uint32 headerSize;

	uint16 x, ySrc, yDst;
	uint16 offsetSrc, offsetDst;

	headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);    // Los datos de cabecera están en little-endian

	bmp = bmp + headerSize;                                                       // Salta cabecera

	for( ySrc=0, yDst=LCD_HEIGHT-1; ySrc<LCD_HEIGHT; ySrc++, yDst-- )             // Voltea verticalmente e invierte los pixels
	{
		offsetDst = yDst*LCD_WIDTH/2;
		offsetSrc = ySrc*LCD_WIDTH/2;
		for( x=0; x<LCD_WIDTH/2; x++ )
			photo[offsetDst+x] = ~bmp[offsetSrc+x];
	}
}

/*
 ** Chequea que se está visualizando la foto indicada comparando pixel a pixel el buffer de vídeo y la foto
 ** Incluida para uso exclusivo en depuración, deberá eliminarse del proyecto final
 **
 ** NO puede hacerse por DMA porque requiere la comparación de pixeles.
 */

void test( uint8 *photo )
{
	int16 x, y;

	for( x=0; x<LCD_COLS; x++ )
		for( y=0; y<LCD_ROWS; y++ )
			if( lcd_buffer[y*LCD_COLS+x] != photo[y*LCD_COLS+x] )
			{
				segs_putchar( 0xE );
				while( 1 );
			}
}

/*
 ** Visualiza una columna de la foto en una columna dada de la pantalla
 **
 ** NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
 */

void lcd_putColumn( uint16 xLcd, uint8 *photo, uint16 xPhoto )
{
	int16 y;

	for( y=0; y<LCD_ROWS; y++ )
		lcd_buffer[(y*LCD_COLS)+xLcd] = photo[(y*LCD_COLS)+xPhoto];
}

/*
 ** Visualiza una linea de la foto en una linea dada de la pantalla
 **
 ** Deberá recodificarse para que se realice mediante una única operación DMA ya que los pixeles de una linea son contiguos en memoria
 */

void lcd_putRow( uint16 yLcd, uint8 *photo, uint16 yPhoto )
{
	/*int16 x;

	for( x=0; x<LCD_COLS; x++ )
        lcd_buffer[(yLcd*LCD_COLS)+x] = photo[(yPhoto*LCD_COLS)+x];*/

	/*DMA*/
	ZDISRC0  = (0 << 30) | (1 << 28) | (uint32) (photo + (yPhoto*LCD_COLS)); // datos de 8b, dirección POST-DECREMENTADA, origen: posición de la última columna de la penultima fila
	ZDIDES0  = (2 << 30) | (1 << 28) | (uint32) (lcd_buffer + (yLcd*LCD_COLS));       // recomendada, dirección POST-DECREMENTADA, destino: posición de la última columna de la ultima fila
	ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (LCD_COLS); 	  	  // whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas por el de filas menos una
	ZDICNT0 |= (1 << 20);                                                               	  	  // enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
	ZDCON0   = 1 ;                                                                      	  	  // start DMA
	while(ZDCCNT0 & 0xFFFFF);                                                           	  	  // Espera a que la transferencia por DMA finalice*/
}

/*
 ** Visualiza una foto en la pantalla
 **
 ** Deberá recodificarse para que se realice mediante una única operación DMA ya que los pixeles de una imagen son contiguos en memoria
 */

void lcd_putPhoto( uint8 *photo )
{
	/*int16 x, y;

	for( y=0; y<LCD_ROWS; y++ )
        for( x=0; x<LCD_COLS; x++ )
            lcd_buffer[(y*LCD_COLS)+x] = photo[(y*LCD_COLS)+x];  */

	/*DMA*/
	ZDISRC0  = (0 << 30) | (1 << 28) | (uint32) (photo + (0*LCD_COLS)); 						// datos de 8b, dirección POST-DECREMENTADA, origen: buffer photo, posición de la primera columna de la primera fila
	ZDIDES0  = (2 << 30) | (1 << 28) | (uint32) (lcd_buffer + (0*LCD_COLS));       				// recomendada, dirección POST-DECREMENTADA, destino: buffer lcd, posición de la primera columna de la primera fila
	ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (LCD_COLS * LCD_ROWS); 	  	  	// whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas por el de filas
	ZDICNT0 |= (1 << 20);                                                               	  	// enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
	ZDCON0   = 1 ;                                                                      	  	// start DMA
	while(ZDCCNT0 & 0xFFFFF);                                                           	  	// Espera a que la transferencia por DMA finalice*
}

/*
 ** Scroll de una fila/columna por DMA
 ** Desplaza el contenido del LCD una linea en desplazamientos verticales, o una columna (formada por 2 pixeles adyacentes) en desplazamientos horizontales
 **
 ** Deberá completarse y ampliarse para poder realizar otros efectos
 */
/* La función está generalizada para poder realizar diferentes efectos, indicando su origen, destino y tamaño*/
void lcd_shift( uint8 sense, uint32 origen, uint32 destino, uint16 size)
{
	int16 x, y;

	switch( sense )
	{
	case LEFT:                                                            				// Al ser un desplazamiento a izquierda, tener en cuenta que columnas consecutivas son contiguas en memoria pero los segmentos de fila no (excepto que sean filas completas y consecutivas)
		ZDISRC0  = (0 << 30) | (1 << 28) | (uint32) (origen);    						// datos de 8b, dirección POST-INCREMENTADA, origen: posición de la segunda columna de la fila correspondiente
		ZDIDES0  = (2 << 30) | (1 << 28) | (uint32) (destino);        					// recomendada, dirección POST-INCREMENTADA, destino: posición de la primera columna de la fila correspondiente
		ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (size);        		// whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas menos una
		ZDICNT0 |= (1 << 20);                                                           // enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
		ZDCON0   = 1;                                                                   // start DMA
		while( ZDCCNT0 & 0xFFFFF );                                                     // Espera a que la transferencia por DMA finalice
		break;
	case RIGHT:                                                                         // Al ser un desplazamiento a izquierda, tener en cuenta que columnas consecutivas son contiguas en memoria pero los segmentos de fila no (excepto que sean filas completas y consecutivas)
		ZDISRC0  = (0 << 30) | (2 << 28) | (uint32) (origen);   						// datos de 8b, dirección POST-INCREMENTADA, origen: posición de la segunda columna de la fila correspondiente
		ZDIDES0  = (2 << 30) | (2 << 28) | (uint32) (destino);   	  					// recomendada, dirección POST-INCREMENTADA, destino: posición de la primera columna de la fila correspondiente
		ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (size);       		// whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas menos una
		ZDICNT0 |= (1 << 20);                                                           // enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
		ZDCON0   = 1;                                                                   // start DMA
		while( ZDCCNT0 & 0xFFFFF );                                                     // Espera a que la transferencia por DMA finalice
		break;
	case UP:                                                                            // Al ser un desplazamiento hacia abajo, tener en cuenta que filas consecutivas son contiguas en memoria
		ZDISRC0  = (0 << 30) | (1 << 28) | (uint32) (origen);       					// datos de 8b, dirección POST-DECREMENTADA, origen: posición de la última columna de la penultima fila
		ZDIDES0  = (2 << 30) | (1 << 28) | (uint32) (destino);            				// recomendada, dirección POST-DECREMENTADA, destino: posición de la última columna de la ultima fila
		ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (size); 	    		// whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas por el de filas menos una
		ZDICNT0 |= (1 << 20);                                                           // enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
		ZDCON0   = 1 ;                                                                  // start DMA
		while(ZDCCNT0 & 0xFFFFF);                                                       // Espera a que la transferencia por DMA finalice
		break;
	case DOWN:                                                                          // Al ser un desplazamiento hacia abajo, tener en cuenta que filas consecutivas son contiguas en memoria
		ZDISRC0  = (0 << 30) | (2 << 28) | (uint32) (origen); 							// datos de 8b, dirección POST-DECREMENTADA, origen: posición de la última columna de la penultima fila
		ZDIDES0  = (2 << 30) | (2 << 28) | (uint32)(destino);       					// recomendada, dirección POST-DECREMENTADA, destino: posición de la última columna de la ultima fila
		ZDICNT0  = (2 << 28) | (1 << 26) | (0 << 22) | (0 << 21) | (size); 	  			// whole service, unit tranfer mode, pooling mode, no autoreload, tamaño: el de columnas por el de filas menos una
		ZDICNT0 |= (1 << 20);                                                           // enable DMA (según manual debe hacerse en escritura separada a la escritura del resto de registros)
		ZDCON0   = 1 ;                                                                  // start DMA
		while(ZDCCNT0 & 0xFFFFF);                                                       // Espera a que la transferencia por DMA finalice
		break;
	}
}

/*******************************************************************/

/*
 ** Efecto nulo: Muestra la foto sin hacer efecto alguno
 **
 ** Incluido por homogeneidad
 */

void efectoNulo( uint8 *photo, uint8 sense )
{
	switch( sense )
	{
	default:
		lcd_putPhoto( photo );
		break;
	}
}

/*
 ** Efecto empuje: La nueva imagen hace un scroll conjunto con la imagen mostrada
 **
 ** Deberá completarse en el proyecto final
 */

void efectoEmpuje( uint8 *photo, uint8 sense )
{
	int16 x, y;

	switch( sense )
	{
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Desplaza toda la pantalla una columna a la izquierda
	 * 		origen: posición de la segunda columna de la fila correspondiente
	 * 		destino: posición de la primera columna de la fila correspondiente
	 * 		tamaño: el de columnas menos una
	 * - Visualiza la columna de la foto que corresponde en la ultima columna de la pantalla
	 */
	case LEFT:
		for( x=0; x<=LCD_COLS-1; x++ )
		{
			for (y=0; y<LCD_ROWS; y++){
				lcd_shift(LEFT, (uint32) lcd_buffer + (y * LCD_COLS) + 1, (uint32) lcd_buffer + (y * LCD_COLS), LCD_COLS - 1);
			}
			lcd_putColumn( LCD_COLS-1, photo, x );    //
		}
		break;
	/* - Recorre la foto por columnas de derecha a izquierda
	 * - Desplaza toda la pantalla una columna a la derecha
	 * 		origen: posición de la penultima columna de la fila correspondiente
	 * 		destino: posición de la ultima columna de la fila correspondiente
	 * 		tamaño: el de columnas menos una
	 * - Visualiza la columna de la foto que corresponde en la primera columna de la pantalla
	*/
	case RIGHT:
		for( x = LCD_COLS - 1; x >= 0; --x)
		{
			for (y=0; y<LCD_ROWS; y++){
				lcd_shift( RIGHT, (uint32) lcd_buffer + (y*LCD_COLS) + LCD_COLS - 2,  (uint32) lcd_buffer + (y*LCD_COLS)+ LCD_COLS - 1, LCD_COLS - 1);
			}
			lcd_putColumn( 0, photo, x);
		}
		break;
	/* - Recorre la foto por filas de arriba a abajo
	 * - Desplaza toda la pantalla una fila arriba
	 * 		origen: posición de la primera columna de la segunda fila
	 * 		destino: posición de la primera columna de la primera fila
	 * 		tamaño: el de columnas por filas menos uno
	 * - Visualiza la columna de la foto que corresponde en la ultima fila de la pantalla
	 */
	case UP:
		for(y = 0; y <= LCD_ROWS - 1; ++y)
		{
			lcd_shift( UP, (uint32) (lcd_buffer + ((0 + 1)*LCD_COLS)), (uint32) (lcd_buffer + (0*LCD_COLS)), (LCD_COLS) * (LCD_ROWS - 1));
			lcd_putRow( LCD_ROWS - 1, photo, y );
			sw_delay_ms( 10 );
		}
		break;
	/* - Recorre la foto por filas de abajo a arriba
	 * - Desplaza toda la pantalla una fila abajo
	 * 		origen: posición de la primera columna de la penúltima fila
	 * 		destino: posición de la primera columna de la última fila
	 * 		tamaño: el de columnas por filas
	 * - Visualiza la columna de la foto que corresponde en la primera fila de la pantalla
	 */
	case DOWN:
		for(y = LCD_ROWS - 1; y >= 0; --y)
		{
			lcd_shift( DOWN,  (uint32) (lcd_buffer + ((LCD_ROWS - 1)*LCD_COLS)), (uint32) (lcd_buffer + (LCD_ROWS*LCD_COLS)), (LCD_COLS) * (LCD_ROWS));
			lcd_putRow( 0, photo, y );
			sw_delay_ms( 10 );
		}
		break;
	}
}

/*
 ** Efecto barrido: La nueva imagen se superpone progresivamente sobre la imagen mostrada desde un lateral al opuesto
 */

void efectoBarrido( uint8 *photo, uint8 sense )
{
	int16 x, y;

	switch( sense )
	{
	case LEFT:											// NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
		for( x=LCD_COLS-1; x >= 0; x-- )                // Recorre la foto por columnas de derecha a izquierda
		{
			lcd_putColumn( x, photo, x);    			// Visualiza la columna de la foto que corresponde en la  columna correspondiente de la pantalla
			sw_delay_ms( 10 );    						// Retarda para que se vea con claridad el efecto
		}
		break;
	case RIGHT:											// NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
		for( x=0; x<=LCD_COLS-1; x++ )                  // Recorre la foto por columnas de izquierda a derecha
		{
			lcd_putColumn( x , photo, x );        		// Visualiza la columna de la foto que corresponde en la  columna correspondiente de la pantalla
			sw_delay_ms( 10 );    						// Retarda para que se vea con claridad el efecto
		}
		break;
	case UP:											// lcd_putRow es una función que muestra las filas a través de DMA
		for(y = LCD_ROWS - 1; y >= 0; --y)    			// Recorre la foto por filas de abajo hacia arriba
		{
			lcd_putRow( y, photo, y );                  // Visualiza la fila de la foto que corresponde en la fila correspondiente de la pantalla
			sw_delay_ms( 20 );    						// Retarda porque el efecto por DMA es demasiado rápido
		}
		break;
	case DOWN:											// lcd_putRow es una función que muestra las filas a través de DMA
		for(y = 0; y <= LCD_ROWS - 1; ++y)    			// Recorre la foto por filas de arriba hacia abajo
		{
			lcd_putRow( y, photo, y );                  // Visualiza la fila de la foto que corresponde en la fila correspondiente de la pantalla
			sw_delay_ms( 20 );    						// Retarda porque el efecto por DMA es demasiado rápido
		}
		break;
	}
}

/*
 ** Efecto revelado: La nueva imagen aparece conforme la imagen mostrada desaparece haciendo scroll
 */

void efectoRevelado( uint8 *photo, uint8 sense )
{
	int16 x, y;

	switch( sense )
	{
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Desplaza toda la pantalla una columna a la izquierda
	 * 		origen: posición de la segunda columna de la fila correspondiente
	 * 		destino: posición de la primera columna de la fila correspondiente
	 * 		tamaño: el de columnas menos una menos las que ya tienen la nueva imagen
	 * - Visualiza la columna de la foto que corresponde en la columna correspondiente de la pantalla
	 */
	case LEFT:
		for ( x=0; x<=LCD_COLS-1; x++ ){
			for (y = 0; y < LCD_ROWS; ++y){
				lcd_shift(LEFT, (uint32) lcd_buffer + (y * LCD_COLS) + 1, (uint32) lcd_buffer + (y * LCD_COLS), LCD_COLS - 1 - x);
			}
			lcd_putColumn(LCD_COLS - 1 - x, photo, LCD_COLS - 1 - x);
		}
		break;
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Desplaza toda la pantalla una columna a la derecha
	 * 		origen: posición de la penultima columna de la fila correspondiente
	 * 		destino: posición de la ultima columna de la fila correspondiente
	 * 		tamaño: el de columnas menos una menos las que ya tienen la nueva imagen
	 * - Visualiza la columna de la foto que corresponde en la columna correspondiente de la pantalla
	 */
	case RIGHT:
		for(x=0; x<=LCD_COLS-1; x++ )
		{
			for (y=0; y<LCD_ROWS; y++){
				lcd_shift( RIGHT, (uint32) lcd_buffer + (y*LCD_COLS) + LCD_COLS - 2,  (uint32) lcd_buffer + (y*LCD_COLS)+ LCD_COLS - 1, LCD_COLS - 1 - x);
			}
			lcd_putColumn( x, photo, x);
		}
		break;
	/* - Recorre la foto por filas de arriba a abajo
	 * - Desplaza toda la pantalla una fila arriba
	 * 		origen: posición de la primera columna de la segunda fila
	 * 		destino: posición de la primera columna de la primera fila
	 * 		tamaño: el de columnas por filas menos uno menos las filas que ya tienen la nueva imagen
	 * - Visualiza la columna de la foto que corresponde en la fila correspondiente de la pantalla
	 */
	case UP:
		for(y = 0; y < LCD_ROWS; ++y)
		{
			lcd_shift( UP, (uint32) (lcd_buffer + ((0 + 1)*LCD_COLS)), (uint32) (lcd_buffer + (0*LCD_COLS)), (LCD_COLS) * (LCD_ROWS - y - 1));
			lcd_putRow( LCD_ROWS - y - 1 , photo,  LCD_ROWS - y - 1);
			sw_delay_ms( 10 );
		}
		break;
	/* - Recorre la foto por filas de arriba a abajo
	 * - Desplaza toda la pantalla una fila abajo
	 * 		origen: posición de la primera columna de la penúltima fila
	 * 		destino: posición de la primera columna de la última fila
	 * 		tamaño: el de columnas por filas menos las filas que ya tienen la nueva imagen
	 * - Visualiza la columna de la foto que corresponde en la fila correspondiente de la pantalla
	 */
	case DOWN:
		for(y = 0; y < LCD_ROWS; ++y)
		{
			lcd_shift( DOWN,  (uint32) (lcd_buffer + ((LCD_ROWS - 1)*LCD_COLS)), (uint32) (lcd_buffer + ((LCD_ROWS)*LCD_COLS)), (LCD_COLS) * (LCD_ROWS - y));
			lcd_putRow( y, photo, y);
			sw_delay_ms( 10 );
		}
		break;
	}
}

/*
 ** Efecto cobertura: La nueva imagen se superpone haciendo scroll sobre la imagen mostrada
 */

void efectoCobertura( uint8 *photo, uint8 sense )
{
	int16 x, y;

	switch( sense )
	{
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Visualiza la columna de la foto que corresponde en la ultima columna de la pantalla
	 * - Desplaza la nueva imagen una columna a la izquierda
	 * 		origen: posición de la ultima columna de la fila correspondiente donde se encuentre la imagen antigua
	 * 		destino: posición de la primera columna de la fila correspondiente donde se encuentre la nueva imagen
	 * 		tamaño: el de las columnas que lleve desplazadas mas uno
	 * - Visualiza la columna de la foto que corresponde en la ultima columna de la pantalla
	 */
	case LEFT:
		for ( x=0; x< LCD_COLS-1; x++ ){
			lcd_putColumn(LCD_COLS - 1, photo, x);
			for (y = 0; y < LCD_ROWS; ++y){
				lcd_shift(LEFT, (uint32) lcd_buffer + (y * LCD_COLS) + (LCD_COLS - 1 - x)  , (uint32) lcd_buffer + (y * LCD_COLS) + (LCD_COLS - 2 - x) , x + 1);
			}
		}
		lcd_putColumn(LCD_COLS - 1, photo, LCD_COLS - 1);
		break;
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Visualiza la columna de la foto que corresponde en la primera columna de la pantalla
	 * - Desplaza la nueva imagen una columna a la izquierda
	 * 		origen: posición de la ultima columna de la fila correspondiente donde se encuentre la nueva imagen
	 * 		destino: posición de la primera columna de la fila correspondiente donde se encuentre la imagen antigua
	 * 		tamaño: el de las columnas que lleve desplazadas mas uno
	 */
	case RIGHT:
		for(x=0; x < LCD_COLS-1; x++ )
		{
			lcd_putColumn( 0, photo, LCD_COLS - 1 - x);
			for (y=0; y<LCD_ROWS; y++){
				lcd_shift( RIGHT, (uint32) lcd_buffer + (y*LCD_COLS) + x,  (uint32) lcd_buffer + (y*LCD_COLS) + x + 1, x + 1);
			}
			lcd_putColumn( 0, photo, 0);
		}
		break;
	/* - Recorre la foto por filas de arriba a abajo
	 * - Visualiza la fila de la foto que corresponde en la ultima fila de la pantalla
	 * - Desplaza la nueva imagen una fila arriba
	 * 		origen: posición de la primera columna de la primera fila donde se encuentre la imagen antigua
	 * 		destino: posición de la primera columna de la ultima fila donde se encuentre la nueva imagen
	 * 		tamaño: el de las columnas por el de las filas que lleve desplazadas
	 * - Visualiza la fila de la foto que corresponde en la ultima fila de la pantalla
	 */
	case UP:
		for(y = 0; y < LCD_ROWS - 1; ++y)
		{
			lcd_putRow( LCD_ROWS - 1, photo, y);
			sw_delay_ms( 10 );
			lcd_shift( UP, (uint32) (lcd_buffer + ((LCD_ROWS - y) *LCD_COLS)), (uint32) (lcd_buffer + ((LCD_ROWS - 1 - y)*LCD_COLS)), (LCD_COLS) * (y));
		}
		lcd_putRow( 0 , photo, 0);
		break;
	/* - Recorre la foto por columnas de izquierda a derecha
	 * - Visualiza la columna de la foto que corresponde en la primera fila de la pantalla
	 * - Desplaza la nueva imagen una fila abajo
	 * 		origen: posición de la primera columna de la ultima fila donde se encuentre la nueva imagen
	 * 		destino: posición de la primera columna de la primera fila donde se encuentre la imagen antigua
	 * 		tamaño: el de las columnas mas uno por el de las filas que lleve desplazadas
	 */
	case DOWN:
		for(y = 0; y < LCD_ROWS; ++y)
		{
			lcd_putRow(0 , photo, LCD_ROWS - y);
			sw_delay_ms( 10 );
			lcd_shift( DOWN, (uint32) (lcd_buffer + (y*LCD_COLS)), (uint32) (lcd_buffer + ((y + 1)*LCD_COLS)), (LCD_COLS + 1) * (y));
		}
		break;
	}
}

/*
 ** Efecto division entrante: La nueva imagen se superpone progresivamente sobre la imagen mostrada desde dos laterales opuestos hasta el centro
 */
void efectoDivisionEntrante( uint8 *photo, uint8 sense ){
	int16 x, y;

	switch( sense )
	{
	case VERTICAL:																// NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
		y = LCD_COLS-1;
		for( x= (LCD_COLS-1) / 2; x >= 0; x-- )									// Recorre la foto por columnas desde la mitad hacia la izquierda
		{
			lcd_putColumn( y, photo, y);										// Visualiza la columna correspondiente de la parte derecha
			lcd_putColumn( LCD_COLS-1 - y, photo, LCD_COLS-1 - y);				// Visualiza la columna correspondiente de la parte izquierda
			sw_delay_ms( 10 );
			--y;																// Recorre la foto por columnas desde la derecha hasta la mitad
		}
		break;
	case HORIZONTAL:
		x = LCD_ROWS - 1;
		for(y = (LCD_ROWS - 1) / 2; y >= 0; --y)								// Recorre la foto por filas desde la mitad hacia abajo
		{
			lcd_putRow( x, photo, x );											// Visualiza la fila correspondiente de la parte abajo
			lcd_putRow( LCD_ROWS - 1 - x, photo, LCD_ROWS - 1 - x);				// Visualiza la columna correspondiente de la parte abajo
			sw_delay_ms( 10 );
			--x;																// Recorre la foto por filas desde abajo hasta la mitad
		}
		break;
	}
}

/*
 ** Efecto division salienta: La nueva imagen se superpone progresivamente sobre la imagen mostrada desde el centro hasta los dos laterales opuestos
 */
void efectoDivisionSaliente( uint8 *photo, uint8 sense ){
	int16 x, y;

	switch( sense )
	{
	case VERTICAL:																// NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
		y = (LCD_COLS-1) / 2;
		for( x= (LCD_COLS-1) / 2; x >= 0; x-- )									// Recorre la foto por columnas desde la mitad hacia la izquierda
		{
			lcd_putColumn( x, photo, x);										// Visualiza la columna correspondiente de la parte izquierda
			lcd_putColumn( y, photo, y);										// Visualiza la columna correspondiente de la parte derecha
			sw_delay_ms( 10 );													// Retarda para que se vea con claridad el efecto
			++y;																// Recorre la foto por columnas desde la mitad hacia la derecha
		}
		lcd_putColumn( LCD_COLS-1, photo, LCD_COLS-1);
		break;
	case HORIZONTAL:
		x = (LCD_ROWS - 1) / 2;
		for(y = (LCD_ROWS - 1) / 2; y >= 0; --y)								// Recorre la foto por filas desde la mitad hacia abajo
		{
			lcd_putRow( x, photo, x);											// Visualiza la fila correspondiente de la parte arriba
			lcd_putRow( y, photo, y);											// Visualiza la fila correspondiente de la parte abajo
			sw_delay_ms( 10 );
			++x;																// Recorre la foto por filas desde la mitad hacia arriba
		}
		lcd_putRow( LCD_ROWS - 1, photo, LCD_ROWS - 1);
		break;
	}
}

/*
 ** Efecto barras: La nueva imagen se superpone progresivamente sobre la imagen rellenando barras aleatorias
 */
void efectoBarras( uint8 *photo, uint8 sense ){
	uint16 x, y;

	switch( sense )
	{
	case HORIZONTAL:															// NO puede hacerse por DMA porque los pixeles de una columna no ocupan posiciones contiguas de memoria
		for(y = 0; y <= LCD_ROWS - 1; ++y)										// Recorre la foto por filas desde la arriba hasta abajo
		{
			if (y % 2 == 0){													// Visualiza las filas pares
				lcd_putRow( y, photo, y );
				sw_delay_ms( 10 );
			}
		}
		for(y = 0; y <= LCD_ROWS - 1; ++y)										// Recorre la foto por filas desde la arriba hasta abajo
		{
			if (y % 2 != 0){													// Visualiza las filas impares
				lcd_putRow( y, photo, y );
				sw_delay_ms( 10 );
			}
		}
		break;
	case VERTICAL:
		for( x=LCD_COLS-1; x >= 0; x-- )										// Recorre la foto por columnas desde la derecha hacia la izquierda
		{
			if (x % 2 == 0){													// Visualiza las columnas pares
				lcd_putColumn( x, photo, x);
			}
		}
		for( x=LCD_COLS-1; x >= 0; x-- )										// Recorre la foto por columnas desde la derecha hacia la izquierda
		{
			if (x % 2 != 0){													// Visualiza las columnas impares
				lcd_putColumn( x, photo, x);
			}
		}
	}
}

/*
 ** Efecto desvanecer: La nueva imagen se superpone después de que la pantalla se quede en negro
 */
void efectoDesvanecer( uint8 *photo, uint8 sense ) {
	uint16 x, y;

	for( y=0; y<LCD_ROWS; y++ ){												// Muestra la pantalla en negro
		for( x=0; x<LCD_COLS; x++ ){
			lcd_buffer[(y*LCD_COLS)+x] = BLACK;
		}
	}
	sw_delay_ms( 1000 );

	lcd_putPhoto( photo );														// Visualiza la foto a traves de DMA
}

/*
 ** Efecto desvanecer: La nueva imagen se superpone después de que la pantalla se quede en blanco
 */
void efectoFlash( uint8 *photo, uint8 sense ) {
	uint16 x, y;

	for( y=0; y<LCD_ROWS; y++ ){
		for( x=0; x<LCD_COLS; x++ ){
			lcd_buffer[(y*LCD_COLS)+x] = WHITE;									// Muestra la pantalla en blanco
		}
	}
	sw_delay_ms( 1000 );

	lcd_putPhoto( photo );														// Visualiza la foto a traves de DMA
}


/* Funciones y efectos que no funcionan bien*/

/*
 ** lcd_putPixelColumn: Función que muestra los pixeles y filas recibidas por parámetro de entrada en x columna también recibida por parámetro de entrada
 */
void lcd_putPixelColumn( uint16 xLcd, uint8 *photo, uint16 xPhoto, uint16 init, uint16 fin)
{
	int16 y;

	for( y=init; y<fin; y++ )
		lcd_buffer[(y*LCD_COLS)+xLcd] = photo[(y*LCD_COLS)+xPhoto];
}


/*
 ** Efecto cuadrado entrante: La nueva imagen se superpone progresivamente rellenando los laterales hasta llegar al centro
 */
void efectoCuadradoEntrante( uint8 *photo, uint8 sense ){
	uint16 x, y, k;
	y = LCD_ROWS - 1;
	k = LCD_COLS-1;
	for (x = (LCD_ROWS - LCD_COLS); x >= 0; --x){
		lcd_putRow( y, photo, y);
		lcd_putRow( LCD_ROWS - 1 - y, photo, LCD_ROWS - 1 - y);
		--y;
		lcd_putColumn( k, photo, k);
		lcd_putColumn( LCD_COLS-1 - k, photo, LCD_COLS-1 - k);
		--k;
		sw_delay_ms( 10 );
	}
}

/*
 ** Efecto cuadrado entrante: La nueva imagen se superpone progresivamente rellenando el centro hasta llegar a los laterales
 */
void efectoCuadradoSaliente( uint8 *photo, uint8 sense ){
	uint16 x, y, k;
	y = (LCD_ROWS - 1) / 2;
	k = (LCD_COLS-1) / 2;
	for (x = ((LCD_ROWS - 1) * (LCD_COLS-1)) / 2; x >= 0; --x){
		lcd_putRow( y, photo, y);
		lcd_putRow( LCD_ROWS - 1 - y, photo, LCD_ROWS - 1 - y);
		sw_delay_ms( 10 );
		++y;
		lcd_putColumn( k, photo, k);
		lcd_putColumn( LCD_COLS-1 - k, photo, LCD_COLS-1 - k);
		sw_delay_ms( 10 );
		++k;
	}
}

/*
 ** Efecto peine: La nueva imagen aparece conforme la imagen mostrada desaparece haciendo scroll intercaladamente hacia a la izquierda y derecha
 */
void efectoPeine( uint8 *photo, uint8 sense ){
	uint16 x, y, cont, init;
	for ( x=0; x<=LCD_COLS-1; x++ ){
		cont = 0;
		init = 0;
		for (y = 0; y < LCD_ROWS; ++y){
			if (cont == 119 && sense == LEFT){
				lcd_putPixelColumn(LCD_COLS - 1 - x, photo, LCD_COLS - 1 - x, init, (y));
				init = y;
				cont = 0;
				sense = RIGHT;
			}
			else if (cont == 119){
				lcd_putPixelColumn(x, photo, x, init, (y));
				init = y;
				cont = 0;
				sense = RIGHT;
			}

			switch(sense){
			case LEFT:
				lcd_shift(LEFT, (uint32) lcd_buffer + (y * LCD_COLS) + 1, (uint32) lcd_buffer + (y * LCD_COLS), LCD_COLS - 1 - x);
				break;
			case RIGHT:
				lcd_shift( RIGHT, (uint32) lcd_buffer + (y*LCD_COLS) + LCD_COLS - 2,  (uint32) lcd_buffer + (y*LCD_COLS)+ LCD_COLS - 1, LCD_COLS - 1 - x);
				break;
			}
			++cont;
		}
		//lcd_putPixelColumn(LCD_COLS - 1 - x, photo, LCD_COLS - 1 - x, init, LCD_ROWS);
		sw_delay_ms(10);
	}
}






