#include <Stepper.h>

// Define the number of steps per revolution for your stepper motor
const int stepsPerRevolution = 200;  // Adjust this for your specific motor

// Initialize the stepper library on the pins
Stepper myStepper(stepsPerRevolution, 0, 1, 2, 3);

void setup() {
  // Set the speed of the motor (RPM)
  myStepper.setSpeed(60);  // 60 RPM
}

void loop() {
  // Turn the motor 30 degrees (assuming 200 steps per revolution, 30 degrees is 200 * 30 / 360 steps)
  int steps = stepsPerRevolution * 30 / 360;
  myStepper.step(steps);

  // Pause for a second
  delay(1000);

  // Turn back the motor 30 degrees to its original position
  myStepper.step(-steps);

  // Pause for a second
  delay(1000);
}
