/********************** I2C functions **************************/

//don't forget to enter i2c frequency
// tested with 100kHz

void i2c_wait()
{
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));  // wait for MSSP module to be free (not busy)
}
 
void I2C_Init(uint32_t i2c_clk_freq)
{
    SSPCON  = 0x28;  // configure MSSP module to work in I2C mode
    SSPADD  = (_XTAL_FREQ/(4 * i2c_clk_freq)) - 1;  // set I2C clock frequency
    SSPSTAT = 0;
}
 
void I2C_Start()
{
    i2c_wait();
    SEN = 1;  // initiate start condition
}
 
void I2C_Repeated_Start()
{
    i2c_wait();
    RSEN = 1;  // initiate repeated start condition
}
 
void I2C_Stop()
{
    i2c_wait();
    PEN = 1;  // initiate stop condition
}
 
void I2C_Write(uint8_t i2c_data)
{
    i2c_wait();
    SSPBUF = i2c_data;  // update buffer
}
 
uint8_t I2C_Read(uint8_t ack)
{
    uint8_t _data;
    i2c_wait();
    RCEN = 1;
    i2c_wait();
    _data = SSPBUF;  // read data from buffer
    i2c_wait();
    // send acknowledge pulse ? (depends on ack, if 1 send, otherwise don't send)
    ACKDT = !ack;
    ACKEN = 1;
    return _data;  // return data read
}
 
/********************** end I2C functions **********************/