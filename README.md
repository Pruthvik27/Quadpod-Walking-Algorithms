# Quadpod-Walking-Algorithms
Forward walking implementations for a quadpod robot using predefined servo angle sequences.

Author: Pruthvik S

This repository contains experimental forward walking implementations for a quadruped robot (Quadpod).

The robot currently uses **hardcoded gait control**, where predefined servo angles are used to generate coordinated leg movements.

---

## Robot Configuration

- 4 Legs
- 2 Servos per leg
- Total Servos: 8

Walking is achieved by sending predefined angle sequences to the servos.

---

## Implemented Versions

- Forward Walking
- Forward Walking Perfect One
- Forward Walking Perfect One with Local Adjustments
- Forward Walking Prototype 2

Each version represents improvements in gait stability and leg coordination.

---

## Future Improvements

Future development will include **PD controller stabilization**.

Using sensor feedback (IMU), the robot will dynamically correct tilt and maintain balance during walking.

Expected improvements:

- Better stability
- Automatic tilt correction
- Improved terrain adaptation

---

## Important Note

Before using this code on your own quadpod robot, **verify the servo rotation directions and angle orientations**.

Different mechanical assemblies may result in reversed servo motion.  
Incorrect orientation can cause the legs to move in the wrong direction and may damage the robot.

Make sure to test the servo angles carefully and adjust the values if necessary.

---

## Future Improvements

The current implementation uses **hardcoded servo angles** to generate the walking gait.

The next stage of this project will focus on improving stability and adaptability.

Planned improvements include:

### PD Control Based Stabilization
A **Proportional–Derivative (PD) control system** will be integrated into the quadpod to dynamically adjust the servo angles.

Using sensor feedback (such as an IMU), the robot will be able to:

- Maintain balance while walking
- Correct body tilt
- Adapt to uneven terrain

### Terrain Adaptation
Future versions aim to allow the quadpod to **walk on different surfaces and terrains** by dynamically adjusting leg motion.

### Improved Walking Stability
Another major objective is to make the robot **walk without falling** while minimizing excessive body bending.

In the current hardcoded implementation, some body bending can be observed during walking.  
Future control algorithms will aim to **reduce this bending angle and maintain a more stable body posture**.
