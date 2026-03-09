#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Initialize the PWM driver, default I2C address is 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// --------------------- FUNCTION PROTOTYPES (Forward Declarations) ----------------------
void setAngle(uint8_t channel, int angle);
void stallAllLegs();
void setInitialPosition();
void forward_efficient(); // The efficient walking function
void rightward_efficient();
void backward_efficient();
void leftward_efficient();

//------------------------5 seconds delay -----------------------------
// --- Global Variables ---
// Time reference for when the current action started
unsigned long previousMillis = 0;
// 10,000 milliseconds = 10 seconds
const long interval = 5000;
// State variable to track which function to run
int currentStep = 0;

// Function to stop all movement (you must define this)


// --------------------- SERVO PULSE CONFIGURATION ----------------------
// *** TUNE THESE VALUES FOR YOUR SERVOS ***
#define SERVOMIN 150  // Approx. 0 degrees 
#define SERVOMAX 600  // Approx. 180 degrees 
#define PULSE_90_DEGREE ((SERVOMIN + SERVOMAX) / 2)

// --------------------- EFFICIENT GAIT CONFIGURATION ----------------------
// These definitions are still global because they are motion parameters
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
// NOTE: Servo channels are defined locally here.
void stallAllLegs() {
    // --------------------- LOCAL CHANNEL DEFINITIONS ----------------------
    // This makes the function independent but requires all other
    // functions that use these channels to define them too.
    // --- Front Right (FR) Leg Servos ---
    uint8_t FR_LEG_LIFT_CH = 8;
    uint8_t FR_LEG_MIDDLE_CH = 9;
    uint8_t FR_LEG_ROTATION_CH = 10;

    // --- Front Left (FL) Leg Servos ---
    uint8_t FL_LEG_LIFT_CH = 5;
    uint8_t FL_LEG_MIDDLE_CH = 6;
    uint8_t FL_LEG_ROTATION_CH = 7;

    // --- Back Left (BL) Leg Servos ---
    uint8_t BL_LEG_LIFT_CH = 2;
    uint8_t BL_LEG_MIDDLE_CH = 3;
    uint8_t BL_LEG_ROTATION_CH = 4;

    // --- Back Right (BR) Leg Servos ---
    uint8_t BR_LEG_LIFT_CH = 11;
    uint8_t BR_LEG_MIDDLE_CH = 12;
    uint8_t BR_LEG_ROTATION_CH = 13;
    // --------------------------------------------------------------------


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
// This now works because stallAllLegs() has its own local definitions.
void setInitialPosition() {
    stallAllLegs();
}

// --------------------- EFFICIENT FORWARD MOVEMENT (TRIPOD GAIT) ----------------------

void forward_efficient() {

    // --------------------- LOCAL CHANNEL DEFINITIONS ----------------------
    // These variables only exist inside this function
    // --- Front Right (FR) Leg Servos ---
    uint8_t FR_LEG_LIFT_CH = 8;
    uint8_t FR_LEG_MIDDLE_CH = 9;
    uint8_t FR_LEG_ROTATION_CH = 10;

    // --- Front Left (FL) Leg Servos ---
    uint8_t FL_LEG_LIFT_CH = 5;
    uint8_t FL_LEG_MIDDLE_CH = 6;
    uint8_t FL_LEG_ROTATION_CH = 7;

    // --- Back Left (BL) Leg Servos ---
    uint8_t BL_LEG_LIFT_CH = 2;
    uint8_t BL_LEG_MIDDLE_CH = 3;
    uint8_t BL_LEG_ROTATION_CH = 4;

    // --- Back Right (BR) Leg Servos ---
    uint8_t BR_LEG_LIFT_CH = 11;
    uint8_t BR_LEG_MIDDLE_CH = 12;
    uint8_t BR_LEG_ROTATION_CH = 13;
    // --------------------------------------------------------------------


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
        
        // Apply angles to Moving Legs (FR, BL) - Using local variables
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

        // Apply angles to Moving Legs (FL, BR) - Using local variables
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


void rightward_efficient() {

    // --------------------- LOCAL CHANNEL DEFINITIONS ----------------------
    // These variables only exist inside this function
    // --- Front Right (FR) Leg Servos ---
    uint8_t FR_LEG_LIFT_CH = 2;
    uint8_t FR_LEG_MIDDLE_CH = 3;
    uint8_t FR_LEG_ROTATION_CH= 4;

    // --- Front Left (FL) Leg Servos ---
    uint8_t FL_LEG_LIFT_CH = 5;
    uint8_t FL_LEG_MIDDLE_CH = 6;
    uint8_t FL_LEG_ROTATION_CH = 7;

    // --- Back Left (BL) Leg Servos ---
    uint8_t BL_LEG_LIFT_CH = 8;
    uint8_t BL_LEG_MIDDLE_CH = 9;
    uint8_t BL_LEG_ROTATION_CH = 10;

    // --- Back Right (BR) Leg Servos ---
    uint8_t BR_LEG_LIFT_CH = 11;
    uint8_t BR_LEG_MIDDLE_CH = 12;
    uint8_t BR_LEG_ROTATION_CH = 13;
    // --------------------------------------------------------------------


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
        
        // Apply angles to Moving Legs (FR, BL) - Using local variables
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

        // Apply angles to Moving Legs (FL, BR) - Using local variables
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

void backward_efficient() {

    // --------------------- LOCAL CHANNEL DEFINITIONS ----------------------
    // These variables only exist inside this function
    // --- Front Right (FR) Leg Servos ---
    uint8_t FR_LEG_LIFT_CH = 11;
    uint8_t FR_LEG_MIDDLE_CH = 12;
    uint8_t FR_LEG_ROTATION_CH = 13;

    // --- Front Left (FL) Leg Servos ---
    uint8_t FL_LEG_LIFT_CH = 2;
    uint8_t FL_LEG_MIDDLE_CH = 3;
    uint8_t FL_LEG_ROTATION_CH = 4;

    // --- Back Left (BL) Leg Servos ---
    uint8_t BL_LEG_LIFT_CH = 5;
    uint8_t BL_LEG_MIDDLE_CH = 6;
    uint8_t BL_LEG_ROTATION_CH = 7;

    // --- Back Right (BR) Leg Servos ---
    uint8_t BR_LEG_LIFT_CH = 8;
    uint8_t BR_LEG_MIDDLE_CH = 9;
    uint8_t BR_LEG_ROTATION_CH = 10;
    // --------------------------------------------------------------------


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
        
        // Apply angles to Moving Legs (FR, BL) - Using local variables
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

        // Apply angles to Moving Legs (FL, BR) - Using local variables
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

void leftward_efficient() {

    // --------------------- LOCAL CHANNEL DEFINITIONS ----------------------
    // These variables only exist inside this function
    // --- Front Right (FR) Leg Servos ---
    uint8_t FR_LEG_LIFT_CH = 8;
    uint8_t FR_LEG_MIDDLE_CH = 9;
    uint8_t FR_LEG_ROTATION_CH = 10;

    // --- Front Left (FL) Leg Servos ---
    uint8_t FL_LEG_LIFT_CH = 11;
    uint8_t FL_LEG_MIDDLE_CH = 12;
    uint8_t FL_LEG_ROTATION_CH = 13;

    // --- Back Left (BL) Leg Servos ---
    uint8_t BL_LEG_LIFT_CH = 2;
    uint8_t BL_LEG_MIDDLE_CH = 3;
    uint8_t BL_LEG_ROTATION_CH = 4;

    // --- Back Right (BR) Leg Servos ---
    uint8_t BR_LEG_LIFT_CH = 5;
    uint8_t BR_LEG_MIDDLE_CH = 6;
    uint8_t BR_LEG_ROTATION_CH = 7;
    // --------------------------------------------------------------------


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
        
        // Apply angles to Moving Legs (FR, BL) - Using local variables
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

        // Apply angles to Moving Legs (FL, BR) - Using local variables
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

// --------------------- ARDUINO MAIN FUNCTIONS ----------------------

void setup() {
  // 1. Initialize the I2C driver
  pwm.begin();
  
  // 2. Set the PWM frequency (60Hz is standard for most hobby servos)
  pwm.setPWMFreq(60); 

  // 3. Set the robot to its initial, resting position.
  setInitialPosition(); 
  
  // NOTE: You can add a delay here to ensure the robot is stable before walking
  delay(1000); 
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if 10 seconds (the interval) have passed since the last transition
  if (currentMillis - previousMillis >= interval) {
    // Save the time of the current transition
    previousMillis = currentMillis;

    // First, stop the previous movement
    stallAllLegs();

    // Move to the next step
    currentStep++;
    // If we finished the last step (step 4), reset to step 0
    if (currentStep > 4) {
      currentStep = 1;
    }
  }

  // --- Run the appropriate function based on the current step ---
  switch (currentStep) {
    case 1:
      // Run step 1 for 10 seconds
      forward_efficient();
      break;
    case 2:
      // Run step 2 for 10 seconds
      rightward_efficient();
      break;
    case 3:
      // Run step 3 for 10 seconds
      backward_efficient();
      break;
    case 4:
      // Run step 4 for 10 seconds
      leftward_efficient();
      break;
    default:
      // Default state, ensures motors are stopped at start
      stallAllLegs();
      break;
  }

  // Other code (like reading sensors, checking buttons, etc.) can go here
  // and will run continuously because there is no 'delay()' blocking it.
}
