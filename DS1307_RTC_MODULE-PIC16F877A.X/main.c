#include <xc.h>
#include <pic16f877a.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define _XTAL_FREQ 20000000
#pragma config FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define RS PORTBbits.RB0
#define RW PORTBbits.RB1
#define EN  PORTBbits.RB2

#define DS1307_address 0xD0

#define second 0x00
#define minute 0x01
#define hour   0x02
#define day    0x03
#define date   0x04
#define month  0x05
#define year   0x06
#define control_reg  0x07

unsigned char __sec , __min , __hr, __day, __date, __month, __yr, __con ; 


void int_Port();
void int_I2C();

void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init();
void lcd_caracter(const unsigned char *str);


char decimal_to_bcd(unsigned char value );
void bcd_to_ascii(unsigned char value );



void DS1307_write(char sec,char min ,char hr, char _day,char _date,char _month ,char _year);
void DS1307_read(char slave_address,char register_address);



void main(void) {
    int_Port();
    lcd_init();
    int_I2C();
    
    lcd_cmd(0x80);
    lcd_caracter("- CLOCK: ");
    lcd_cmd(0xC0);
    lcd_caracter("- DATE: ");
    //DS1307_write(3,2,1,5,4,2,0);
    __delay_ms(300);
    
     lcd_cmd(0xD4);
    lcd_caracter("_DS1307 RTC MODULE_");
    while (1){  
        __delay_ms(300);
        DS1307_read(DS1307_address,0);
        }
    
    return;
}
void int_Port(){
    TRISB = 0x00;
    TRISC = 0xFF;   
};
void int_I2C(){
    SSPADD = 49;
    SSPCON = 0x28;
}

void lcd_cmd(unsigned char cmd){
    // 0x80 0xC0 0x94 0xD4
    PORTB = (cmd & 0xF0);
    EN = 1;
    RW = 0;
    RS = 0;
    __delay_ms(2);
    EN = 0 ;
    PORTB = ((cmd<<4)& 0xF0);
    EN = 1;
    RW = 0;
    RS = 0;
    __delay_ms(2);
    EN = 0;
}
void lcd_data(unsigned char data){
    PORTB = (data & 0xF0);
    EN = 1;
    RW = 0;
    RS = 1;
    __delay_ms(2);
    EN = 0 ;
    PORTB = ((data<<4)& 0xF0);
    EN = 1;
    RW = 0;
    RS = 1;
    __delay_ms(2);
    EN = 0;
}
void lcd_init(){
    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x04);
    lcd_cmd(0x01);
}
void lcd_caracter(const unsigned char *str){
    unsigned char i ,num = strlen(str) ;
    for(i=0;i<num;i++){
        lcd_data(str[i]);
    }
}

void bcd_to_ascii(unsigned char value){
    unsigned char bcd ;
    bcd = value ;
    bcd = bcd&0xf0 ;
    bcd = bcd>>4 ;
    bcd = bcd|0x30 ;
    lcd_data(bcd) ;
    bcd = value ;
    bcd = bcd&0x0f ;
    bcd = bcd|0x30 ;
    lcd_data(bcd) ;

}

char decimal_to_bcd(unsigned char value ){
    unsigned char msb,lsb,hex ;
    msb = value/10 ;
    lsb = value%10 ;
    hex = ((msb<<4)+lsb);
    return hex ;
}


void DS1307_write(char sec,char min ,char hr, char _day,char _date,char _month ,char _year){
    SSPCON2bits.SEN = 1 ;
    while(SEN);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = DS1307_address ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN = 1 ;
        while(PEN);
        return ;
    }
    
    SSPBUF = second ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN = 1 ;
        while(PEN);
        return ;
    }
    
    SSPBUF = decimal_to_bcd(sec);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(min);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(hr);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(_day);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(_date);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(_month);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = decimal_to_bcd(_year);
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = 0x00 ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    
    
    SSPCON2bits.PEN = 1 ;
    while(PEN);
}

void DS1307_read(char slave_address,char register_address){
    SSPCON2bits.SEN = 1 ;
    while(SEN);
    PIR1bits.SSPIF = 0;
    
    SSPBUF = slave_address ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN = 1 ;
        while(PEN);
        return ;
    }
    
    SSPBUF = register_address ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN = 1 ;
        while(PEN);
        return ;
    }
    
    SSPCON2bits.RSEN = 1 ;
    while(RSEN);
    PIR1bits.SSPIF = 0;
    
    
    SSPBUF = slave_address + 1 ;
    while(!SSPIF);
    PIR1bits.SSPIF = 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN = 1 ;
        while(PEN);
        return ;
    }
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __sec = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __min = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __hr = SSPBUF ;
    
     SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __day = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __date = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __month = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __yr = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.RCEN = 1 ;
    while(!SSPSTATbits.BF);
    __con = SSPBUF ;
    
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(ACKEN);
    
    SSPCON2bits.PEN = 1 ;
    while(PEN);
    
    lcd_cmd(0x88);
    bcd_to_ascii(__hr);
    lcd_data(':');
    bcd_to_ascii(__min);
    lcd_data(':');
    bcd_to_ascii(__sec);
    
    lcd_cmd(0xC7);
    bcd_to_ascii(__date);
    lcd_data('/');
    bcd_to_ascii(__month);
    lcd_data('/');
    bcd_to_ascii(__yr);
    
}





