#include "mbed.h"
#include "TextLCD.h"

static const float THRESHOLD = 1.0f;  

AnalogIn pulseSensor(p15);            
DigitalOut led(LED1);                
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);  


Timer pulseTimer;
bool rising = false;  
float bpm = 0.0f;

void detectPulseAndComputeBPM(float sensorVal) {
    static float lastPulseTime = 0.0f;

    if (sensorVal > THRESHOLD && !rising) {
       
        float currentTime = pulseTimer.read();
        float dt = currentTime - lastPulseTime;

        if (dt > 0.2f && dt < 2.0f) {
            bpm = 60.0f / dt;
        }

        lastPulseTime = currentTime;
        rising = true;
    } else if (sensorVal < THRESHOLD) {
        rising = false;
    }
}

void displayBPM(float bpmVal) {
    int val = (int)(bpmVal + 0.5f);
    val = (val < 0) ? 0 : (val > 9999 ? 9999 : val);
    lcd.cls();
    lcd.printf("BPM: %d\n", val);
}


int main() {
    lcd.cls();
    lcd.printf("Initializing...\n");
    pulseTimer.start();

    while (true) {
        float val = pulseSensor.read_u16() / 65535.0f;

        detectPulseAndComputeBPM(val);
        displayBPM(bpm);

        led = (val > THRESHOLD) ? 1 : 0;

        ThisThread::sleep_for(50ms);
    }
}

