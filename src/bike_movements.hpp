#ifndef __BIKE_MOVEMENTS_HPP
#define __BIKE_MOVEMENTS_HPP

#define ANGLE_OF_TURNS  2.0f

enum TurnDirection
{
    NO_TURN = 0,
    TURN_LEFT,
    TURN_RIGHT
};

enum Accelerating
{
    SPEED_NORMAL = 0,
    SPEED_ACCELERATE,
    SPEED_BRAKE
};

#endif
