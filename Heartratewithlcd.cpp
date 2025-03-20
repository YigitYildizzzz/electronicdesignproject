#include "mbed.h"
#include "TextLCD.h"


TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);

AnalogIn heartSensor(p15);  


float alpha = 0.5; 
float filteredInput = 0.0;
float previousFilteredInput = 0.0;

const float thresholdHigh = 0.5;  
const float thresholdLow = 0.3;
bool pulseDetected = false;
Timer timer;
float lastPulseTime = 0;
float bpm = 0;

int main() {
    lcd.cls();  
    lcd.printf("Heart Rate:");

    timer.start();  g

    while (true) {
     
        float voltageIn = heartSensor.read();
        filteredInput = alpha * voltageIn + (1 - alpha) * previousFilteredInput;
        previousFilteredInput = filteredInput;

      
        printf("Raw: %.2f, Filtered: %.2f\n", voltageIn, filteredInput);

      
        if (!pulseDetected && filteredInput > thresholdHigh) {
            pulseDetected = true;  
            float currentTime = timer.read();
            if (lastPulseTime > 0) {
                float timeBetweenPulses = currentTime - lastPulseTime;
                bpm = 60.0 / timeBetweenPulses; 
            }
            lastPulseTime = currentTime;
        }
        if (pulseDetected && filteredInput < thresholdLow) {
            pulseDetected = false;  
        }
    
        lcd.locate(0, 1);
        lcd.printf("BPM: %d    ", (int)bpm);

        ThisThread::sleep_for(500ms);
    }
}
