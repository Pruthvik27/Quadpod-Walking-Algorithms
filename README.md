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
