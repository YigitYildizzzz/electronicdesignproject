#include "mbed.h"
#include "TextLCD.h"

#define BLINKING_RATE     500ms
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);

int c;
int main()
{

    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    DigitalOut led4(LED4);
    c = 0;

    while (true) {
        led1 = !led1; 
        led2 = !led2;
        led3 = !led3;
        led4 = !led4;
        lcd.printf("HeartSync  %d \n", c); 
        ++c; 
        lcd.printf("HeartSync  %d \n", c); 
        ThisThread::sleep_for(BLINKING_RATE); 
    }
}