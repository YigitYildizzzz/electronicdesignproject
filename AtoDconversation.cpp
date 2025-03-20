#include "mbed.h"
#include <cstdio>

AnalogIn Vin(p15);
AnalogOut Vout(p18);

int main() {
    float pulse = 0.0;
    float averagePulse = 0.0;
    float outputPulse = 0.0;
    float stepSize = 0.0;
    float min = 1.0;
    float max = 0.0;

    const int samplesNo = 10;
    float samples[samplesNo] = {0};
    int sampleIndex = 0;
    float sum = 0;

    while (true) {
        ThisThread::sleep_for(50ms);
   
        pulse = Vin.read();
        sum -= samples[sampleIndex];
        samples[sampleIndex] = pulse;
        sum += samples[sampleIndex];
        sampleIndex = (sampleIndex + 1) % samplesNo;
        averagePulse = sum / samplesNo;

        if (averagePulse > max) {
            max = averagePulse;
        }

        if (averagePulse < min) {
            min = averagePulse;
        }

        stepSize = (max - min) / 8.0;
        outputPulse = ((averagePulse - min) / (max-min)) * 8.0 * stepSize;
        Vout.write(outputPulse);
    }
   
}
