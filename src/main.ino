#include <LiquidCrystal.h>

int pwm_output = 3; // Pin for PWM output
int pot_pin = A0; // Pin for potentiometer input
int output_voltage = A1; // Pin for output voltage measurement
int output_current = A2; // Pin for output current measurement

int output; // Placeholder variable
float current = 0; // Current value in Amps
int duty_set = 128; // Initial duty cycle setting
float duty_cycle = 0.0; // Current duty cycle value
float known_resistance = 0.1; // Known resistance value for current measurement

int count = 0; // Counter variable for display update

#define NUM_SAMPLES 30 // Number of analog samples to take for averaging

int voltage_sum = 0; // Sum of voltage samples
int current_sum = 0; // Sum of current samples
unsigned char voltage_sample_count = 0; // Counter for voltage samples
unsigned char current_sample_count = 0; // Counter for current samples

float voltage = 0.0; // Voltage value in Volts
float prev_voltage = 0.0; // Previous voltage value for comparison
float prev_current = 0.0; // Previous current value for comparison

const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // LCD object for display

void changePWMFreq() {
  // Set PB0 as output (waveform output)
  PORTB.DIRSET = PIN0_bm;

  // Frequency: Fpwm_ss = Fclk_per/(N(PER+1))
  // Max resolution: Rpwm_ss = (log(PER+1))/(log(2))

  TCA0.SINGLE.PER = 256; // Set period register for PWM frequency

  // CMP sets the duty cycle of the PWM signal -> CT = CMP0 / PER
  // DUTY CYCLE is approximately 50% when CMP0 is PER / 2
  TCA0.SINGLE.CMP0 = 200; // Set initial duty cycle

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
  voltage = getVoltage(); // Get current voltage reading

  // Adjust duty cycle based on voltage level
  if (voltage >= 5.05) {
    duty_set = duty_set - 1;
    duty_set = constrain(duty_set, 1, 128);
  } else if (voltage <= 4.9) {
    duty_set = duty_set + 1;
    duty_set = constrain(duty_set, 1, 128);
  }

  analogWrite(pwm_output, duty_set); // Apply new duty cycle
  duty_cycle = duty_set / 255.0; // Calculate duty cycle percentage
  lcd.setCursor(8, 0);
  lcd.print(duty_cycle * 100); // Display duty cycle as a percentage
  lcd.print("%");
}

float getVoltage() {
  // Take a number of analog samples and add them up
  while (voltage_sample_count < NUM_SAMPLES) {
    voltage_sum += analogRead(output_voltage);
    voltage_sample_count++;
  }

  // Calculate average voltage value
  voltage = ((float)voltage_sum / (float)NUM_SAMPLES * 4.95 ) / 1024.0;
  voltage_sample_count = 0;
  voltage_sum = 0;
  return voltage * 2; // Multiply by 2 due to voltage divider
}

float getCurrent() {
  // Take a number of analog samples and add them up
  while (current_sample_count < NUM_SAMPLES) {
    current_sum += analogRead(output_current);
    current_sample_count++;
  }

  // Calculate average current value
  current = ((float)current_sum / (float)NUM_SAMPLES * 4.95) / 1024.0 / known_resistance;
  current_sample_count = 0;
  current_sum = 0;
  return current;
}

void displayOutput() {
  current = getCurrent(); // Get current current reading
  voltage = getVoltage(); // Get current voltage reading

  // Update display if values have changed
  if (count == 50) {
    if (voltage != prev_voltage) {
      lcd.setCursor(0, 0);
      lcd.print(voltage, 1);
      lcd.print(" V");
    }

    if (current != prev_current) {
      lcd.setCursor(0, 1);
      lcd.print(current, 1);
      lcd.print(" A");
    }

    count = 0;
  }

  prev_voltage = voltage;
  prev_current = current;
}

void setup() {
  Serial.begin(9600); // Start serial communication
  lcd.begin(16, 2); // Initialize LCD display
  changePWMFreq(); // Configure PWM frequency
  pinMode(3, OUTPUT); // Set PWM output pin as output
}

void loop() {
  Serial.begin(9600);

  changeDutyCycle(); // Adjust duty cycle based on voltage
  count++;
  displayOutput(); // Update LCD display with voltage and current readings
  delay(1); // Delay for stability
}
