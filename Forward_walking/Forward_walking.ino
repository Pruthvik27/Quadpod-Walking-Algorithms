#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Initialize the PWM driver, default I2C address is 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// --------------------- FUNCTION PROTOTYPES (Forward Declarations) ----------------------
// These are added to tell the compiler about the functions before they are defined later in the file.
void setAngle(uint8_t channel, int angle);
void stallAllLegs();
void setInitialPosition();

// --------------------- SERVO PULSE CONFIGURATION ----------------------
// These are the raw 12-bit values (0-4096) for the PCA9685 chip
// The values need tuning for your specific servos.
#define SERVOMIN 150  // Approx. 0 degrees
#define SERVOMAX 600  // Approx. 180 degrees
#define PULSE_90_DEGREE ((SERVOMIN + SERVOMAX) / 2) // Neutral Stand Position (Approx. 375)
#define PULSE_0_DEGREE SERVOMIN                     // Fully retracted/min position

// --------------------- INDIVIDUAL PCA9685 CHANNEL DEFINITIONS ----------------------

// --- Front Right (FR) Leg Servos ---
#define FR_LEG_LIFT_CH 8    // Leg Lift/Hip (Coxa)
#define FR_LEG_MIDDLE_CH 9  // Middle/Femur
#define FR_LEG_ROTATION_CH 10 // Rotation/Tibia

// --- Front Left (FL) Leg Servos ---
#define FL_LEG_LIFT_CH 5    // Leg Lift/Hip (Coxa)
#define FL_LEG_MIDDLE_CH 6  // Middle/Femur
#define FL_LEG_ROTATION_CH 7 // Rotation/Tibia

// --- Back Left (BL) Leg Servos ---
#define BL_LEG_LIFT_CH 2    // Leg Lift/Hip (Coxa)
#define BL_LEG_MIDDLE_CH 3  // Middle/Femur
#define BL_LEG_ROTATION_CH 4 // Rotation/Tibia

// --- Back Right (BR) Leg Servos ---
#define BR_LEG_LIFT_CH 11   // Leg Lift/Hip (Coxa)
#define BR_LEG_MIDDLE_CH 12 // Middle/Femur
#define BR_LEG_ROTATION_CH 13 // Rotation/Tibia

// --------------------- NEW HELPER FUNCTION TO SET ANGLE ----------------------

/**
 * @brief Converts a desired angle (0-180) into a raw PWM pulse value (SERVOMIN-SERVOMAX)
 * and sends the command to the specified channel.
 * @param channel The PCA9685 channel number to control.
 * @param angle The desired servo angle in degrees (0 to 180).
 */
void setAngle(uint8_t channel, int angle) {
  // Ensure the angle is within the 0 to 180 degree range
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  // Map the input angle (0-180) to the required pulse width (SERVOMIN-SERVOMAX)
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  
  // Send the command to the servo driver
  pwm.setPWM(channel, 0, pulse);
}

// --------------------- STALL HELPER FUNCTION ----------------------

/**
 * @brief Sets all 12 leg servos to the neutral 90-degree stall position.
 * This is used to ensure the Lift and Middle joints remain stationary.
 */
void stallAllLegs() {
    const int NEUTRAL_ANGLE = 90;

    // --- Front Right (FR) ---
    setAngle(FR_LEG_LIFT_CH, NEUTRAL_ANGLE);
    setAngle(FR_LEG_MIDDLE_CH, NEUTRAL_ANGLE);
    setAngle(FR_LEG_ROTATION_CH, NEUTRAL_ANGLE);

    // --- Front Left (FL) ---
    setAngle(FL_LEG_LIFT_CH, NEUTRAL_ANGLE);
    setAngle(FL_LEG_MIDDLE_CH, NEUTRAL_ANGLE);
    setAngle(FL_LEG_ROTATION_CH, NEUTRAL_ANGLE);

    // --- Back Left (BL) ---
    setAngle(BL_LEG_LIFT_CH, NEUTRAL_ANGLE);
    setAngle(BL_LEG_MIDDLE_CH, NEUTRAL_ANGLE);
    setAngle(BL_LEG_ROTATION_CH, NEUTRAL_ANGLE);

    // --- Back Right (BR) ---
    setAngle(BR_LEG_LIFT_CH, NEUTRAL_ANGLE);
    setAngle(BR_LEG_MIDDLE_CH, NEUTRAL_ANGLE);
    setAngle(BR_LEG_ROTATION_CH, NEUTRAL_ANGLE);
}


// --------------------- EXISTING FUNCTIONS ----------------------

void setInitialPosition() {
    // Sets all 12 leg servos to 90 degrees
    stallAllLegs(); 
}

void setup() {
    pwm.begin();
    pwm.setPWMFreq(60);  
    delay(10); 

    Serial.println("Setting all servos to neutral stand position (90 degrees)...");
    setInitialPosition();
    delay(2000); // Wait 2 seconds in the initial position

    Serial.println("Setup complete. Starting Rotation Servo Sweep Test.");
}

// --------------------- MAIN LOOP (ROTATION Sweep Test) ----------------------

void forward(void){
    
    for (int angle = 90; angle <= 135 ; angle += 1) {
        setAngle(BL_LEG_ROTATION_CH, angle);
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(15);
    }

    for (int angle = 135; angle >= 90 ; angle -= 1){
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(15);
    }

    for (int angle = 90; angle >= 45 ; angle -= 1) {
        setAngle(BR_LEG_ROTATION_CH, angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(15);
    }

    for (int angle = 45; angle <= 90 ; angle += 1){
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(15);
    }

    for (int angle = 135; angle >= 90 ; angle -= 1){
        setAngle(BL_LEG_ROTATION_CH, angle);
        setAngle(FL_LEG_ROTATION_CH, angle - 45);
        setAngle(FR_LEG_ROTATION_CH, 225 - angle);
        setAngle(BR_LEG_ROTATION_CH, 225 - angle);
        delay(15);
    }

    for (int angle = 45 ; angle <= 90 ; angle += 1) {
        setAngle(FL_LEG_ROTATION_CH, angle);
        setAngle(FL_LEG_MIDDLE_CH,135 - angle);
        delay(15);
    }

    for (int angle = 45 ; angle <= 90 ; angle +=1 ){
        setAngle(FL_LEG_MIDDLE_CH,angle);
        delay(15);
    }

    for (int angle = 135 ; angle >= 90 ; angle -= 1) {
        setAngle(FR_LEG_ROTATION_CH, angle);
        setAngle(FR_LEG_MIDDLE_CH,225-angle);
        delay(15);
    }

    for (int angle = 135 ; angle >= 90 ; angle -=1 ){
        setAngle(FR_LEG_MIDDLE_CH,angle);
        delay(15);
    }


    delay(500);

}

void loop() {// Delay in ms for smooth movement
    
    // 1. Ensure all 12 leg servos are exactly at 90 degrees and stall
    // This locks the LIFT and MIDDLE joints at 90 degrees throughout the sweep.
    stallAllLegs();
    forward();
}
    

