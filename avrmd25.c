/*
 * ros_robot.c
 *
 * Created: 12/25/2016 7:39:31 PM
 *  Author: kaviranga
 */ 
#ifndef F_CPU
#define F_CPU 16000000UL // 16Mhz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "avrmd25.h"

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

unsigned char buffer[10];		// Used to read data from i2c bus
unsigned long encorder1,encorder2;

int main(void)
{
 unsigned char t;	
	
	setup();										// General chip setup
	for(t = 0; t < 4; t++)
	{
		SetTimer(0xFFFF);							// Wait for everything to power up
		WaitForTimer();
	}
	
	setup_md25();
	i2c_receive(MD25, 13, 1);						// Get the software version of the MD25
    while(1)
    {
        drive(255);									// Start the motors driving forward
        do{
	        drive(255);
	        get_encoders();							// Get the encoder values and display them both on the LCD03
	        i2c_receive(MD25, 11, 2);					// Get the current for both motors
        }while(encorder1 < 0x00004000);
        drive(0);
        do{
	        drive(0);								// Start motor driving reverse
	        get_encoders();							// Get the encoder values and print to LCD03
	        i2c_receive(MD25, 11, 2);					// Get the current for both motors
        }while(encorder1 > 0x00001000);
    }
}

/********************
* Routines for MD25 *
********************/

void setup_md25(void)
{
	i2c_transmit(MD25,15,2);		// Set MD25 to mode 2, both motors controlled by speed 1
	i2c_transmit(MD25,16,32);		// Reset encoders to 0
}

void drive(char speed)
{
	i2c_transmit(MD25,0,speed);		// Drives the MD25 to value in speed
}

void get_encoders(void)
{
	i2c_receive(MD25,2,4);				// Get four bytes for first encoder
	encorder1 = buffer[0];					// Put together the four bytes that make encoder 1 value
	encorder1 = (encorder1<<8)+buffer[1];
	encorder1 = (encorder1<<8)+buffer[2];
	encorder1 = (encorder1<<8)+buffer[3];
	i2c_receive(MD25,6,4);				// get four bytes for second encoder
	encorder2 = buffer[0];					// Put together the four bytes that make encoder 2 value
	encorder2 = (encorder2<<8)+buffer[1];
	encorder2 = (encorder2<<8)+buffer[2];
	encorder2 = (encorder2<<8)+buffer[3];

}

void SetTimer(int time)
{
	OCR1A = time;			// Time set to count to
	TIFR = 0x10;			// Clear timer compare match flag
	TCNT1 = 0x00; 			// Clear timer to zero
	TCCR1B = START_CLK;		// Start timer running 1:8 Prescaler
}

void WaitForTimer(void)
{
	while(!(TIFR & 0x10));	// wait for timer to set compare match flag
	TCCR1B = STOP_CLK;		// Stop the timer
}

void setup(void)
{
	TCCR1A = 0x00;			// Set timer up in CTC mode
	TCCR1B = 0x08;
	
	TWBR = (uint8_t)TWBR_val;  
	
	//UCSRB = 0x18;			// Enable USART receiver and transmitter
	//UCSRC = 0x8E;			// Asynchronous mode, 8 data bits, 2 stop no parity
	//UBRRL = 0x33;			// USART Baud of 9600
}

void i2c_transmit(char address,char reg,char data)
{
	TWCR = 0xA4;                    // send a start bit on i2c bus
	while(!(TWCR & 0x80));          // wait for confirmation of transmit
	TWDR = address;                 // load address of i2c device
	TWCR = 0x84;                    // transmit
	while(!(TWCR & 0x80));          // wait for confirmation of transmit
	TWDR = reg;
	TWCR = 0x84;                    // transmit
	while(!(TWCR & 0x80));          // wait for confirmation of transmit
	TWDR = data;
	TWCR = 0x84;                    // transmit
	while(!(TWCR & 0x80));          // wait for confirmation of transmit
	TWCR = 0x94;                    // stop bit
} 

void i2c_receive(char address, char regRead,char byteCount)
{
  unsigned char x;

	TWCR = 0xA4;                     // send a start bit on i2c bus
	while(!(TWCR & 0x80));           // wait for confirmation of transmit
	TWDR = address;                  // load address of i2c device
	TWCR = 0x84;                     // transmit
	while(!(TWCR & 0x80));           // wait for confirmation of transmit
	TWDR = regRead;                  // send register number to read from
	TWCR = 0x84;                     // transmit
	while(!(TWCR & 0x80));           // wait for confirmation of transmit
    
	for(x = 0; x < (byteCount-1); x++)	// Receive data into buffer
	{
		TWCR = 0xA4;                    // send repeated start bit
		while(!(TWCR & 0x80));          // wait for confirmation of transmit
		TWDR = address+1;               // transmit address of i2c device with read bit set
		TWCR = 0xC4;                    // clear transmit interrupt flag
		while(!(TWCR & 0x80));          // wait for confirmation of transmit
		TWCR = 0x84;                    // transmit, ack (last byte request)
		while(!(TWCR & 0x80));          // wait for confirmation of transmit
		buffer[x] = TWDR;               // and grab the target data
		TWCR = 0x94;                    // send a stop bit on i2c bus
		//return read_data;
    }
		
	TWCR = 0xA4;                      // send repeated start bit
	while(!(TWCR & 0x80));            // wait for confirmation of transmit
	TWDR = address+1;                 // transmit address of i2c device with read bit set
	TWCR = 0xC4;                      // clear transmit interrupt flag
	while(!(TWCR & 0x80));            // wait for confirmation of transmit
	TWCR = 0x84;                      // transmit, nack (last byte request)
	while(!(TWCR & 0x80));            // wait for confirmation of transmit
	buffer[byteCount-1] = TWDR;       // and grab the target data
	TWCR = 0x94;                      // send a stop bit on i2c bus
}