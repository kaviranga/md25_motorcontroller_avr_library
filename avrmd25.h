/*
 * mainmd25.h
 *
 * Created: 12/26/2016 2:20:11 PM
 *  Author: kaviranga
 */ 

#ifndef MAINMD25_H_
#define MAINMD25_H_
#define	MD25	0xB0	              // Address of MD25
#define START_CLK	0x0A			  // Ctart clock with 1:8 prescaler
#define STOP_CLK	0x08			  // Stop clock


// Prototypes
void setup(void);
void setup_lcd(void);
void print_buff(void);
void set_cursor(unsigned char pos);
void i2c_transmit(char address,char reg,char data);
void i2c_receive(char address,char regRead,char byteCount);
void SetTimer(int time);
void WaitForTimer(void);
void setup_md25(void);
void drive(char speed);
void get_encoders(void);
void print_encoders(void);
void print_current(void);

#endif /* MAINMD25_H_ */