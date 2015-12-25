#ifndef __BIKE_MOVEMENTS_HPP
#define __BIKE_MOVEMENTS_HPP

#define ANGLE_OF_TURNS      2.0f
#define RATE_OF_ACCELERATE  0.005f
#define BIKE_SPEED_DEFAULT  0.4f
#define BIKE_SPEED_SLOWEST  0.2f
#define BIKE_SPEED_FASTEST  0.6f

//#define DEBUG
#ifdef DEBUG

// When you toggle a light trail off, don't fade away
// useful for debugging collisions
//#define DEBUG_STOP_TRAILS_FADING

// Show the light trail segments in different colours per segment
// red = straight, green = circle, blue = spiral
// this draws them based on the stored data not on the bike position
// useful for making sure the segments match the light trail
//#define DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS

// don't show the light trail. Only really useful in combination with
// DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS to simplify what you are seeing
// note you can still crash into the light trail
//#define DEBUG_HIDE_NORMAL_LIGHT_TRAIL

#if defined(DEBUG_STOP_TRAILS_FADING) && defined(DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS)
// Allow activating only one segment, the rest won't be shown
// and you can't collide with them. Useful for debugging collisions
//#define DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
#endif
#endif

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
