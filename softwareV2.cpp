#include "mbed.h"
#include "TextLCD.h"

#define BUFFER_SIZE 128
#define ROLLING_WINDOW 10
#define SAMPLE_RATE_HZ 80
#define SAMPLE_PERIOD chrono::milliseconds(1000 / SAMPLE_RATE_HZ)

// LED Matrix
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

// Pin inputs and outputs
AnalogIn Vin(p15);
DigitalIn switch_input(p9); // TRUE for LCD : FALSE for LED
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);
DigitalOut LED(LED1);
DigitalOut load(p8);
SPI max72_spi(p5, NC, p7);
BufferedSerial serial(USBTX, USBRX ,115200);
AnalogOut Vout(p19);

// Define circular buffer variables
Ticker sampler;
unsigned int buffer_head = 0;
bool buffer_full = false;
float buffer[BUFFER_SIZE];

// Define rolling buffer
float rolling_sum = 0.0;
float rolling_buffer[ROLLING_WINDOW];
int rolling_index = 0;
bool rolling_full = false;

// Define filtering variables
const float alpha = 0.7;
float filtered_input = 0.0;

// Define pulse rate detection variables
Timer pulse_timer;
const float threshold_high = 0.1;
const float threshold_low = 0.05;
unsigned int beat_count;

// LED matrix variables
unsigned int matrix_buffer[8];
unsigned int matrix_head = 0;

// Moving average calculation -> currently not using
float rolling_average(float new_value) {
    rolling_sum -= rolling_buffer[rolling_index]; // Remove old sample
    rolling_buffer[rolling_index] = new_value;   // Add new sample
    rolling_sum += new_value;                     // Update sum

    rolling_index = (rolling_index + 1) % ROLLING_WINDOW; // Move index

    if (!rolling_full && rolling_index == 0) {
        rolling_full = true;  // Rolling buffer is full after first n samples
    }

    return rolling_full ? (rolling_sum / ROLLING_WINDOW) : new_value;
}

// First-order low-pass filter
float apply_filter(float raw_input) {
    filtered_input = alpha * raw_input + (1 - alpha) * filtered_input;
    return filtered_input;
}

void show_on_LCD() {
    float time_between_beats = chrono::duration<float>(pulse_timer.elapsed_time()).count();
    float bpm = 60.0 / time_between_beats;
    int int_bpm = bpm * 100;

    if (bpm == 0 || bpm > 200) {
        printf("Unreliable signal\n");
    }
    else {
        printf("Heart Rate: %d BPM\n", int_bpm);
    }
}

void pulse_LED() {
    lcd.cls(); // clear LCD
    LED = !LED; // flash LED on/off when pulse detected
}

void show_on_matrix(float data) {
    // normalise each point to 8 bits
    unsigned int normalised_data = data * 7;

    // add to array of 8, shifting along
    matrix_buffer[matrix_head] = normalised_data;
    matrix_head = (matrix_head + 1) % BUFFER_SIZE; 

    // once array full and from then on light up LEDS on each column based on signal
    for (int i=0; i<8; i++) {
        //write_to_max(max7219_reg_digit0 + i, data[i])
    }
}

void write_to_max(int reg, int col) {
    load = LOW;            // begin
    max72_spi.write(reg);  // specify register
    max72_spi.write(col);  // put data
    load = HIGH;           // make sure data is loaded (on rising edge of LOAD/CS)
}

void setup_dot_matrix ()
{
    // initiation of the max 7219
    // SPI setup: 8 bits, mode 0
    max72_spi.format(8, 0);
    max72_spi.frequency(SAMPLE_RATE_HZ);
      

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

void sample_heart_rate() {
    // Read and filter signal
    float raw_voltage = Vin.read(); // Read from A0 (0.0 to 1.0)
    float smoothed_voltage = apply_filter(raw_voltage);
    //show_on_matrix(smoothed_voltage);

    // Store in circular buffer
    buffer[buffer_head] = smoothed_voltage;
    buffer_head = (buffer_head + 1) % BUFFER_SIZE; 

    if (buffer_head == 0) {
        buffer_full = true;
    }

    if (buffer_full) {
        static bool above_threshold = false;

        // Detect beats based on threshold crossings
        if (buffer[buffer_head] > threshold_high && !above_threshold) {
            above_threshold = true;

            if (beat_count > 0) {
                if (switch_input) {
                    show_on_LCD();
                }
                else {
                    pulse_LED();
                }
                pulse_timer.reset();
            }
            beat_count++;

        } else if (buffer[buffer_head] < threshold_low) {
            above_threshold = false;
        }
    }
}

int main()
{
    pulse_timer.start();

    while (true) {
        sample_heart_rate();
        ThisThread::sleep_for(SAMPLE_PERIOD);
    }
}
