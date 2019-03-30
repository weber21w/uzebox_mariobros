// Function declarations.

// NEW - still defined in MARIOBROS2018.h
void N782_print(int x,int y,const char *string);
unsigned char collision_PlayerVsActor(unsigned char i);
unsigned char collision_PlayerVsActor_bumpCheck(unsigned char i);
unsigned char getEnemyCount_inLocalQueue();
void playSFX(unsigned char patch);
unsigned char gstate2_coinRoom_getCoin(unsigned char i, unsigned char tile_id, unsigned char checkedX, unsigned char checkedY);
// NEW - still defined in MARIOBROS2018.h

// LOW LEVEL
void pre_VsyncCallBack();
void post_VsyncCallBack();
void updateIndividualTimers();
void getInputs();
void gameInit();
void newGameInit();
// LOW LEVEL

// DEBUG/UTILITIES
static void _emu_whisper(int port, unsigned char val);
void debug_displayQueue();
void printNumber(unsigned char slot, unsigned long value);
void debug_displayRowFreezeState();
// DEBUG/UTILITIES

// COLLISION DETECTION
unsigned char collisionDetect_sprite2Background(unsigned char i, unsigned char dir);
unsigned char collisionDetect_sprite2Sprite(unsigned char actor1, unsigned char actor2);
// COLLISION DETECTION

// ACTOR CONTROL
unsigned char findOpenActorSlot();
void hideActor(unsigned char i);
void removeActor(unsigned char i);
void spawnActor( unsigned char charType );
void gameInit_actorSystem();
// ACTOR CONTROL

// ACTOR POSITION ADJUSTMENTS
void moveEnemies();
void movePlayers();
void moveFireball();
void redrawAllActorSprites();
// ACTOR POSITION ADJUSTMENTS

// GAME CONTROL
void newPhase(unsigned char phaseNum);
void setRowType(unsigned char row, unsigned char type);
void drawPow( unsigned char POW_state );
void powBlockHit();
void bumpEnemy(unsigned char i);
void assignPoints(unsigned char i, unsigned int newPoints);
void twinkleCoins();
void coinsShow(unsigned char showThem);
unsigned char checkForEndOfPhase();
// GAME CONTROL

// GAME STATES (game.gamestate1)
void gstate_title();
void gstate_playing();
// GAME STATES (game.gamestate1)

// GAME STATES 2 (game.gamestate2)
void gstate2_normal();

void gstate2_coinRoom();
// GAME STATES 2 (game.gamestate2)

// MAIN
int main();
// MAIN

	// __asm__ __volatile__ ("wdr");
	// __asm__ __volatile__ ("wdr");