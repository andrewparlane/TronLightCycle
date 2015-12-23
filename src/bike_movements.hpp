#ifndef __BIKE_MOVEMENTS_HPP
#define __BIKE_MOVEMENTS_HPP

#define ANGLE_OF_TURNS      2.0f
#define RATE_OF_ACCELERATE  0.005f
#define BIKE_SPEED_DEFAULT  0.4f
#define BIKE_SPEED_SLOWEST  0.2f
#define BIKE_SPEED_FASTEST  0.6f

//#define DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
//#define DEBUG_HIDE_NORMAL_LIGHT_TRAIL

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
