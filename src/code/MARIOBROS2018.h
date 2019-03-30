// LIBRARIES
#include <util/delay.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <uzebox.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdBase.h>
#include <spiram.h>
// LIBRARIES

// USER CODE INCLUDES
#include "../assets/defineSR.def"             //
#include "../assets/bg_tiles_progmem.inc"     //
#include "../assets/sprite_tiles_progmem.inc" //
// USER CODE INCLUDES

// COMPILE STRINGS
const char COMPILEGAMENAME[]  PROGMEM = _COMPILEGAMENAME;
const char COMPILEDATETIME1[] PROGMEM = _COMPILEDATETIME1;
const char COMPILEDATETIME2[] PROGMEM = _COMPILEDATETIME2;
// COMPILE STRINGS

// DEFINES: SOUND FX PATCHES
// playSFX( PATCHNAME );
#define SND_GET_COIN        0 // Same sound
#define SND_KICK_ENEMY      1 // Same sound
#define SND_GET_BONUSWAFFER 2 // Same sound
#define SND_BUMPHEAD        3 // Used for head bump.
#define SND_PLAYER_MOVEMENT 4 // Player walking on ground
#define SND_PLAYER_JUMP     5 // NO SOUND
#define SND_PLAYER_SKID     6 // When player walking on ground AND out of xFrames and no player input
#define SND_PLAYER_DEFEATED 7 //
#define SND_PHASE_END       8 //
#define SND_FIRST_PHASE     9 // NO SOUND
// DEFINES: SOUND FX PATCHES

// DEFINES: NUMBER PRINTING
#define PRINT_PLAYER1SCORE 0
#define PRINT_PLAYER2SCORE 1
#define PRINT_PHASENUM     2
#define PRINT_PLAYER1LIVES 3
#define PRINT_PLAYER2LIVES 4
// DEFINES: NUMBER PRINTING

// DEFINES: ROWS START AND END
#define ROW0_START  0
#define ROW0_END    4
#define ROW1_START  5
#define ROW1_END    9
#define ROW2_START 10
#define ROW2_END   15
#define ROW3_START 16
#define ROW3_END   21
#define ROW4_START 22
#define ROW4_END   27
// DEFINES: ROWS START AND END

// DEFINES: PLAYER RESPAWN POINTS
#define MARIO_RESPAWN_X_TILE   6 * 8 // 10 * 8 == 80
#define LUIGI_RESPAWN_X_TILE  19 * 8 // 12 * 8 == 96
#define PLAYER_RESPAWN_Y_TILE 22 * 8 // 10 * 8 == 80
// DEFINES: PLAYER RESPAWN POINTS

// DEFINES: ENEMY SPAWN POINTS (X)
#define ENEMY_SPAWN_X_TILE_LEFT     4*8
#define ENEMY_SPAWN_X_TILE_RIGHT   19*8
// DEFINES: ENEMY SPAWN POINTS (X)

// DEFINES: FIREBALL SPAWN POINTS (X AND Y)
#define FIREBALL_SPAWN_X_TILE_LEFT  0
#define FIREBALL_SPAWN_X_TILE_RIGHT 23

#define FIREBALL_SPAWN_Y_TILE_ROW1  4
#define FIREBALL_SPAWN_Y_TILE_ROW2  10
// #define FIREBALL_SPAWN_Y_TILE_ROW3  11
#define FIREBALL_SPAWN_Y_TILE_ROW3  16
#define FIREBALL_SPAWN_Y_TILE_ROW4  22
// DEFINES: FIREBALL SPAWN POINTS (X AND Y)

// DEFINES: SCREEN TILE BOUNDARIES
#define BOUNDRY_LEFT_COL              0  * 8 // 1  * 8 == 8
#define BOUNDRY_RIGHT_COL             23 * 8 // 24 * 8 == 192
#define BOUNDRY_BOTTOM_LEFT_PIPE_COL  2  * 8 // 3  * 8 == 24
#define BOUNDRY_BOTTOM_RIGHT_PIPE_COL 21 * 8 // 22 * 8 == 176
// #define BOUNDRY_TOP_COL    0
// #define BOUNDRY_BOTTOM_COL 25
// DEFINES: SCREEN TILE BOUNDARIES

// DEFINES: ACTOR CHAR TYPES
#define noenemy        0
#define NOENEMY        0
#define MARIO          1
#define LUIGI          2
#define SHELLSCRAPER   3
#define SIDESTEPPER    4
#define FIGHTERFLY     5
#define SLIPICE        6
#define FIREBALL       7
#define BONUSWAFFER    8
#define COIN           9
#define SLOTAVAILABLE 10
// DEFINES: ACTOR CHAR TYPES

// DEFINES: GAME STATES
#define GSTATE_TITLE     1
#define GSTATE_PLAYING   2
#define GSTATE2_NORMAL   3
#define GSTATE2_COINROOM 4
// DEFINES: GAME STATES

// DEFINES: DIRECTION (USED FOR COLLISION DETECTION.)
#define D_UP    1
#define D_DOWN  2
#define D_LEFT  3
#define D_RIGHT 4
// DEFINES: DIRECTION (USED FOR COLLISION DETECTION.)

// DEFINES: X / Y DIRECTIONS (NAMED CONSTANTS.)
#define X_LEFT  0
#define X_RIGHT 1
#define Y_UP    0
#define Y_DOWN  1
#define Y_NONE  2
// DEFINES: X / Y DIRECTIONS (NAMED CONSTANTS.)

// DEFINES: SPEEDS
#define SPEED_100 100 //
#define SPEED_90  90  //
#define SPEED_80  80  //
#define SPEED_70  70  //
#define SPEED_60  60  //
#define SPEED_50  50  //
#define SPEED_40  40  //
#define SPEED_30  30  //
#define SPEED_20  20  //
#define SPEED_10  10  //
#define SPEED_00   0  //

#define SPEED_FIGHTERFLY_NORMAL 50  //
#define SPEED_FIGHTERFLY_FAST   35  //

#define ENEMY_SPEED_SLOW 1
#define ENEMY_NORMAL     2
#define ENEMY_MAX        3

#define DEFAULT_PLAYER_SPEED        3
#define DEFAULT_SPEED_COUNTER_LIMIT 3

#define MOVESPEED_STOP   1 // NAMED CONSTANTS
#define MOVESPEED_NORMAL 2 // NAMED CONSTANTS
#define MOVESPEED_FAST   3 // NAMED CONSTANTS
// DEFINES: SPEEDS

// DEFINES: POINTS.
#define ENEMY_KICK_POINTS   800
#define SLIPICE_BUMP_POINTS 500
#define SLIPICE_KICK_POINTS 800
#define COIN_POINTS         800
#define BONUSWAFFER_POINTS  800
// DEFINES: POINTS.

// DEFINES: TIMERS.
#define P_FRAME1_ANIM            8 // anim timer using vcounter8b_anim.
#define P_FRAME2_ANIM           16 // anim timer using vcounter8b_anim.
#define E_FRAME1_ANIM           16 // anim timer using vcounter8b_anim.
#define E_FRAME2_ANIM           32 // anim timer using vcounter8b_anim.

#define SHELLSCRAPER_F1_OB     250 // onBack timer using vcounter16b_general.
#define SHELLSCRAPER_F2_OB     350 // onBack timer using vcounter16b_general.
#define SHELLSCRAPER_F3_OB     500 // onBack timer using vcounter16b_general.

#define SIDESTEPPER_F1_OB      250 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F2_OB      400 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F3_OB      410 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F4_OB      420 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F5_OB      430 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F6_OB      440 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F7_OB      450 // onBack timer using vcounter16b_general.
#define SIDESTEPPER_F8_OB      500 // onBack timer using vcounter16b_general.

#define FIGHTERFLY_F1_OB       250 // onBack timer using vcounter16b_general.
#define FIGHTERFLY_F2_OB       500 // onBack timer using vcounter16b_general.

#define FIREBALL_F1_ANIM         8 // anim timer using vcounter8b_anim.
#define FIREBALL_F2_ANIM        16 // anim timer using vcounter8b_anim.
#define FIREBALL_F3_ANIM        24 // anim timer using vcounter8b_anim.
#define FIREBALL_F4_ANIM        32 // anim timer using vcounter8b_anim.
#define FIREBALL_DELAY          96 // delay timer using vcounter8b_general.

#define SLIPICE_F1_DELAY         8 // delay timer using vcounter8b_general.
#define SLIPICE_F2_DELAY       250 // delay timer using vcounter8b_general.
#define SLIPICE_F3_DELAY       375 // delay timer using vcounter8b_general.
#define SLIPICE_F4_DELAY       500 // delay timer using vcounter8b_general.

#define COINROOM_TWINKLE_DELAY  24 // delay timer using vsynccounter8b_gen1.
#define COINROOM_MAX_TIME      900 // delay timer using vsynccounter8b_gen1. (900/60==15 seconds)
// DEFINES: TIMERS.

// COUNTERS/TIMERS:
unsigned int vsynccounter16b_1 = 0; // 16-bit counters (0-65535)
uint8_t vsynccounter8b_gen1    = 0; // 8-bit counters (0-255)
uint8_t counter8b_gen1         = 0; // 8-bit value. Used for the coin rooms.
unsigned char vsyncCounter     = 0; // USED????
// COUNTERS/TIMERS:

// EXTERNAL GLOBALS
extern                       uint8_t ram_tiles[] ;
extern u8                    vram[]              ;
extern unsigned char         masterVolume        ;
extern volatile unsigned int joypad1_status_lo   ;
extern volatile unsigned int joypad2_status_lo   ;
extern unsigned char         free_tile_index     ;
extern unsigned char         masterVolume        ;
// EXTERNAL GLOBALS

// ENEMY DATABASE AND ACCESSING
// SHELLSCRAPER,SIDESTEPPER,FIGHTERFLY,SLIPICE,FIREBALL,BONUSWAFFER
const unsigned char phases_enemies[][8] = {
	// READ ONLY!
	// All of  the  levels  after  level  014  will  be repeats of one of the previous levels.

	// DEBUG: One of each enemy.
	// { SHELLSCRAPER , SIDESTEPPER  , FIGHTERFLY   , SLIPICE      , 0            , 0            , 0          , 0           , }, //

	// // DEBUG: Only 1 enemy.
	// { SHELLSCRAPER , 0            , 0            , 0            , 0            , 0            , 0          , 0           ,}, // Only 1 SLIPICE
	// { SIDESTEPPER  , 0            , 0            , 0            , 0            , 0            , 0          , 0           ,}, // Only 1 FIGHTERFLY
	// { FIGHTERFLY   , 0            , 0            , 0            , 0            , 0            , 0          , 0           ,}, // Only 1 SHELLSCRAPER
	// { SLIPICE      , 0            , 0            , 0            , 0            , 0            , 0          , 0           ,}, // Only 1 SIDESTEPPER

	// // DEBUG: All enemies the same type.
	// { SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , 0          , 0           , }, // All SHELLSCRAPER
	// { SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , 0          , 0           , }, // All SIDESTEPPER
	// { FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , 0          , 0           , }, // All FIGHTERFLY
	// { SLIPICE      , SLIPICE      , SLIPICE      , SLIPICE      , SLIPICE      , SLIPICE      , 0          , 0           , }, // All SLIPICE

	// (PRODUCTION) ENEMY QUEUES FOR EACH PHASE
	{ SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , 0            , 0            , 0           , 0           , 0           , }, // PHASE 1
	{ SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , 0           , 0           , 0           , }, // PHASE 2
	{ 0            , 0            , 0            , 0            , 0            , 0           , 0           , 0           , }, // PHASE 3 - COIN ROOM
	{ SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , 0            , 0           , 0           , 0           , }, // PHASE 4
	{ SHELLSCRAPER , SHELLSCRAPER , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER , 0           , 0           , }, // PHASE 5
	{ FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , 0            , 0           , 0           , 0           , }, // PHASE 6
	{ FIGHTERFLY   , FIGHTERFLY   , FIGHTERFLY   , SIDESTEPPER  , SIDESTEPPER  , 0           , 0           , 0           , }, // PHASE 7
	{ 0            , 0            , 0            , 0            , 0            , 0           , 0           , 0           , }, // PHASE 8 - COIN ROOM
	{ SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , FIGHTERFLY   , 0           , 0           , 0           , }, // PHASE 9  (Slipice)
	{ SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , FIGHTERFLY   , 0           , 0           , 0           , }, // PHASE 10 (Slipice)
	{ SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , FIGHTERFLY   , FIGHTERFLY  , 0           , 0           , }, // PHASE 11 (Slipice)
	{ SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , SIDESTEPPER  , FIGHTERFLY   , 0           , 0           , 0           , }, // PHASE 12 (Slipice)
	{ 0            , 0            , 0            , 0            , 0            , 0           , 0           , 0           , }, // PHASE 13 - COIN ROOM
	{ SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SHELLSCRAPER , SIDESTEPPER  , SIDESTEPPER , SIDESTEPPER , SIDESTEPPER , }, // PHASE 14

};
// Local enemy queue. Enemies for the current phase are initially copied into the local queue.
unsigned char phases_enemies_queue[8] = { };
// Tracks the current position in the local queue.
unsigned char phases_enemies_queue_index = 0;
// ENEMY DATABASE AND ACCESSING

// GAME DATA STRUCTURE
struct game_ {
	unsigned char gamestate1;       // Game state. Title, playing, game over.
	unsigned char gamestate2;       // Secondary game state. Coin room, normal game.
	unsigned char numPlayers;       // 1=One player, 2=Two player.
	unsigned char hardMode;        	// 0=no fireballs, 1=fireballs.
	unsigned char uzenet;          	// 0=no (local only), 1=Use Uzenet. (Player 2 controls come from Uzenet.)
	unsigned char phaseNum;        	// Level counter.
	unsigned char displayedPhaseNum;// Level counter.
	unsigned char player1Lives;     // Lives counter for player 1.
	unsigned char player2Lives;     // Lives counter for player 2.
	unsigned long player1Score;     // Score for player 1.
	unsigned long player2Score;     // Score for player 2.

	// Stores the game pad state: player 1
	unsigned int btnHeld1;
	unsigned int btnPressed1;
	unsigned int btnPrev1;
	unsigned int btnReleased1;

	// Stores the game pad state: player 2
	unsigned int btnHeld2;
	unsigned int btnPressed2;
	unsigned int btnPrev2;
	unsigned int btnReleased2;
	unsigned char row1Frozen;   // Frozen row value (row1).
	unsigned char row2Frozen;   // Frozen row value (row2).
	unsigned char row3Frozen;   // Frozen row value (row3).
	unsigned char POW_state;    // POW block state.
								// 0 = Unavailable,
								// 1 = Gray       ,
								// 2 = Orange     ,
								// 3 = Yellow     ,
	// unsigned char nextActorSlot;
};
struct game_ game;
// GAME DATA STRUCTURE

// ACTOR SYSTEM AND DATA STRUCTURE
struct actor_ {
	unsigned char charType;            // Shellscraper, Sidestepper, Fighterfly, Slipice, Fireball.
	unsigned char vcounter8b_anim;     // Used to time each displayed frame.
	unsigned char vcounter8b_general;  //
	unsigned char vcounter8b_general2;  // Used for FIGHTERFLY.
	unsigned int  vcounter16b_general; //

	unsigned char movespeed;           // Used for determining if the actor is gifted another speed 'tick'.
	unsigned char speed;               // Used by players and enemies. Add this to the speed_counter.
	unsigned int  speed_counter;        // Used by players and enemies. When the max value is reached then a pixel movement is allowed.

	// unsigned char isIdle;              // <UNUSED> 0=no, 1=yes. (only used by mario, luigi, FighterFly, and fireball.)
	unsigned char x;                   // X position ( (0,0) is upper left.)
	unsigned char y;                   // Y position ( (0,0) is upper left.)
	unsigned char xDir;                // 0=left, 1=right.
	unsigned char yDir;                // 0=up, 1=down, 2=neither jumping up or falling down.
	unsigned char xFramesRemaining;    // Each movement consists of a movement of 8 pixels. This is how many remain in the current movement.
	unsigned char yFramesRemaining;    // Each movement consists of a movement of 8 pixels. This is how many remain in the current movement.
	unsigned char isDefeated;          // Indicates that the enemy is no longer in play.
	unsigned char isOnScreen;          // Indicates that the actor is currently on screen.
	unsigned char isEnemy;             // Indicates that the actor is an enemy.
	unsigned char whichRow;            // What row is actor on?  0=Initial fall from pipe, 1=top row, 2=second from top, row3=third from top, row4=bottom row.
	unsigned char onBack;              // Indicates the need for SPRITE_FLIP_Y.
	unsigned char isMoving;            // Used by MARIO/LUIGI for skidding, FIREBALL for movement delay. 0=no, 1=yes.
	unsigned char isStopping;          // Used by MARIO/LUIGI. Used to indicate skidding. FIREBALL for adding new xFrames. 0=no, 1=yes.
	unsigned char width;               // All actors have the same width. This is here just in case.
	unsigned char height;              // How many tiles high is the actor?
};

// Slots
// Player 1 and Player 2      : 2 slots
// Max three onscreen enemies : 3 slots
// Max one fireball           : 1 slot
// One extra slot             : 1 slot
// ------------------------------------
//                      TOTAL : 7 slots (21 bytes times 7 = 147)

// 21 * 10 = 210				// Old way
// 21 * 7  = 147				// New way
// DIFF:     63 				// RAM recovered

#define NUM_ACTORS 7
struct actor_ actors[NUM_ACTORS]; //
// ACTOR SYSTEM AND DATA STRUCTURE

// PROTOTYPES
#include "mb_prototypes.c"
// PROTOTYPES