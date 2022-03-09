/********************** RTC chip functions *********************/
// convert BCD to decimal function
uint8_t bcd_to_decimal(uint8_t number)
{
  return((number >> 4) * 10 + (number & 0x0F));
}
 
// convert decimal to BCD function
uint8_t decimal_to_bcd(uint8_t number)
{
  return(((number / 10) << 4) + (number % 10));
}
 
void RTC_read()    // read current time & date from RTC chip
{
  I2C_Start();           // start I2C
  I2C_Write(0xD0);       // RTC chip address
  I2C_Write(0);          // send register address
  I2C_Repeated_Start();  // restart I2C
  I2C_Write(0xD1);       // initialize data read
  second = I2C_Read(1);  // read seconds from register 0
  minute = I2C_Read(1);  // read minutes from register 1
  hour   = I2C_Read(1);  // read hour from register 2
  w_day  = I2C_Read(1);  // read day of the week from register 3
  m_day  = I2C_Read(1);  // read date from register 4
  month  = I2C_Read(1);  // read month from register 5
  year   = I2C_Read(0);  // read year from register 6
  I2C_Stop();            // stop I2C
}
 
void alarms_read_display()   // read and display alarm1 and alarm2 data function
{
  uint8_t control_reg;
  //int8_t temperature_msb;
  I2C_Start();          // start I2C
  I2C_Write(0xD0);      // RTC chip address
  I2C_Write(0x08);      // send register address (alarm1 minutes register)
  I2C_Repeated_Start(); // restart I2C
  I2C_Write(0xD1);      // initialize data read
  alarm1_minute = I2C_Read(1);   // read alarm1 minutes
  alarm1_hour   = I2C_Read(1);   // read alarm1 hours
  I2C_Read(1);                   // skip alarm1 day/date register
  alarm2_minute = I2C_Read(1);   // read alarm2 minutes
  alarm2_hour   = I2C_Read(1);   // read alarm2 hours
  I2C_Read(1);                   // skip alarm2 day/date register
  control_reg = I2C_Read(1);     // read control register
  status_reg  = I2C_Read(1);     // read status register
  I2C_Read(1);                   // skip aging offset register
  I2C_Read(1); // read temperature MSB
  I2C_Read(0); // read temperature LSB
  I2C_Stop();                    // stop I2C
  // convert BCD to decimal
  alarm1_minute = bcd_to_decimal(alarm1_minute);
  alarm1_hour   = bcd_to_decimal(alarm1_hour);
  alarm2_minute = bcd_to_decimal(alarm2_minute);
  alarm2_hour   = bcd_to_decimal(alarm2_hour);
  // end conversion
  // update Alarm1 and Alarm2
  Alarm1[8]     = alarm1_minute % 10  + '0';
  Alarm1[7]     = alarm1_minute / 10  + '0';
  Alarm1[5]     = alarm1_hour   % 10  + '0';
  Alarm1[4]     = alarm1_hour   / 10  + '0';
  Alarm2[8]     = alarm2_minute % 10  + '0';
  Alarm2[7]     = alarm2_minute / 10  + '0';
  Alarm2[5]     = alarm2_hour   % 10  + '0';
  Alarm2[4]     = alarm2_hour   / 10  + '0';
  alarm1_status = control_reg & 0x01;      // read alarm1 interrupt enable bit (A1IE) from RTC chip control register
  alarm2_status = (control_reg >> 1) & 0x01; // read alarm2 interrupt enable bit (A2IE) from RTC chip control register

  LCD_Set_Cursor(2, 1);
  LCD_Write_String(Alarm1);       // print Alarm1
  if(alarm1_status) {
    LCD_Set_Cursor(2, 11);
    LCD_Write_String("ON ");      // if A1IE = 1 print 'ON'
  }
  else {
    LCD_Set_Cursor(2, 11);
    LCD_Write_String("OFF");   // if A1IE = 0 print 'OFF'
  }
  LCD_Set_Cursor(2, 1);
  LCD_Write_String(Alarm2);    // print Alarm2
  if(alarm2_status){
    LCD_Set_Cursor(2, 11);
    LCD_Write_String("ON ");   // if A2IE = 1 print 'ON'
  }
  else {
    LCD_Set_Cursor(2, 11);
    LCD_Write_String("OFF");   // if A2IE = 0 print 'OFF'
  }
}
/*
void w_day_display()    // print day of the week
{
  LCD_Set_Cursor(1, 2);
  switch(w_day){
    case 1:  LCD_Print("Sun"); break;
    case 2:  LCD_Print("Mon"); break;
    case 3:  LCD_Print("Tue"); break;
    case 4:  LCD_Print("Wed"); break;
    case 5:  LCD_Print("Thu"); break;
    case 6:  LCD_Print("Fri"); break;
    default: LCD_Print("Sat");
  }
}
 
 */

// display time and date function
void RTC_display()
{
  // convert data from BCD format to decimal format
  second = bcd_to_decimal(second);
  minute = bcd_to_decimal(minute);
  hour   = bcd_to_decimal(hour);
  m_day  = bcd_to_decimal(m_day);
  month  = bcd_to_decimal(month);
  year   = bcd_to_decimal(year);
  // end conversion
  // update time
  time[7] = second % 10  + '0';
  time[6] = second / 10  + '0';
  time[4] = minute % 10  + '0';
  time[3] = minute / 10  + '0';
  time[1] = hour   % 10  + '0';
  time[0] = hour   / 10  + '0';
  // update date
  Date[9] = year  % 10 + '0';
  Date[8] = year  / 10 + '0';
  Date[4] = month % 10 + '0';
  Date[3] = month / 10 + '0';
  Date[1] = m_day  % 10 + '0';
  Date[0] = m_day  / 10 + '0';
 
  LCD_Set_Cursor(1, 1);    // go to column 1, row 1
  LCD_Write_String(time);   // print time
  //LCD_Goto(5, 2);    // go to column 1, row 2
  //LCD_Print(Date);   // print date
  //w_day_display();   // print day of the week
 
}
 