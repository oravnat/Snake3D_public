//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

// Car related:
#define ROTATE_SPEED 3
#define MAX_SPEED 25.0
#define MIN_SPEED 5.0

#define CAR_FRONT_DISTANCE (1.0)
#define CAR_BACK_DISTANCE (1.0)
#define CAR_LENGTH (CAR_FRONT_DISTANCE + CAR_BACK_DISTANCE)

// Car speed:
#define DEFAULT_SPEED 25.0
#define TAIL_START_SIZE 4
#define CAR_DEFAULT_ACCELERATION 150.0

#define SHOOT_RATE 1000
#define TAIL_RATE 100

#define TAIL_ADDITION 10

// Radar related
#define RADAR_X 0.7f
#define RADAR_TOP_DISTANCE 0.3f
#define RADAR_SIZE 0.4f

// Game related:
#define SPHERE_RADIUS 60.0f

//#define M_PI       3.14159265358979323846
#define IS_KEY(key) ((game.m_keys[key] & 0x80) != 0)
#define MAX_TAIL 1000

#define PLANE_SPEED 100.0f
#define PLANE_HEIGHT 20.0f
#define BALL_SPEED 100.0f // meter/s

#define PLANE_ROTATE_SPEED 100 // degrees/second
#define NEW_BULLET_RATE 3000

#define GRAV (9.8)

#define GRID_SENSITIVITY 10
#define MAX_PLAYERS 4
#define DEFAULT_NUMBER_OF_PLAYERS 1
#define MAX_BALLS 10
#define MAX_ENTITIES 100
#define MAX_PACKETS 100
#define GAME_PORT 23232
#define PACKET_FLAG_YOUR_ENTITY 1
#define HIT_DISTANCE2 1.0 // 1 meter (1 = 1^2, for 5 meters this would be 25)
#define SMALL_HIT_DISTANCE2 0.64 // 0.8 meters (0.64 = 0.8^2)

#define SHIELD_SIZE 2.5f

// Points:
#define DEATH_POINTS -50
#define SHOT_KILL_POINTS 2
#define TAIL_KILL_POINTS 50
#define PRIZE_POINTS 10
#define BALL_HIT_PLAYER_POINTS 30

#define PRIZE_DOLLARS 10
#define DYMOND_DOLLARS 100
#define BALL_HIT_PLAYER_STEAL_DOLLARS 50

#define DEAD_TIME 3000
#define UNDEAD_TIME 3000
#define CAR_REJECTION_RADIUS 20.0
#define CAR_REJECTION_FORCE 4000.0

#define POINTS_PER_ROUND 500

#define MSG_WELCOME "Welcome to Snake3D"
#define MSG_LEVEL "Welcome to Level X"
#define MSG_BONUS "BONUS LEVEL !!!"
#define MSG_SELECT "Select Your Desired car !"
#define MSG_GAMEOVER "GAME OVER !"
#define MSG_STOLE "YOU STOLED 50 POINTS !"
#define MSG_QUIT "Are you sure want leave ?"

#define MSG_GOAL_EARN "Your Goal is to earn 1000 Points !"
#define MSG_GOAL_5MINS "Your Goal is to earn more than your enemys in 05:00 minutes !"
#define MSG_GOAL_FALL "Your Goal is to is not to fall !"
#define MSG_GOAL_KILL "Your Goal is to Kill your Enemy !"
#define MSG_GOAL_HELP "Your Goal help each other to climb to end of the map !"
