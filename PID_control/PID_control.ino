#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

// DS18B20 temperature sensor setup
#define ONE_WIRE_BUS 2 // Define the digital pin for the DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Peltier setup
int peltierPin = 3; // Digital pin connected to the Peltier module

// Setpoint temperatures
float setpoints[] = {55, 45, 40}; // Desired setpoint temperatures in Celsius
int setpointIndex = 0; // Index to keep track of the current setpoint

// Control parameters for PID
double Input, Output, Setpoint;
double Kp = 2; // Proportional gain
double Ki = 5; // Integral gain
double Kd = 1; // Derivative gain
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Time interval between temperature control (30 seconds)
unsigned long previousMillis = 0;
const unsigned long interval = 30000;

int totalCycles = 35; // Total number of cycles to complete
int currentCycle = 0; // Current cycle counter
bool temperatureControlEnabled = true; // Flag to enable temperature control

void setup() {
  pinMode(peltierPin, OUTPUT);
  sensors.begin();
  Serial.begin(9600);

  // Initialize PID control
  Input = sensors.getTempCByIndex(0); // Initial temperature
  Setpoint = setpoints[setpointIndex];
  myPID.SetMode(AUTOMATIC);
}

void loop() {
  unsigned long currentMillis = millis();
  if (temperatureControlEnabled && currentMillis - previousMillis >= interval) {
    // Read temperature
    sensors.requestTemperatures();
    Input = sensors.getTempCByIndex(0);

    // Compute PID control
    myPID.Compute();
    float pidOutput = Output;

    // Check if the current temperature is below the current setpoint
    if (Input < Setpoint) {
      digitalWrite(peltierPin, HIGH); // Turn on Peltier (cooling)
    } else {
      digitalWrite(peltierPin, LOW); // Turn off Peltier
      setpointIndex++; // Increment the setpoint index
      if (setpointIndex >= sizeof(setpoints) / sizeof(setpoints[0])) {
        // All setpoints reached, stop the process
        currentCycle++; // Increment the cycle counter
        if (currentCycle >= totalCycles) {
          // All cycles completed, stop the process
          temperatureControlEnabled = false; // Disable temperature control
          digitalWrite(peltierPin, LOW); // Turn off Peltier
          Serial.println("Process complete.");
        } else {
          // Reset the cycle for the next setpoint
          setpointIndex = 0;
          Setpoint = setpoints[setpointIndex];
        }
      } else {
        Setpoint = setpoints[setpointIndex]; // Set the new setpoint
      }
    }

    // Print current temperature, setpoint, and PID output
    Serial.print("Current Temperature: ");
    Serial.print(Input);
    Serial.print(" °C, Setpoint: ");
    Serial.print(Setpoint);
    Serial.print(" °C, PID Output: ");
    Serial.println(pidOutput);

    previousMillis = currentMillis;
  }
}
