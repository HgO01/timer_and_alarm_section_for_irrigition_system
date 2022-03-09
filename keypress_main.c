/*
 * File:   keypress_main.c
 * Author: HgO
 *
 * Created on March 6, 2022, 12:42 PM
 */

// set configuration words
#pragma config FOSC = XT        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
 
#include <xc.h>
#include <stdint.h>
#include<stdbool.h>

#define _XTAL_FREQ  4000000
#include "LCD.h"
#include "i2c.h"
#include "adc.h"

////////////////// time variables ///////////////

char time[]     = "  :  :  ",
     Date[] = "  /  /20  ",
     Alarm1[]   = "A1:   :  :00", Alarm2[]   = "A2:   :  :00",
        
        water[] = "water delay:   ";
     //Temperature[] = "T:   .   C";

int8_t  i, second, minute, hour, w_day, m_day, month, year,
         alarm1_minute, alarm1_hour, alarm2_minute, alarm2_hour,
         status_reg, alarm1_status, alarm2_status, flag = 0;

int waatering_time = 3; //default to 3 secs

uint8_t thres_val = 500;
//////////////////////////////////////////////////////

#include "rtcc.h"

#define MENU    RB6
#define ENTER   RB7
#define UP      RB4
#define DOWN    RB5
#define LED     RC7
#define PUMP    RB1

///////////////Function parameters ///////////////
void disp_time();
void segment_selection(int cnt, uint8_t line);
bool is_up_key_pressed();
bool is_down_key_pressed();
void update_time();
void blink();
void EXT_Interrupt_Init();
uint8_t eeprom_read(uint8_t addr);
void eeprom_write(uint8_t addr, uint8_t data);
void water_display();

/////////////////////////////////////////////////

void delay_ms(unsigned int delay)
{
    
    while(delay > 0)
    {
        __delay_ms(1);
        
        delay--;
    }
}

void main(void) {
    
    //main variables
    //int next = 0, count = 0;
    int adc_value = 0;
    
    __delay_ms(1000);
    //eeprom_write(1, 03);
    waatering_time = eeprom_read(1); // get the last save value from address 1
    
    TRISB = 0xFF;
    
    TRISCbits.TRISC7 = 0; // configure the LED pin as output
    
    EXT_Interrupt_Init();
    
    LCD_Init();
    ADC_Init();
    
    I2C_Init(100000);
    
    LCD_Clear();
    LCD_Set_Cursor(1,1);
    
    LCD_Write_String("Welcome Loading...");
    __delay_ms(1000);
    
    LCD_Clear();
    
    LED = 0;
    
    char adc[] = "    ";
    while(1)
    {
        
        
//        adc_value  = ADC_Read(0); //read analog value from channel zero
//        
//        adc[0] = adc_value/100 + '0';
//        adc[1] = adc_value/10+'0';
//        adc[2] = (adc_value%10) + '0';
//        
//        LCD_Clear();
//        LCD_Set_Cursor(1,1);
//        LCD_Write_String(adc);
//        
//        __delay_ms(2000);
        
        if(!MENU) // is menu key pressed
        {
            __delay_ms(100);
            
            if(!MENU) 
            {
                //next = 0;
                update_time();
                
                
                
                eeprom_write(1, waatering_time); //save the last value of the watering delay in EEPROM
                
                
                // write data to DS3231 RTC
              I2C_Start();         // start I2C
              I2C_Write(0xD0);     // RTC chip address
              I2C_Write(0);        // send register address
              I2C_Write(0);        // reset seconds and start oscillator
              I2C_Write(minute);   // write minute value to RTC chip
              I2C_Write(hour);     // write hour value to RTC chip
              I2C_Write(w_day);    // write day of the week value to RTC chip
              I2C_Write(m_day);    // write date (day of the month) value to RTC chip
              I2C_Write(month);    // write month value to RTC chip
              I2C_Write(year);     // write year value to RTC chip
              I2C_Stop();          // stop I2C
            }
        }
        
        
        if(flag == 1 || adc_value > thres_val) // as LED turn ON
        {
            PUMP = 1; // activate pump
            flag = 0; //clear flag
            LED = 1; //turn the LED
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Watering...");
            
            delay_ms(waatering_time);
            
            PUMP = 0;
            LED = 0; // turn off LED then Alarm
            I2C_Start();             // start I2C
            I2C_Write(0xD0);         // RTC chip address
            I2C_Write(0x0E);         // send register address (control register)
            // write data to control register (Turn OFF the occurred alarm and keep the other as it is)
            I2C_Write(4 | ((!(status_reg & 1)) & alarm1_status) | (((!((status_reg >> 1) & 1)) & alarm2_status) << 1));
            I2C_Write(0);    // clear alarm flag bits
            I2C_Stop();      // stop I2C
        }
        
        
        RTC_read();
        RTC_display();
        alarms_read_display();
    }
    return;
}

void update_time()
{
    int next = 0;
    int line = 0;
    while(!MENU) //is menu key stilll pressed
    {
        if(!ENTER) // is enter key pressed then
        {
            __delay_ms(100); //debounce time
            if(!ENTER)
            {
                next++; // goto next segment example: from hour section to seconds
                if(next >2)
                {
                    next = 0;
                    line++;
                    
                    if(line > 3) line = 0;
                }
            }

        }
        
        
        segment_selection(next, line);
        
        if(line == 1)
        {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String(Alarm1);
        }
        
        else if(line == 2)
        {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String(Alarm2);
        }
        else if(line == 3)
        {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String(water);
            
            water_display();
        }
        
        else 
        {
            LCD_Clear();
            RTC_display();
        }
        
        __delay_ms(100);
    }
    
    return;
}

bool is_up_key_pressed()
{
    if(!UP)
    {
        __delay_ms(100);
        
        if(!UP)
            return true;
    }
    
    return false;
}

bool is_down_key_pressed()
{
    if(!DOWN)
    {
        __delay_ms(100);
        
        if(!DOWN)
            return true;
    }
    
    return false;
}

void segment_selection(int cnt, uint8_t line)// int line)
{
    
    //edit seconds 
    if(is_up_key_pressed() && cnt == 2 && !line)
    {
        second +=1;
        if(second > 59)
            second = 0;
    }
            
    if(is_down_key_pressed() && cnt == 2 && !line)
    {
        second -=1;
        if(second < 0)
            second = 0;
    }
    
    //for alarm and watering time delay
    if(is_up_key_pressed() && cnt == 0)
    {
        if(line == 0)
        {
            hour +=1;
            if(hour > 24)
                hour = 0;
        }
        
        
        else if(line == 1)
        {
            alarm1_hour +=1;
            if(alarm1_hour >= 24)
            alarm1_hour = 0;  
        }
        
        else if(line == 2)
        {
            alarm2_hour +=1;
            if(alarm2_hour >= 24)
            alarm2_hour = 0;  
        }
        
        else
        {
            waatering_time++;
        }
        
    }
            
    if(is_down_key_pressed() && cnt == 0)
    {
        if(line == 0){
            hour -=1;
            if(hour < 0)
                hour = 0;
        }
        
        else if(line == 1)
        {
            alarm1_hour -=1;
            if(alarm1_hour < 0)
            alarm1_hour = 0;  
        }
        
        else if(line == 2)
        {
            alarm2_hour -=1;
            if(alarm2_hour < 0)
            alarm2_hour = 0;  
        }
        
        else
        {
            waatering_time--;
        }
    }
    
    if(is_up_key_pressed() && cnt == 1)
    {
        
        if(line == 0)
        {
            minute +=1;
            if(minute > 59)
                minute = 0;
        }

        else if(line == 1)
        {
            alarm1_minute +=1;
            if(alarm1_minute > 59 )
            alarm1_minute = 0;  
        }
        
        else if(line == 2)
        {
            alarm2_minute +=1;
            if(alarm2_minute > 59)
                alarm2_minute = 0;  
        }
        
    }
            
    if(is_down_key_pressed() && cnt == 1)
    {
        if(line == 0)
        {
            minute -=1;
            if(minute < 0)
                minute = 0;
        }
            
        if(line == 1)
        {
            alarm1_minute -=1;
            if(alarm1_minute < 0)
                alarm1_minute = 0;  
        }
        
        else if(line == 2)
        {
            alarm2_minute -=1;
            if(alarm2_minute < 0)
                alarm2_minute = 0;  
        }
    }
    
    if(line == 0){
        switch(cnt)
        {
            case 0:
                LCD_Set_Cursor(1,1);
                LCD_Write_String("  ");
                blink();
                break;
            case 1:
                LCD_Set_Cursor(1,4);
                LCD_Write_String("  ");
                blink();
                break;
            case 2:
                LCD_Set_Cursor(1,7);
                LCD_Write_String("  ");
                blink();
                break;
        }
    }
    
    else if(line == 1 || line  == 2)
    {
        switch(cnt)
        {
            case 0:
                LCD_Set_Cursor(1,5);
                LCD_Write_String("  ");
                blink();
                break;
            case 1:
                LCD_Set_Cursor(1,8);
                LCD_Write_String("  ");
                blink();
                break;
            case 2:
                LCD_Set_Cursor(1,11);
                LCD_Write_String("  ");
                blink();
                break;
        }
    }
    
    else{
        switch(cnt)
        {
            case 0:
                LCD_Set_Cursor(1,13);
                LCD_Write_String("  ");
                blink();
                break;
        }
  
    }
    return;
}

void blink()
{
    uint8_t j = 0;
    
    while(j <100 && ENTER && UP && DOWN)
    {
        j++;
        
        __delay_ms(5);
    }
}

void EXT_Interrupt_Init()
{
    INTF   = 0;      // clear external interrupt flag bit
    GIE    = 1;      // enable global interrupts
    INTEDG = 0;      // set external Interrupt on falling edge
    INTE   = 1;      // enable external interrupt
}

void __interrupt() EXT(void)
{
  if (INTF){
    flag = 1;
    INTF = 0;
  }
}


void eeprom_write(uint8_t addr, uint8_t data)
{
    uint8_t gie_status;
    
    while(WR); // check for write operation
    
    EEADR = addr;
    EEDATA = data;
    
    EEPGD = 0;
    
    WREN = 1;
    
    gie_status = GIE;
    
    //data sequence
    EECON2 = 0x55;
    EECON2 = 0xAA;
    ////////////
    
    WR = 1;
    
    GIE = (char)gie_status;
    
    WREN = 0; //disable write operation
    
    while(EEIF == 0);
    
    EEIF = 0;
}


uint8_t eeprom_read(uint8_t addr)
{
    
    while(RD || WR);
    
    EEADR = addr;
    EEPGD = 0;
    
    RD = 1;
    
    
    return EEDATA;
}

void water_display()
{
    water[12] = waatering_time/10 + '0';
    water[13] = waatering_time%10 + '0';
}