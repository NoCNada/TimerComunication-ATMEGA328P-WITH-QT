/*
 * TimerComunication.c
 *
 * Created: 17/8/2021 10:09:56
 * Author : Lauta
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU	16000000L
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// typedef
typedef union{
	struct{
		uint8_t b0: 1;
		uint8_t b1: 1;
		uint8_t b2: 1;
		uint8_t b3: 1;
		uint8_t b4: 1;
		uint8_t b5: 1;
		uint8_t b6: 1;
		uint8_t b7: 1;
	}bit;
	uint8_t byte;
}_sFlag;
//..



// Defines
#define ALIVESENT flag1.bit.b0 //enviar alive
#define PLUS10MSFLAG flag1.bit.b1 // hacer cuenta pocha
#define MINUS10MSFLAG flag1.bit.b2 //hacer cuenta pocha
#define STARTTIMERFLAG flag1.bit.b3 //bandera activar timer primera vez
#define PRUEBA flag1.bit.b4 //bandera de cuando el timer esta contando
#define ALIVECMD 0xF0 // comando alive
#define PLUS10MSCMD 0xF1 // comando alive
 // comando subir 10ms
#define MINUS10MSCMD 0xF2 // comando bajar 10ms
//..

// Prototipos
void USART_Init();
void LeerCabecera(uint8_t ind);
void RecibirDatos(uint8_t head);
void EnviarDatos(uint8_t cmd);
void initPort();
void initTimers();
//..

// Variables Globales
volatile _sFlag flag1;
volatile uint16_t timeOut1ms = 500,valorTimeOut = 500;
volatile uint8_t indRX = 0, indTX = 0,indBufferRX = 0, indBufferTX = 0;
volatile uint8_t bufferRX[256], bufferTX[256];
uint8_t nBytes,cks,headDeco;
//..

// defines prototipos variables globales interrupciones funcines

//
//void USART_Transmit( unsigned char data )
//{
///* Wait for empty transmit buffer */
//while ( !(UCSRnA & (1<<UDREn)) )
//;
///* Put data into buffer, sends the data */
//UDRn = data;
//}

//Interrupciones
ISR(USART_RX_vect){
	bufferRX[indRX++] = UDR0;
}

ISR(TIMER1_COMPA_vect){
	if (timeOut1ms)
	{
		timeOut1ms--;
	}
	
}

//..

// Funciones
void USART_Init(){
	/* Configuración del USART como UART */

	// USART como UART
	UCSR0C &=~ (1<<UMSEL00);
	UCSR0C &=~ (1<<UMSEL01);

	// Paridad desactivada
	UCSR0C &=~ (1<<UPM00);
	UCSR0C &=~ (1<<UPM01);

	// Stops = 1
	UCSR0C &=~ (1<<USBS0);

	// Datos de 8 bits
	UCSR0C |=  (1<<UCSZ00);
	UCSR0C |=  (1<<UCSZ01);
	UCSR0B &=~ (1<<UCSZ02);
	
	// Calculo del baudrate
	UCSR0A &=~ (1<<U2X0);
	UBRR0 = MYUBRR;

	UCSR0B |= (1<<TXEN0); //activo recepcion de datos
	UCSR0B |= (1<<RXEN0); //activo envio de datos

	UCSR0B |= (1<<RXCIE0); //interrupcion de recepcion completada
}

void initPort(){
	//Configuro Pin del LED como salida (PB5 Arduino uno)
	DDRB = (1 << DDB5);
	
	PORTB |= (1 << PORTB5);
}

void initTimers(){
	OCR1A = 0x7D0;
	TCCR1A = 0x00;
	TCCR1B = 0x00; //reseteo registros
	TCNT1 = 0x00; //inicializo timer
	TIFR1 = TIFR1;
	TIMSK1 = (1 << OCIE1A); //activo flag interrupt cuando TCNT1 = OCR1A
	TCCR1B = (1 << WGM12) | (1 << CS11); //configuro prescaler 8 y modo CTC
}

void LeerCabecera(uint8_t ind){
	static uint8_t caracter = 0;
	
	while(ind != indBufferRX)
	{
		switch (caracter)
		{
			case 0:
			if (bufferRX[indBufferRX] == 'U')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 1:
			if (bufferRX[indBufferRX] == 'N')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 2:
			if (bufferRX[indBufferRX] == 'E')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 3:
			if (bufferRX[indBufferRX] == 'R')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 4:
			nBytes = bufferRX[indBufferRX];
			caracter++;
			break;
			case 5:
			if (bufferRX[indBufferRX] == 0x00)
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 6:
			if (bufferRX[indBufferRX] == ':')
			{
				cks= 'U'^'N'^'E'^'R'^nBytes^0x00^':';
				caracter++;
				headDeco = indBufferRX+1;
			}
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			
			case 7:
			if(nBytes>1){
				cks^=bufferRX[indBufferRX];
			}
			nBytes--;
			if(nBytes==0){
				caracter=0;
				if(cks==bufferRX[indBufferRX]){
					RecibirDatos(headDeco);
				}
			}
			break;
			default:
			caracter = 0;
			break;
		}
		indBufferRX++;
	}
}

void RecibirDatos(uint8_t head){
	switch (bufferRX[head++]){
		case 0xF0:
			ALIVESENT = 1;
			//algo
		break;
		case PLUS10MSCMD:
			PRUEBA = 1;
			PLUS10MSFLAG = 1;
		break;
		case MINUS10MSCMD:
			MINUS10MSFLAG = 1;
		break;
	}
}

void EnviarDatos(uint8_t cmd){
	bufferTX[indBufferTX++]='U';
	bufferTX[indBufferTX++]='N';
	bufferTX[indBufferTX++]='E';
	bufferTX[indBufferTX++]='R';
	
	switch(cmd){
		case ALIVECMD:
			bufferTX[indBufferTX++] = 0x02;
			bufferTX[indBufferTX++] = 0x00;
			bufferTX[indBufferTX++] = ':';
			bufferTX[indBufferTX++] = cmd;
			ALIVESENT = 0;
		break;
		case PLUS10MSCMD:
			bufferTX[indBufferTX++] = 0x02;
			bufferTX[indBufferTX++] = 0x00;
			bufferTX[indBufferTX++] = ':';
			bufferTX[indBufferTX++] = PLUS10MSCMD;
		break;
		case MINUS10MSCMD:
			bufferTX[indBufferTX++] = 0x02;
			bufferTX[indBufferTX++] = 0x00;
			bufferTX[indBufferTX++] = ':';
			bufferTX[indBufferTX++] = MINUS10MSCMD;
		break;
		
		case 0xF5:
			bufferTX[indBufferTX++] = 0x02;
			bufferTX[indBufferTX++] = 0x00;
			bufferTX[indBufferTX++] = ':';
			bufferTX[indBufferTX++] = 0xF5;
		break;
		case 0xF6:
			bufferTX[indBufferTX++] = 0x02;
			bufferTX[indBufferTX++] = 0x00;
			bufferTX[indBufferTX++] = ':';
			bufferTX[indBufferTX++] = 0xF6;
		break;
		default:
		break;
	}
	
	cks=0;
	for(uint8_t i=indTX; i<indBufferTX; i++) {
		cks^=bufferTX[i];
		//pc.printf("%d - %x - %d   v: %d \n",i,cks,cks,tx[i]);
	}
	if(cks>0)
	bufferTX[indBufferTX++]=cks;
}

//..

int main(void)
{
	USART_Init();
	initPort();
	//valorTimeOut=timeOut1ms;
	initTimers();
	sei();
	/* Replace with your application code */
	while (1)
	{
		if (indRX!=indBufferRX)
		{
			LeerCabecera(indRX);
		}
		
		if (ALIVESENT)
		{
			EnviarDatos(ALIVECMD);
		}
		if (PLUS10MSFLAG)
		{
			//EnviarDatos(PLUS10MSCMD);
			if(valorTimeOut<3000)
				valorTimeOut += 100;
			else
				EnviarDatos(0xF5);
			PLUS10MSFLAG = 0;
		}
		if (MINUS10MSFLAG)
		{
			//EnviarDatos(MINUS10MSCMD);
				if(valorTimeOut>100)
					valorTimeOut -= 100;
				else
					EnviarDatos(0xF6);
			MINUS10MSFLAG = 0;
		}
		if (!timeOut1ms){
			timeOut1ms = valorTimeOut;
			//(1 << PORTB5 1) desplazado por B5 es igual a 00100000
			if(PORTB & (1 << PORTB5)) //desplazo al numero 1 cinco veces para que aparezca en la posicion 5
				PORTB &= ~(1 << PORTB5); //~ (1 << PORTB5) = 11011111 hago cero el bit 5 de PORTB5
			else
				PORTB |= (1 << PORTB5);
		}
		
		while (indTX!=indBufferTX){
			while(!(UCSR0A & (1<<UDRE0)));
			UDR0 = bufferTX[indTX++];
		}

	}
}
