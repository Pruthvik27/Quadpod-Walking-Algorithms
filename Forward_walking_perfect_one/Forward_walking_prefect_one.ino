#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Initialize the PWM driver, default I2C address is 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// --------------------- FUNCTION PROTOTYPES (Forward Declarations) ----------------------
void setAngle(uint8_t channel, int angle);
void stallAllLegs();
void setInitialPosition();
void forward_efficient(); // The efficient walking function
void initialsetups(void); // Retained for setup/initial sweeps

// --------------------- SERVO PULSE CONFIGURATION ----------------------
// *** TUNE THESE VALUES FOR YOUR SERVOS ***
#define SERVOMIN 150  // Approx. 0 degrees 
#define SERVOMAX 600  // Approx. 180 degrees 
#define PULSE_90_DEGREE ((SERVOMIN + SERVOMAX) / 2)

// --------------------- INDIVIDUAL PCA9685 CHANNEL DEFINITIONS ----------------------

// --- Front Right (FR) Leg Servos ---
#define FR_LEG_LIFT_CH 7    // Leg Lift/Hip (Coxa)
#define FR_LEG_MIDDLE_CH 8  // Middle/Femur
#define FR_LEG_ROTATION_CH 9 // Rotation/Tibia

// --- Front Left (FL) Leg Servos ---
#define FL_LEG_LIFT_CH 4    // Leg Lift/Hip (Coxa)
#define FL_LEG_MIDDLE_CH 5  // Middle/Femur
#define FL_LEG_ROTATION_CH 6 // Rotation/Tibia

// --- Back Left (BL) Leg Servos ---
#define BL_LEG_LIFT_CH 1    // Leg Lift/Hip (Coxa)
#define BL_LEG_MIDDLE_CH 2  // Middle/Femur
#define BL_LEG_ROTATION_CH 3 // Rotation/Tibia

// --- Back Right (BR) Leg Servos ---
#define BR_LEG_LIFT_CH 10   // Leg Lift/Hip (Coxa)
#define BR_LEG_MIDDLE_CH 11 // Middle/Femur
#define BR_LEG_ROTATION_CH 12 // Rotation/Tibia

// --------------------- EFFICIENT GAIT CONFIGURATION ----------------------
#define STEP_COUNT 30       // Higher = smoother motion
#define STEP_DELAY 20       // Lower = faster walk
#define SWING_ANGLE 45      // Stride length (degrees of rotation)

// --- LIFT/MIDDLE ANGLES FOR FR & BL (Group 1 - Original Angles) ---
#define FBL_LIFT_ANGLE_MIN 45   // Max lift height (lowest angle value)
#define FBL_LIFT_ANGLE_MAX 90   // Grounded position (neutral angle value)

// --- LIFT/MIDDLE ANGLES FOR FL & BR (Group 2 - NEW ANGLES) ---
#define FLBR_LIFT_ANGLE_MIN 135 // Max lift height (highest angle value - 90 to 135)
#define FLBR_LIFT_ANGLE_MAX 90  // Grounded position (neutral angle value)


// --------------------- HELPER FUNCTION TO SET ANGLE ----------------------

void setAngle(uint8_t channel, int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(channel, 0, pulse);
}

// --------------------- STALL HELPER FUNCTION ----------------------

void stallAllLegs() {
    // Stall FR and BL at 90
    setAngle(FR_LEG_LIFT_CH, 90);
    setAngle(FR_LEG_MIDDLE_CH, 90);
    setAngle(BL_LEG_LIFT_CH, 90);
    setAngle(BL_LEG_MIDDLE_CH, 90);
    
    // Stall FL and BR at 90 (NEW LIFT_ANGLE_MAX)
    setAngle(FL_LEG_LIFT_CH, FLBR_LIFT_ANGLE_MAX); 
    setAngle(FL_LEG_MIDDLE_CH, FLBR_LIFT_ANGLE_MAX);
    setAngle(BR_LEG_LIFT_CH, FLBR_LIFT_ANGLE_MAX);
    setAngle(BR_LEG_MIDDLE_CH, FLBR_LIFT_ANGLE_MAX);

    // Stall all rotation channels at 90
    setAngle(FR_LEG_ROTATION_CH, 90);
    setAngle(FL_LEG_ROTATION_CH, 90);
    setAngle(BL_LEG_ROTATION_CH, 90);
    setAngle(BR_LEG_ROTATION_CH, 90);
}

// --------------------- INITIAL POSITION ----------------------

void setInitialPosition() {
    stallAllLegs();
}

// --------------------- EFFICIENT FORWARD MOVEMENT (TRIPOD GAIT) ----------------------

void forward_efficient() {

    // --- Phase 1: Move FR and BL legs forward (Swing) while FL and BR push back (Power Stroke) ---
    for (int i = 0; i < STEP_COUNT; i++) {
        
        // 1. Calculate angles for **Moving Legs (FR, BL) - Lift, Swing, Lower**
        
        // Rotation: Move from back (90 - SWING_ANGLE) to front (90 + SWING_ANGLE)
        int rot_swing_angle = map(i, 0, STEP_COUNT - 1, 90 - SWING_ANGLE, 90 + SWING_ANGLE);

        // FR/BL Lift/Middle: Cycles between 90 (Grounded) and 45 (Lifted)
        int fbl_lift_angle;
        if (i < STEP_COUNT / 2) {
            fbl_lift_angle = map(i, 0, STEP_COUNT/2 - 1, FBL_LIFT_ANGLE_MAX, FBL_LIFT_ANGLE_MIN);
        } else {
            fbl_lift_angle = map(i, STEP_COUNT/2, STEP_COUNT - 1, FBL_LIFT_ANGLE_MIN, FBL_LIFT_ANGLE_MAX);
        }
        
        // Apply angles to Moving Legs (FR, BL)
        setAngle(FR_LEG_LIFT_CH, fbl_lift_angle);
        setAngle(FR_LEG_MIDDLE_CH, fbl_lift_angle);
        setAngle(FR_LEG_ROTATION_CH, 180 - rot_swing_angle); // Adjusted for mirrored servo

        setAngle(BL_LEG_LIFT_CH, fbl_lift_angle);
        setAngle(BL_LEG_MIDDLE_CH, fbl_lift_angle);
        setAngle(BL_LEG_ROTATION_CH, rot_swing_angle);


        // 2. Calculate angles for **Supporting Legs (FL, BR) - Power Stroke**
        
        // Rotation: Move from front (90 + SWING_ANGLE) to back (90 - SWING_ANGLE)
        int rot_power_angle = map(i, 0, STEP_COUNT - 1, 90 + SWING_ANGLE, 90 - SWING_ANGLE);

        // FL/BR Lift/Middle: Stay grounded (FLBR_LIFT_ANGLE_MAX = 90)
        setAngle(FL_LEG_LIFT_CH, FLBR_LIFT_ANGLE_MAX);
        setAngle(FL_LEG_MIDDLE_CH, FLBR_LIFT_ANGLE_MAX);
        setAngle(FL_LEG_ROTATION_CH, rot_power_angle);

        setAngle(BR_LEG_LIFT_CH, FLBR_LIFT_ANGLE_MAX);
        setAngle(BR_LEG_MIDDLE_CH, FLBR_LIFT_ANGLE_MAX);
        setAngle(BR_LEG_ROTATION_CH, 180 - rot_power_angle); // Adjusted for mirrored servo

        delay(STEP_DELAY);
    }

    // --- Phase 2: Move FL and BR legs forward (Swing) while FR and BL push back (Power Stroke) ---
    for (int i = 0; i < STEP_COUNT; i++) {
        
        // 1. Calculate angles for **Moving Legs (FL, BR) - Lift, Swing, Lower**
        int rot_swing_angle = map(i, 0, STEP_COUNT - 1, 90 - SWING_ANGLE, 90 + SWING_ANGLE);
        
        // FL/BR Lift/Middle: Cycles between 90 (Grounded) and 135 (Lifted)
        int flbr_lift_angle;
        if (i < STEP_COUNT / 2) {
            // Lifting Up (90 up to 135 down)
            flbr_lift_angle = map(i, 0, STEP_COUNT/2 - 1, FLBR_LIFT_ANGLE_MAX, FLBR_LIFT_ANGLE_MIN);
        } else {
            // Lowering Down (135 down to 90 up)
            flbr_lift_angle = map(i, STEP_COUNT/2, STEP_COUNT - 1, FLBR_LIFT_ANGLE_MIN, FLBR_LIFT_ANGLE_MAX);
        }

        // Apply angles to Moving Legs (FL, BR)
        setAngle(FL_LEG_LIFT_CH, flbr_lift_angle);
        setAngle(FL_LEG_MIDDLE_CH, flbr_lift_angle);
        setAngle(FL_LEG_ROTATION_CH, rot_swing_angle);

        setAngle(BR_LEG_LIFT_CH, flbr_lift_angle);
        setAngle(BR_LEG_MIDDLE_CH, flbr_lift_angle);
        setAngle(BR_LEG_ROTATION_CH, 180 - rot_swing_angle); // Adjusted for mirrored servo


        // 2. Calculate angles for **Supporting Legs (FR, BL) - Power Stroke**
        int rot_power_angle = map(i, 0, STEP_COUNT - 1, 90 + SWING_ANGLE, 90 - SWING_ANGLE);

        // FR/BL Lift/Middle: Stay grounded (FBL_LIFT_ANGLE_MAX = 90)
        setAngle(FR_LEG_LIFT_CH, FBL_LIFT_ANGLE_MAX);
        setAngle(FR_LEG_MIDDLE_CH, FBL_LIFT_ANGLE_MAX);
        setAngle(FR_LEG_ROTATION_CH, 180 - rot_power_angle); // Adjusted for mirrored servo

        setAngle(BL_LEG_LIFT_CH, FBL_LIFT_ANGLE_MAX);
        setAngle(BL_LEG_MIDDLE_CH, FBL_LIFT_ANGLE_MAX);
        setAngle(BL_LEG_ROTATION_CH, rot_power_angle);

        delay(STEP_DELAY);
    }
}

void rotationLeft(void){
    for( int angle = 90 ; angle <= 180 ; angle++){
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        setAngle(BL_LEG_LIFT_CH,180-angle);
        setAngle(BL_LEG_MIDDLE_CH,180-angle);
        setAngle(FR_LEG_LIFT_CH,180-angle);
        setAngle(FR_LEG_MIDDLE_CH,180-angle);
        delay(10);
    }

    for (int angle = 90 ; angle >=0 ; angle--){
        setAngle(FL_LEG_ROTATION_CH,angle);
        setAngle(BL_LEG_ROTATION_CH,angle);
        setAngle(FR_LEG_ROTATION_CH,angle);
        setAngle(BR_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for( int angle = 180 ; angle >= 90 ; angle--){
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        setAngle(BL_LEG_LIFT_CH,180-angle);
        setAngle(BL_LEG_MIDDLE_CH,180-angle);
        setAngle(FR_LEG_LIFT_CH,180-angle);
        setAngle(FR_LEG_MIDDLE_CH,180-angle);
        delay(10);
    }

    for(int angle = 90 ; angle >= 50 ; angle--){//lifting
        setAngle(FR_LEG_LIFT_CH,angle);
        setAngle(FR_LEG_MIDDLE_CH,angle);
        delay(5);
    }

    for (int angle = 0 ; angle <= 90 ; angle++){//rotation
        setAngle(FR_LEG_ROTATION_CH,angle);
        delay(5);
    }

    for(int angle = 50 ; angle <=90 ; angle++){//dropping
        setAngle(FR_LEG_LIFT_CH,angle);
        setAngle(FR_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle >= 50 ; angle--){//lifting
        setAngle(BL_LEG_LIFT_CH,angle);
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for (int angle = 0 ; angle <= 90 ; angle++){//rotation
        setAngle(BL_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 50 ; angle <=90 ; angle++){//dropping
        setAngle(BL_LEG_LIFT_CH,angle);
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle <= 130 ; angle++){//lifting
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for (int angle = 0 ; angle <= 90 ; angle++){//rotation
        setAngle(FL_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 130 ; angle >= 90 ; angle--){//dropping
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle <= 130 ; angle++){//lifting
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(5);
    }

    for (int angle = 0 ; angle <= 90 ; angle++){
        setAngle(BR_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 130 ; angle >= 90 ; angle--){//dropping
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(5);
    }

}


void rotationRight(void){
    for( int angle = 90 ; angle <= 180 ; angle++){
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        setAngle(BL_LEG_LIFT_CH,180-angle);
        setAngle(BL_LEG_MIDDLE_CH,180-angle);
        setAngle(FR_LEG_LIFT_CH,180-angle);
        setAngle(FR_LEG_MIDDLE_CH,180-angle);
        delay(10);
    }

    for (int angle = 90 ; angle <= 180 ; angle++){
        setAngle(FL_LEG_ROTATION_CH,angle);
        setAngle(BL_LEG_ROTATION_CH,angle);
        setAngle(FR_LEG_ROTATION_CH,angle);
        setAngle(BR_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for( int angle = 180 ; angle >= 90 ; angle--){
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        setAngle(BL_LEG_LIFT_CH,180-angle);
        setAngle(BL_LEG_MIDDLE_CH,180-angle);
        setAngle(FR_LEG_LIFT_CH,180-angle);
        setAngle(FR_LEG_MIDDLE_CH,180-angle);
        delay(10);
    }

    for(int angle = 90 ; angle >= 50 ; angle--){//lifting
        setAngle(FR_LEG_LIFT_CH,angle);
        setAngle(FR_LEG_MIDDLE_CH,angle);
        delay(5);
    }

    for (int angle = 180 ; angle >= 90 ; angle--){//rotation
        setAngle(FR_LEG_ROTATION_CH,angle);
        delay(5);
    }

    for(int angle = 50 ; angle <=90 ; angle++){//dropping
        setAngle(FR_LEG_LIFT_CH,angle);
        setAngle(FR_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle >= 50 ; angle--){//lifting
        setAngle(BL_LEG_LIFT_CH,angle);
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for (int angle = 180 ; angle >= 90 ; angle--){//rotation
        setAngle(BL_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 50 ; angle <=90 ; angle++){//dropping
        setAngle(BL_LEG_LIFT_CH,angle);
        setAngle(BL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle <= 130 ; angle++){//lifting
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for (int angle = 180 ; angle >= 90 ; angle--){//rotation
        setAngle(FL_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 130 ; angle >= 90 ; angle--){//dropping
        setAngle(FL_LEG_LIFT_CH,angle);
        setAngle(FL_LEG_MIDDLE_CH,angle);
        delay(5);
    }


    for(int angle = 90 ; angle <= 130 ; angle++){//lifting
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(5);
    }

    for (int angle = 180 ; angle >= 90 ; angle--){
        setAngle(BR_LEG_ROTATION_CH,angle);
        delay(10);
    }

    for(int angle = 130 ; angle >= 90 ; angle--){//dropping
        setAngle(BR_LEG_LIFT_CH,angle);
        setAngle(BR_LEG_MIDDLE_CH,angle);
        delay(5);
    }


}

// --------------------- ARDUINO STANDARD FUNCTIONS ----------------------

void setup() {
    Serial.begin(9600); // Start serial communication
    pwm.begin();
    pwm.setPWMFreq(60); // Set PWM frequency to 60Hz
    delay(10); 

    Serial.println("Setting all servos to neutral stand position (90 degrees)...");
    setInitialPosition();
    delay(2000); // Wait 2 seconds in the initial position

    Serial.println("Setup complete. Starting Asymmetrical Tripod Gait.");
}

void loop() {
    //forward_efficient();
    //rotationLeft();
    rotationRight();

}