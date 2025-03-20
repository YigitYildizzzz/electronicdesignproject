#include "mbed.h"
//#include "SerialStream.h"
/*
example of driving maxim chip for Glasgow Uni Projects.

Dr J.J.Trinder 2013,14
jon.trinder@glasgow.ac.uk
 
*/

#define max7219_reg_noop         0x00
#define max7219_reg_digit0       0x01
#define max7219_reg_digit1       0x02
#define max7219_reg_digit2       0x03
#define max7219_reg_digit3       0x04
#define max7219_reg_digit4       0x05
#define max7219_reg_digit5       0x06
#define max7219_reg_digit6       0x07
#define max7219_reg_digit7       0x08
#define max7219_reg_decodeMode   0x09
#define max7219_reg_intensity    0x0a
#define max7219_reg_scanLimit    0x0b
#define max7219_reg_shutdown     0x0c
#define max7219_reg_displayTest  0x0f

#define LOW 0
#define HIGH 1

//FDRM PTD2 = MOSI, PTD1 = SCLK, LOAD = PTDO (digital pin);
//LPC1768 mosi = p5, sclk = p7, load = p8;
SPI max72_spi(p5, NC, p7);
//SPI max72_spi(PTD2, NC, PTD1);

DigitalOut load(p8); //will provide the load signal
//DigitalOut load(PTD0); //will provide the load signal

DigitalOut LEDout(LED1); //will provide the load signal

BufferedSerial serial(USBTX, USBRX ,115200);
//SerialStream<BufferedSerial> pc(serial);

char  pattern_diagonal[8] = { 0x01, 0x2,0x4,0x08,0x10,0x20,0x40,0x80};
char  pattern_square[8] = { 0xff, 0x81,0x81,0x81,0x81,0x81,0x81,0xff};  
char  pattern_star[8] = { 0x04, 0x15, 0x0e, 0x1f, 0x0e, 0x15, 0x04, 0x00};


/*
Write to the maxim via SPI
args register and the column data
*/
void write_to_max( int reg, int col)
{
    load = LOW;            // begin
    max72_spi.write(reg);  // specify register
    max72_spi.write(col);  // put data
    load = HIGH;           // make sure data is loaded (on rising edge of LOAD/CS)
    //pc.printf("Writing\n");
}

//writes 8 bytes to the display  
void pattern_to_display(char *testdata){
    int cdata; 
    for(int idx = 0; idx <= 7; idx++) {
        cdata = testdata[idx]; 
        write_to_max(idx+1,cdata);
    }
} 
 

void setup_dot_matrix ()
{
    // initiation of the max 7219
    // SPI setup: 8 bits, mode 0
    max72_spi.format(8, 0);
     
  
  
       max72_spi.frequency(100000); //down to 100khx easier to scope ;-)
      

    write_to_max(max7219_reg_scanLimit, 0x07);
    write_to_max(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
    write_to_max(max7219_reg_shutdown, 0x01);    // not in shutdown mode
    write_to_max(max7219_reg_displayTest, 0x00); // no display test
    for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
   // maxAll(max7219_reg_intensity, 0x0f & 0x0f);    // the first 0x0f is the value you can set
     write_to_max(max7219_reg_intensity,  0x08);     
 
}

void clear(){
     for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
}
//wait_ms has been depreciated
//ThisThread::sleep_for(BLINKING_RATE) is now used

int main()
{
    setup_dot_matrix ();      /* setup matric */
    while(1){
    //da_star();
    pattern_to_display(pattern_diagonal);
    LEDout = HIGH;
    thread_sleep_for(1000);
    LEDout = LOW;
    //pc.printf("Hello World\n");
    pattern_to_display(pattern_square);
    thread_sleep_for(1000);
    LEDout = HIGH;
    pattern_to_display(pattern_star);
    thread_sleep_for(1000);
    LEDout = LOW;
    thread_sleep_for(1000);
    clear();

    }
}
