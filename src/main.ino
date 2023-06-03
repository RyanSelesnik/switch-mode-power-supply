#include <LiquidCrystal.h>

int pwm_output = 3;
int pot_pin = A0;
int output_voltage = A1;
int ouput_current = A2;
int output;
float current = 0;
int duty_set = 128;
float duty_cycle = 0.0;
float known_resistance = 0.1;
int count = 0;

#define NUM_SAMPLES 30

int voltage_sum = 0;                    // sum of samples taken
int current_sum = 0;                    // sum of samples taken
unsigned char voltage_sample_count = 0; // current sample number
unsigned char current_sample_count = 0;
float voltage = 0.0;
float prev_voltage = 0.0;
float prev_current = 0.0;


const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void changePWMFreq() {

  // Set PB0 as output (waveform output, pg. 204)
  PORTB.DIRSET = PIN0_bm;

  // Frequency: Fpwm_ss = Fclk_per/(N(PER+1))
  // Max resolution: Rpwm_ss = (log(PER+1))/(log(2))

  TCA0.SINGLE.PER = 256;

  // CMP sets the duty cycle of the PWM signal -> CT = CMP0 / PER
  // DUTY CYCLE is approximately 50% when CMP0 is PER / 2
  TCA0.SINGLE.CMP0 = 200;

  // Counter starts at 0
  TCA0.SINGLE.CNT = 0x00;

  // Configuring CTRLB register
  // Compare 0 Enabled: Output WO0 (PB0) is enabled
  // Single slope PWM mode is selected
  TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;

  // Using system clock (no frequency division, the timer clock frequency is Fclk_per)
  // Enable the timer peripheral
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}

void changeDutyCycle() {
  //    output = analogRead(pot_pin);
  //    duty_set = map(output, 0, 1023, 0, 255);
  voltage = getVoltage();
  if (voltage >= 5.05) {
    duty_set = duty_set - 1;
    duty_set = constrain(duty_set, 1, 128);
  } else if (voltage <= 4.9) {
    duty_set = duty_set + 1;
    duty_set = constrain(duty_set, 1, 128);
  }
 
  analogWrite(pwm_output, duty_set);
  duty_cycle = duty_set / 255.0;
  lcd.setCursor(8, 0);
  lcd.print(duty_cycle * 100);
  lcd.print("%");

}

float getVoltage() {

  // take a number of analog samples and add them up
  while (voltage_sample_count < NUM_SAMPLES) {
    voltage_sum += analogRead(output_voltage);
    voltage_sample_count++;
//    delay(1);
  }

  voltage = ((float)voltage_sum / (float)NUM_SAMPLES * 4.95 ) / 1024.0;
  voltage_sample_count = 0;
  voltage_sum = 0;
  return voltage * 2;
}

float getCurrent() {
  // take a number of analog samples and add them up
  while (current_sample_count < NUM_SAMPLES) {
    current_sum += analogRead(ouput_current);
    current_sample_count++;
//    delay(1);
  }

  current = ((float)current_sum / (float)NUM_SAMPLES * 4.95) / 1024.0 / known_resistance;
  
  current_sample_count = 0;
  current_sum = 0;
  return current;
}

void displayOutput() {
  current = getCurrent();
  voltage = getVoltage();
  if(count == 50) {
  if (voltage != prev_voltage) {
    lcd.setCursor(0, 0);
    lcd.print(voltage,1);
    lcd.print(" V");
  }

  if (current != prev_current) {
    lcd.setCursor(0, 1);
    lcd.print(current,1);
    lcd.print(" A");
  }
  count = 0;
  }
  prev_voltage = voltage;
  prev_current = current;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  changePWMFreq();
  pinMode(3, OUTPUT);
}

void loop() {
  Serial.begin(9600);
//
//    if (getCurrent() >= 0.6) {
//      // Turn off mosfet
//      analogWrite(3, 0);
//      while (1) {
//        lcd.setCursor(0, 0);
//        lcd.print("Too much current!!");
//      }
//    }
  changeDutyCycle();
  count++;
 displayOutput();
  delay(1);
}
