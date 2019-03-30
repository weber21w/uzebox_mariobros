#include "MARIOBROS2018.h"

// Kernel processing
// 1. Process user pre callback      // Update the sprites[] array.
//
// 2. Invoke video mode tasks        // Does the actual screen drawing.
// 3. Refresh buttons/mouse state    // This is handled directly via the program, NOT the kernel.
// 4. Process music (music & mixing) // No music yet.
//
// 5. Process user post callback     // Update counters.

// LOW LEVEL
void pre_VsyncCallBack(){
	redrawAllActorSprites();
}
void updateIndividualTimers(){
	// Y, 0=up, 1=down, 2=neither

	// General shared timers.

	// 16-bit -
	// Used in the coin room for the countdown timer.
	// Used by the title screen to time the logo color changes.
	vsynccounter16b_1++   ;

	// 8-bit -
	// Used by the title screen to toggle the menu cursor tiles.
	vsynccounter8b_gen1++ ;

	// Individual actor timers.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		// if(actors[i].isDefeated == 0){
		if(actors[i].isOnScreen == 1){
			// 8-bit -
			// Timer used for determining the next animation frame for an actor.

			// Change
			if(actors[i].speed==ENEMY_MAX ){
				// These actors need different animation speeds.
				if     (actors[i].charType==SHELLSCRAPER){ actors[i].vcounter8b_anim+=2; } // Faster.
				else if(actors[i].charType==SIDESTEPPER) { actors[i].vcounter8b_anim+=3; } // Noticeably faster.
				else if(actors[i].charType==FIGHTERFLY)  { actors[i].vcounter8b_anim+=1; } // A little faster.
				// The rest of the actors have the same animation speed.
				else                                     { actors[i].vcounter8b_anim+=1; }
			}
			else{ actors[i].vcounter8b_anim+=1; }

			// 8-bit -
			// Used by FIREBALL for delay between x/y warps.
			// (FORMERLY) speed control. Used by SHELLSCRAPER, SIDESTEPPER, SLIPICE, FIGHTERFLY to determine when an xframe can be consumed.
			actors[i].vcounter8b_general++;

			// 8-bit - Used for a jump delay with FIGHTERFLY
			actors[i].vcounter8b_general2++;

			// 16-bit -
			// Used by SHELLSCRAPER, SIDESTEPPER, SLIPICE, FIGHTERFLY to keep track of the enemy onBack and recovery phases.
			// Also used by SLIPICE to manage the melt frames.
			// When NOT onBack and not SLIPICE melting, it is used as a timer for speed.
			// if( actors[i].onBack
			actors[i].vcounter16b_general++;

		}
	}
}
void post_VsyncCallBack(){
	updateIndividualTimers();
}

void getInputs(){
	// Get local inputs.
	ReadControllers();

	// If using Uzenet then the host is player 1 and the client is player 2.
	if(game.uzenet){
		// Receive gamepad states from Uzenet for player 2.
		//

		// If this is the Uzenet host then make them player 1.
		//

		// If this is the Uzenet client then make them player 2.
		//
	}
	else{
		// Gamepad #1
		game.btnPrev1     = game.btnHeld1;
		game.btnHeld1     = joypad1_status_lo;
		game.btnPressed1  = game.btnHeld1 & (game.btnHeld1 ^ game.btnPrev1);
		game.btnReleased1 = game.btnPrev1 & (game.btnHeld1 ^ game.btnPrev1);

		// Gamepad #2
		game.btnPrev2     = game.btnHeld2;
		game.btnHeld2     = joypad2_status_lo;
		game.btnPressed2  = game.btnHeld2 & (game.btnHeld2 ^ game.btnPrev2);
		game.btnReleased2 = game.btnPrev2 & (game.btnHeld2 ^ game.btnPrev2);

		// DEBUG TESTING! Give P2 the same inputs as P1.
		// game.btnPrev2     = game.btnHeld1;
		// game.btnHeld2     = joypad1_status_lo;
		// game.btnPressed2  = game.btnHeld1 & (game.btnHeld1 ^ game.btnPrev1);
		// game.btnReleased2 = game.btnPrev1 & (game.btnHeld1 ^ game.btnPrev1);
	}
}
void gameInit(){
	// System init.
	SetTileTable( bg_tiles );
	ClearVram();
	SetSpritesTileBank(0 , sprite_tiles   ); // 0 -- SPRITE_BANK0
	SetSpritesTileBank(1 , bg_tiles   );     // 1 -- SPRITE_BANK1

	SetMasterVolume(40);

	// InitMusicPlayer(YourPatches);

	// while(true){
	// TriggerFx(YOUR_PATCH_NUM, 0xFF, true);
	// WaitVsync(60);
	// }

	// Init the inital game settings.
	game.gamestate1   = GSTATE_TITLE ;   // Game state. Title, playing, game over.
	game.gamestate2   = GSTATE2_NORMAL ; // Secondary game state. Coin room, normal game.
	game.uzenet       = 0 ; //

	// Init the rest of the game settings.
	newGameInit();

		// Set the pre/post vsync callbacks.
	SetUserPreVsyncCallback ( &pre_VsyncCallBack  ); // Redraws sprites.
	SetUserPostVsyncCallback( &post_VsyncCallBack ); // Updates counters.

	// Show the compile date.
	WaitVsync(50);
	N782_print( 0,  1, COMPILEGAMENAME ) ;
	N782_print( 1,  2, COMPILEDATETIME1 ) ;
	N782_print( 4, 2, COMPILEDATETIME2 ) ;
	WaitVsync(50);
	ClearVram();
}
void newGameInit(){
	// Used after the initial init for games that have already had their init.

	// Init the game settings.
	game.phaseNum     = 0 ; // Level counter.
	game.row1Frozen   = 0 ; // 0=not frozen, 1=frozen.
	game.row2Frozen   = 0 ; // 0=not frozen, 1=frozen.
	game.row3Frozen   = 0 ; // 0=not frozen, 1=frozen.
	game.POW_state    = 3 ;
	game.player1Score = 0;
	game.player2Score = 0;

	// Reset data within the sprites array and the actor system.
	gameInit_actorSystem();
}
// LOW LEVEL

// DEBUG
static void _emu_whisper(int port, unsigned char val) {
	if(port==0x39 || port == 0){ u8 volatile * const _whisper_pointer1 = (u8 *) 0x39; *_whisper_pointer1 = val; }
	if(port==0x3A || port == 1){ u8 volatile * const _whisper_pointer2 = (u8 *) 0x3A; *_whisper_pointer2 = val; }
}
void debug_displayQueue(){
	// SHELLSCRAPER,SIDESTEPPER,FIGHTERFLY,SLIPICE,FIREBALL,BONUSWAFFER

	unsigned char cnt_SHELLSCRAPER = 0 ;
	unsigned char cnt_SIDESTEPPER  = 0 ;
	unsigned char cnt_FIGHTERFLY   = 0 ;
	unsigned char cnt_SLIPICE      = 0 ;
	// unsigned char cnt_BONUSWAFFER  = 0 ;
	// unsigned char cnt_NOENEMY      = 0 ;

	unsigned char cnt_SHELLSCRAPER2 = 0 ;
	unsigned char cnt_SIDESTEPPER2  = 0 ;
	unsigned char cnt_FIGHTERFLY2   = 0 ;
	unsigned char cnt_SLIPICE2      = 0 ;
	// unsigned char cnt_BONUSWAFFER2  = 0 ;
	// unsigned char cnt_NOENEMY2      = 0 ;

	// Set the font index.
	unsigned char font_index = pgm_read_byte(&(yellowS0[2])) ;

	// Count each type.
	for(unsigned char i=0; i<sizeof(phases_enemies_queue); i++){
		// Local queue count.
		if(phases_enemies_queue[i] == SHELLSCRAPER){ cnt_SHELLSCRAPER ++ ; }
		if(phases_enemies_queue[i] == SIDESTEPPER) { cnt_SIDESTEPPER  ++ ; }
		if(phases_enemies_queue[i] == FIGHTERFLY)  { cnt_FIGHTERFLY   ++ ; }
		if(phases_enemies_queue[i] == SLIPICE)     { cnt_SLIPICE      ++ ; }
		// if(phases_enemies_queue[i] == BONUSWAFFER) { cnt_BONUSWAFFER  ++ ; }
		// if(phases_enemies_queue[i] == NOENEMY)     { cnt_NOENEMY      ++ ; }

		// Global queue (const)
		if(phases_enemies[game.phaseNum][i] == SHELLSCRAPER){ cnt_SHELLSCRAPER2 ++ ; }
		if(phases_enemies[game.phaseNum][i] == SIDESTEPPER) { cnt_SIDESTEPPER2  ++ ; }
		if(phases_enemies[game.phaseNum][i] == FIGHTERFLY)  { cnt_FIGHTERFLY2   ++ ; }
		if(phases_enemies[game.phaseNum][i] == SLIPICE)     { cnt_SLIPICE2      ++ ; }
		// if(phases_enemies[game.phaseNum][i] == BONUSWAFFER) { cnt_BONUSWAFFER2  ++ ; }
		// if(phases_enemies[game.phaseNum][i] == NOENEMY)     { cnt_NOENEMY2      ++ ; }
	}

	// Display the count of each type.
	SetTile(1+7, 27, (cnt_SHELLSCRAPER %10) + font_index);
	SetTile(1+8, 27, (cnt_SIDESTEPPER  %10) + font_index);
	SetTile(1+9, 27, (cnt_FIGHTERFLY   %10) + font_index);
	SetTile(1+10, 27, (cnt_SLIPICE      %10) + font_index);
	// SetTile(4+7, 27, (cnt_BONUSWAFFER  %10) + font_index);
	// SetTile(9  , 26, (cnt_NOENEMY      %10) + font_index);

	SetTile(20-6, 27, (cnt_SHELLSCRAPER2 %10) + font_index);
	SetTile(21-6, 27, (cnt_SIDESTEPPER2  %10) + font_index);
	SetTile(22-6, 27, (cnt_FIGHTERFLY2   %10) + font_index);
	SetTile(23-6, 27, (cnt_SLIPICE2      %10) + font_index);
	// SetTile(24-6, 27, (cnt_BONUSWAFFER2  %10) + font_index);
	// SetTile(16  , 26, (cnt_NOENEMY2      %10) + font_index);

	// Current index in the queue.
	// SetTile(12, 0, (phases_enemies_queue_index  %10) + font_index);
}
void debug_displayRowFreezeState(){
	// Place a text 0 or 1 in the column to the left of each freezable row.

	// Set the font index.
	unsigned char font_index = pgm_read_byte(&(yellowS0[2])) ;

	SetTile(0, 7,  (game.row1Frozen) + font_index);
	SetTile(0, 13, (game.row2Frozen) + font_index);
	SetTile(0, 19, (game.row3Frozen) + font_index);

}
// DEBUG

// UTILITIES
void printNumber(unsigned char slot, unsigned long value){
	unsigned char font_index ;
	uint8_t startx ;
	uint8_t starty ;

	// Points for either player 1 or player 2
	if     (slot==PRINT_PLAYER1SCORE || slot==PRINT_PLAYER2SCORE){
		// P1 points (blue)
		if(slot==PRINT_PLAYER1SCORE){
			font_index = pgm_read_byte(&(blueS0[2]))   ;
			startx=0;
			starty=27;
		}
		// P2 points (green)
		else if (slot==PRINT_PLAYER2SCORE){
			font_index = pgm_read_byte(&(greenS0[2]))  ;
			startx=18;
			starty=27;
		}

		u8 cols[6] = {0};
		cols[0] = ( (value/100000) +0 ) ; value %= 100000;
		cols[1] = ( (value/10000)  +0 ) ; value %= 10000;
		cols[2] = ( (value/1000)   +0 ) ; value %= 1000;
		cols[3] = ( (value/100)    +0 ) ; value %= 100;
		cols[4] = ( (value/10)     +0 ) ; value %= 10;
		cols[5] = ( (value/1)      +0 ) ; value %= 1;

		for(uint8_t i=0; i<6; i++){ SetTile(startx+i, starty, cols[i]+font_index); }
	}

	// Phase number (yellow).
	else if(slot==PRINT_PHASENUM){
		font_index = pgm_read_byte(&(yellowS0[2])) ;
		SetTile(11, 26, (value /10) + font_index); // tensDigit;
		SetTile(12, 26, (value %10) + font_index); // onessDigit;
	}

	// Player 1 lives counter.
	else if(slot==PRINT_PLAYER1LIVES){
		// value+=1;
		font_index = pgm_read_byte(&(blueS0[2])) ;
		SetTile(4, 26, (value /10) + font_index); // tensDigit;
		SetTile(5, 26, (value %10) + font_index); // onessDigit;
	}

	// Player 2 lives counter.
	else if(slot==PRINT_PLAYER2LIVES){
		// value+=1;
		font_index = pgm_read_byte(&(greenS0[2])) ;
		SetTile(22, 26, (value /10) + font_index); // tensDigit;
		SetTile(23, 26, (value %10) + font_index); // onessDigit;
	}
}
void N782_print(int x,int y,const char *string){
	// This works like the kernel print function but does not require a complete fontset.
	// Usually your fontset will have at least all capital letters, numbers.
	// Punctuation characters can be added as needed.

	// A string will be passed along with the start x and y pos for it to be printed to.

	int stringPos=0;
	char thischar;
	unsigned char charAsTileNum;
	unsigned char unknownFont = pgm_read_byte(&(offBlackTile[2]));

	// Keep printing the string until the string terminator (NULL) is found.
	while(1){
		// Read the next character in the string.
		thischar=pgm_read_byte(&(string[stringPos++]));

		// Is this character a NULL?
		if(thischar!=0){
			// No? Okay, is it a character that we have? If so, get the tile for that character.
			// If we have the character in our fontset then print it.

			// Uppercase.
			if     (thischar >= 'A' && thischar <= 'Z') { charAsTileNum = 16 + thischar-65; } // 16 is the tile index of A. 65 is the ASCII value of A.

			// Force lowercase to uppercase.
			else if(thischar >= 'a' && thischar <= 'z') { charAsTileNum = 16 + thischar-97; } // 16 is the tile index of A. 97 is the ASCII value of a.

			// Numbers.
			else if(thischar >= '0' && thischar <= '9') { charAsTileNum = 4  + thischar-48; } // 4 is the tile index of 0. 48 is the ASCII value of 0.

			// Punctuation.
			else if( thischar==' ' )                    { charAsTileNum = 0 ; }
			else if( thischar=='!' )                    { charAsTileNum = 1 ; }
			else if( thischar=='-' )                    { charAsTileNum = 2 ; }
			else if( thischar=='.' )                    { charAsTileNum = 3 ; }
			else if( thischar==':' )                    { charAsTileNum = 14 ; }
			else if( thischar=='=' )                    { charAsTileNum = 15 ; }

			// Unmatched character. Use the offBlackTile tile.
			else                                        { charAsTileNum = unknownFont+1 ;  }

			// Print the character to the screen.
			SetTile(x++,y+0 , charAsTileNum);
		}

		// Yes? The string is over.
		else{ break; }
	}

}
unsigned char getEnemyCount_inLocalQueue(){
	unsigned char enemyCount=false;
	for(unsigned char ii=0; ii < sizeof(phases_enemies_queue); ii++){
		if(phases_enemies_queue[ii] != NOENEMY){ enemyCount++; }
	}

	return enemyCount;
}
void playSFX(unsigned char patch){
	unsigned char volume = 128  ;
	bool retrig          = true ;

	// switch(phase){
	// 	case SND_GET_COIN        : { break ; }
	// 	case SND_BUMPHEAD        : { break ; }
	// 	case SND_PLAYER_MOVEMENT : { break ; }
	// 	case SND_PLAYER_JUMP     : { break ; }
	// 	case SND_PLAYER_SKID     : { break ; }
	// 	case SND_PLAYER_DEFEATED : { break ; }
	// 	case SND_KICK_ENEMY      : { break ; }
	// 	case SND_GET_BONUSWAFFER : { break ; }
	// 	case SND_FIRST_PHASE     : { break ; }
	// 	case SND_PHASE_END       : { break ; }
	// 	default                  : { break ; }
	// };

	// TriggerFx(unsigned char patch,unsigned char volume, bool retrig);
	// TriggerFx(patch,  volume, retrig);
}
// UTILITIES

// COLLISION DETECTION
unsigned char collisionDetect_sprite2Background(unsigned char i, unsigned char dir){
	// SOLID TILES

	unsigned char blacktile = pgm_read_byte(&(blackTile[2]));
	unsigned char platform1 = pgm_read_byte(&(platform_noIce[2]));
	unsigned char platform2 = pgm_read_byte(&(platform_ice[2]));
	unsigned char platform3 = pgm_read_byte(&(platform_floor[2]));
	unsigned char pipe      = pgm_read_byte(&(pipe_tile[2]));

	// SHELLSCRAPER,SIDESTEPPER,FIGHTERFLY,SLIPICE,FIREBALL,BONUSWAFFER
	//
	// COLLISION DETECTION: SPRITE MAP VS TILES
	//

	// IDEA: If an actor is actually overlapping a tile then the actor should be pushed one tile away opposite of their current direction.
	//       This could help to solve a problem when moving both x and y.

	//
	// HOW IT SHOULD WORK
	//
	// A return of a 1 means no collision.
	// A return of a 0 means collision.
	// All checked tiles need to be non-solid. Only passable tile is tile 0 (or the green pipe.)
	// All actors are expected to be 1 tile wide.
	// Actors can have individual heights.

	//
	// LOCAL VARIABLES
	//
	// unsigned char w                = actors[i].width  ;           //
	unsigned char h                = actors[i].height ;           //
	unsigned char x                = actors[i].x >> 3 ;           // Get the tile x coord from the sprite x pos.
	unsigned char y                = actors[i].y >> 3 ;           // Get the tile y coord from the sprite y pos.
	unsigned char xDir             = actors[i].xDir ;             // Get the direction from the actor.
	// unsigned char yDir             = actors[i].yDir ;             // Get the direction from the actor.
	unsigned char xFramesRemaining = actors[i].xFramesRemaining ; //

	unsigned char tile_id      ;
	unsigned char is_collision ;
	unsigned char checkedX;
	unsigned char checkedY;
	// unsigned char isCoin=false;
	unsigned char coinFrame1 = pgm_read_byte(&(TcoinF1[2]));
	unsigned char coinFrame2 = pgm_read_byte(&(TcoinF2[2]));
	unsigned char tile_pow3 = pgm_read_byte(&(pow3[2])); // 6 ; // Orange
	unsigned char tile_pow2 = pgm_read_byte(&(pow2[2])); // 7 ; // Yellow
	unsigned char tile_pow1 = pgm_read_byte(&(pow1[2])); // 8 ; // Gray

	//
	// COLLISION OVERRIDES
	//
	// DEBUG: By-pass collision detection by always returning no collision.
	if( game.btnHeld1 & BTN_SR ) { return 1; }

	// Don't allow a fall from the floor (sounds silly, I know.)
	// if( (y + h) >= VRAM_TILES_V-4 && dir==D_DOWN){ return 0; }

	// Don't allow a jump beyond the top of the screen.
	// if( y == 0 && dir==D_UP){ return 0; }
	if( actors[i].y == 0 && dir==D_UP){ return 0; }

	if     (dir==D_UP)    {
		// Get the tile_id.
		// Check if the actor is currently moving left/right.
		if( xFramesRemaining ){
			if     ( xDir == 0 ){ checkedX=x;   checkedY=y; }// Moving left.
			else if( xDir == 1 ){ checkedX=x+1; checkedY=y; }// Moving right.
		}
		else                    { checkedX=x;   checkedY=y; }// Not moving on X.

		// Get the tile.
		tile_id=GetTile(checkedX, checkedY);
		// Is it a collision?
		is_collision = tile_id == 0x00 ? 0 : 1;

		// We have the tile_id and we know if we had a collision.

		// Did we have a collision?
		if(is_collision==1){

			// Was the collision with a platform?
			if( tile_id == platform1 || tile_id == platform2 || tile_id == platform3 || tile_id == pipe ) {
				// Immediately change the actor yDir to falling.
				// actors[i].yDir = 1;
				// Give the actor some yFrames.
				// actors[i].yFramesRemaining=8;
				return 0;
			}
			// Was the collision with the POW block?
			else if(
				(tile_id == tile_pow3 || tile_id == tile_pow2 || tile_id == tile_pow1)
				&& actors[i].yDir == Y_UP
			) {
				// Immediately change the actor yDir to falling.
				actors[i].yDir = 1;
				// Give the actor some yFrames.
				actors[i].yFramesRemaining=8;
				// Activate the POW block.
				powBlockHit();
			}

			// Was it a coin and are we checking for coin?
			else{
				if(game.gamestate2==GSTATE2_COINROOM) {
					// Determine if the sprite is overlapping the coin background tiles.
					return gstate2_coinRoom_getCoin(i, tile_id, checkedX, checkedY);
				}
			}

			return 0;
		}


	}
	else if(dir==D_DOWN)  {
		// Check if the actor is currently moving left/right.
		if(xFramesRemaining ){
			if     ( xDir == 0 ){ checkedX=x;   checkedY=y+(h); }// Moving left.
			else if( xDir == 1 ){ checkedX=x+1; checkedY=y+(h); }// Moving right.
		}
		else                    { checkedX=x;   checkedY=y+(h); } // Not moving on X.

		// Get the tile.
		tile_id=GetTile(checkedX, checkedY);
		// Is it a collision?
		is_collision = tile_id == 0x00 ? 0 : 1;

		// Did we have a collision?
		if(is_collision==1){
			// Was the collision with a platform?
			if( tile_id == platform1 || tile_id == platform2 || tile_id == platform3 || tile_id == pipe ) {
				return 0;
			}

			// If this is a coin room then check for collision with a coin.
			else if(game.gamestate2==GSTATE2_COINROOM) {
				return gstate2_coinRoom_getCoin(i, tile_id, checkedX, checkedY);
			}

			// Otherwise just return the collision.
			// else{
			// 	return 0;
			// }
		}

	}

	else if(dir==D_LEFT || dir==D_RIGHT){
		// What if x == BOUNDRY_LEFT_COL ?
		// What if x == BOUNDRY_RIGHT_COL ?

		// Collision check specific to Horizontal boundaries.
		// If checking against a boundry return no collision.
		if(dir==D_LEFT  && x==BOUNDRY_LEFT_COL>>3 ) { return 1; }
		if(dir==D_RIGHT && x==BOUNDRY_RIGHT_COL>>3) { return 1; }

		// Normal collision check.
		if(dir==D_LEFT)  { checkedX=x-1; }
		if(dir==D_RIGHT) { checkedX=x+1; }
		checkedY=y;

		// Check a column h number of tiles high to the immediate left/right tile of the actor.
		for(unsigned char h_it=0; h_it<h; h_it++){
			// Get the tile.
			tile_id=GetTile(checkedX, checkedY+h_it);
			// Is it a collision?
			is_collision = tile_id == 0x00 ? 0 : 1;

			if(is_collision==1){
				// Was the collision with a platform?
				if( tile_id == platform1 || tile_id == platform2 || tile_id == platform3 || tile_id == pipe ) {
					return 0;
				}

				// If this is a coin room then check for collision with a coin.
				else if(game.gamestate2==GSTATE2_COINROOM) {
					return gstate2_coinRoom_getCoin(i, tile_id, checkedX, checkedY+h_it);
				}

				// Otherwise just return the collision.
				// else{
				// 	return 0;
				// }
			}
		}

	}

	//
	// NO COLLISION
	//
	return 1;
}
unsigned char collisionDetect_sprite2Sprite(unsigned char actor1, unsigned char actor2){
	// Checks the player against all the bad guys on the screen.

	// Detects collision:
	// -- Removes enemy if it is stunned.
	// -- Harms player if enemy is not stunned.

	// Check player position for collisions with any other sprite positions.
	// -- Enemies
	// -- -- Stunned?
	// -- -- Not stunned?
	// -- Coins? Bonus waffers?

	struct Rect_ {
		unsigned char x;
		unsigned char y;
		unsigned char w;
		unsigned char h;
	};

	struct Rect_ rect1 = {
		actors[actor1].x      ,
		actors[actor1].y      ,
		actors[actor1].width  ,
		actors[actor1].height ,
	};
	struct Rect_ rect2 = {
		actors[actor2].x      ,
		actors[actor2].y      ,
		actors[actor2].width  ,
		actors[actor2].height ,
	};

	unsigned char Ax = ((rect1.x>>3)<<3);
	unsigned char Ay = ((rect1.y>>3)<<3);
	unsigned char AX = ((rect1.x>>3)<<3)+((rect1.w-1)<<3);
	unsigned char AY = ((rect1.y>>3)<<3)+((rect1.h-1)<<3);

	unsigned char Bx = ((rect2.x>>3)<<3);
	unsigned char By = ((rect2.y>>3)<<3);
	unsigned char BX = ((rect2.x>>3)<<3)+((rect2.w-1)<<3);
	unsigned char BY = ((rect2.y>>3)<<3)+((rect2.h-1)<<3);

	return !(AX<Bx || BX<Ax || AY<By || BY<Ay);

}
unsigned char collision_PlayerVsActor_bumpCheck(unsigned char i){
	// Used if bumping from under.

	// Check the surrounding 3 tiles. // Get the x coord.
	unsigned char x_left  = (actors[i].x>>3)-1 ;
	unsigned char x_mid   = (actors[i].x>>3)-0 ;
	// unsigned char x_right = (actors[i].x>>3)+1 ;
	unsigned char y_above = (actors[i].y>>3) ;
	unsigned char yTilePos_underFeet;
	unsigned char enemyFound=false;

	for(unsigned char ii=0; ii<NUM_ACTORS; ii++){
		if( !actors[ii].isEnemy            ){ continue ; }
		if( !actors[ii].isOnScreen         ){ continue ; }
		if(  actors[ii].charType==FIREBALL ){ continue ; }
		// if(  actors[ii].charType==BONUSWAFFER ){ continue ; }

		yTilePos_underFeet = (actors[ii].y >> 3) + (actors[ii].height) ;

		// To be detected the enemy must be directly on the row (not jumping or falling.)
		if(y_above == yTilePos_underFeet){
			if( actors[ii].x>>3 == x_left  ){ enemyFound=true; }
			// else if( actors[ii].x>>3 == x_mid   ){ enemyFound=true; }
			// else if( actors[ii].x>>3 == x_right ){ enemyFound=true; }

			if( actors[ii].x>>3 == x_mid   ){ enemyFound=true; }

			if( enemyFound ) {
				// Normal enemy speed is 2. Faster speed is 1. Fastest speed is 0.
				if     ( actors[ii].charType==SLIPICE)     { bumpEnemy(ii); assignPoints(i, SLIPICE_BUMP_POINTS); playSFX( SND_KICK_ENEMY ); }
				else if( actors[ii].charType==BONUSWAFFER) {                assignPoints(i, BONUSWAFFER_POINTS);  playSFX( SND_GET_BONUSWAFFER ); removeActor(ii); }
				else                                       { bumpEnemy(ii); }

				// Enemy found. No need to continue checking.
				break ;
			}
		}
	}
}
unsigned char collision_PlayerVsActor(unsigned char i){
	// Used if touching another actor. NOT for bumping from under.

	// The passed value of 'i' should be the 'i' of a player.

	for(unsigned char ii=0; ii<NUM_ACTORS; ii++){
		// If it isn't onscreen then we don't care!
		if( !actors[ii].isOnScreen         ){ continue ; }

		// Enemies (also BONUSWAFFER):
		if( actors[ii].isEnemy             ){
			// Did the player touch an enemy?
			if( collisionDetect_sprite2Sprite(i, ii) ){
				// Will the enemy be defeated? (Doesn't work for FIREBALL.)
				if( actors[ii].onBack && !actors[ii].isDefeated ) {
					assignPoints(i, ENEMY_KICK_POINTS);
					actors[ii].isDefeated=true;
					playSFX( SND_KICK_ENEMY );
				}

				// Slipice can be defeated by walking into it if it is melting.
				else if ( actors[ii].vcounter16b_general > 8 && actors[ii].charType == SLIPICE){
					assignPoints(i, SLIPICE_KICK_POINTS);
					removeActor(ii);
					playSFX( SND_KICK_ENEMY );
				}

				// Touching a BONUSWAFFER removes it and gives points.
				else if ( actors[ii].charType == BONUSWAFFER ) {
					assignPoints(i, BONUSWAFFER_POINTS);
					removeActor(ii);
					playSFX( SND_GET_BONUSWAFFER );
				}

				// Will the player be defeated?
				else if( !actors[ii].onBack ) {
					playSFX( SND_PLAYER_DEFEATED );
					actors[i].isDefeated=true;
					return true;
				}
			}
		}
	}

	return false;
}
// COLLISION DETECTION

// ACTOR CONTROL
unsigned char findOpenActorSlot(){
	// Production version:
	unsigned char nextSlot=99;
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		if( actors[i].isOnScreen==0 ) { nextSlot = i ; break; }
	}

	// If the value remains at 99 then there was not a free slot.
	// The calling function needs to handle this.
	return nextSlot;
}
void hideActor(unsigned char i){
	// Hide this enemy.
	actors[i].isOnScreen = 0 ;
}
void removeActor(unsigned char i){
	// Go through the local queue and replace an entry matching the charType with NOENEMY.
	for(unsigned char ii=0; ii<sizeof(phases_enemies_queue); ii++){
		if( phases_enemies_queue[ii] == actors[i].charType ){
			phases_enemies_queue[ii] = NOENEMY    ;
			break;
		}
	}

	// Reset and hide the actor too.
	actors[i].isOnScreen     = 0          ;
	actors[i].charType       = NOENEMY    ;

}
void spawnActor( unsigned char charType ){
	// New version.
	// Don't keep track of the nextActorSlot. Instead, just find an inactive slot.

	// Determine the actor slot that will be used for the the actor.
	unsigned char nextActorSlot = findOpenActorSlot() ;

	// End function if the slot value is the special indicator 99.
	if(nextActorSlot==99){ return; }

	// NOENEMY

	// Universal settings.
	actors[ nextActorSlot ].charType            = charType ;
	actors[ nextActorSlot ].vcounter8b_anim     = 0 ;
	actors[ nextActorSlot ].vcounter8b_general  = 0 ;
	actors[ nextActorSlot ].vcounter16b_general = 0 ;
	// actors[ nextActorSlot ].isIdle              = 1 ;
	actors[ nextActorSlot ].yDir                = 1 ; // Falling.
	actors[ nextActorSlot ].xFramesRemaining    = 0 ;
	actors[ nextActorSlot ].yFramesRemaining    = 0 ;
	// actors[ nextActorSlot ].speed               = 3 ;
	actors[ nextActorSlot ].isDefeated          = 0 ;
	actors[ nextActorSlot ].isOnScreen          = 1 ;
	actors[ nextActorSlot ].isEnemy             = 0 ;
	actors[ nextActorSlot ].whichRow            = 0 ;
	actors[ nextActorSlot ].onBack              = 0 ;
	actors[ nextActorSlot ].width               = 1 ;
	actors[ nextActorSlot ].isStopping          = 0 ;
	actors[ nextActorSlot ].isMoving            = 0 ;

	// actors[ nextActorSlot ].movespeed        = 0 ;
	// actors[ nextActorSlot ].speed            = 0 ;
	actors[ nextActorSlot ].speed_counter    = 0 ;

	// Players.
	if( charType == MARIO || charType == LUIGI ) {
		actors[ nextActorSlot ].height   = 3;
		actors[ nextActorSlot ].y        = PLAYER_RESPAWN_Y_TILE; // 22*8 ;
		actors[ nextActorSlot ].x        = charType == MARIO ? MARIO_RESPAWN_X_TILE : LUIGI_RESPAWN_X_TILE ;
		actors[ nextActorSlot ].xDir     = charType == MARIO ? 1 : 0 ;
		actors[ nextActorSlot ].yDir     = 2 ;
		actors[ nextActorSlot ].whichRow = 4 ;
		// actors[ nextActorSlot ].isEnemy  = 1 ;
		actors[ nextActorSlot ].isEnemy  = 0 ;
		actors[ nextActorSlot ].speed    = 0 ;
		// actors[ nextActorSlot ].speed    = DEFAULT_PLAYER_SPEED ;

		// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
		// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
		// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
	}
	// Fireball.
	else if(charType == FIREBALL){

		unsigned char Xpos_choices[] = {
			FIREBALL_SPAWN_X_TILE_LEFT,
			FIREBALL_SPAWN_X_TILE_RIGHT
		};         //
		unsigned char Ypos_choices[] = {
			// 4, 10, 16, 22
			FIREBALL_SPAWN_Y_TILE_ROW1,
			FIREBALL_SPAWN_Y_TILE_ROW2,
			FIREBALL_SPAWN_Y_TILE_ROW3,
			FIREBALL_SPAWN_Y_TILE_ROW4

		}; //
		int x = (rand() % 2);
		int y = (rand() % 4);

		actors[ nextActorSlot ].x        = Xpos_choices[ x ] << 3 ;
		actors[ nextActorSlot ].y        = Ypos_choices[ y ] << 3 ;
		actors[ nextActorSlot ].xDir     = x==0 ? 1 : 0;
		actors[ nextActorSlot ].isEnemy  = 1 ;

		actors[ nextActorSlot ].height = 1;
		// actors[ nextActorSlot ].speed    = DEFAULT_ENEMY_SPEED ;
		actors[ nextActorSlot ].speed  = ENEMY_MAX;
		// actors[ nextActorSlot ].speed    = DEFAULT_PLAYER_SPEED ;

		// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
		// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
		// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
	}

	// Other enemies.
	else if( charType != MARIO && charType != LUIGI ) {
		// Randomly decide which pipe the actor will appear from.
		unsigned char Xpos_choices[] = {
			// 5, 20
			ENEMY_SPAWN_X_TILE_LEFT,
			ENEMY_SPAWN_X_TILE_RIGHT
		};
		int x = (rand() % 2);

		// Assign the x and the xDir.
		actors[ nextActorSlot ].xDir    = Xpos_choices[ x ] == ENEMY_SPAWN_X_TILE_LEFT ? 1 : 0 ;
		actors[ nextActorSlot ].x       = Xpos_choices[ x ] ;// << 3 ;
		// actors[ nextActorSlot ].y       = 1 << 3 ;

		// // Set as an enemy if this isn't a BONUSWAFFER.
		// if(charType == BONUSWAFFER){ actors[ nextActorSlot ].isEnemy = 0 ; }
		// else                       { actors[ nextActorSlot ].isEnemy = 1 ; }

		// Set as enemy. Only the players aren't enemies.
		actors[ nextActorSlot ].isEnemy  = 1 ;

		if     ( charType == SHELLSCRAPER) {
			actors[ nextActorSlot ].height = 1;
			actors[ nextActorSlot ].y       = 2 << 3 ;
			actors[ nextActorSlot ].speed    = ENEMY_NORMAL ;

			// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
		}

		else if( charType == SIDESTEPPER)  {
			actors[ nextActorSlot ].height = 2;
			actors[ nextActorSlot ].y      = 1 << 3 ;
			actors[ nextActorSlot ].speed    = ENEMY_SPEED_SLOW ;
			// actors[ nextActorSlot ].speed  = 0;
			// actors[ nextActorSlot ].onBack  = 1;

			// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
		}

		else if( charType == FIGHTERFLY)   {
			actors[ nextActorSlot ].height = 2;
			actors[ nextActorSlot ].y      = 1 << 3 ;
			actors[ nextActorSlot ].speed    = ENEMY_NORMAL ;
			// actors[ nextActorSlot ].speed  = 0;

			// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
		}

		else if( charType == SLIPICE)      {
			actors[ nextActorSlot ].height = 2;
			actors[ nextActorSlot ].y      = 1 << 3 ;
			actors[ nextActorSlot ].speed    = ENEMY_NORMAL ;

			// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
		}

		else if( charType == BONUSWAFFER)  {
			actors[ nextActorSlot ].height = 1;
			actors[ nextActorSlot ].y      = 1 << 3 ;

			// Bonus waffer moves a little faster.
			// actors[ nextActorSlot ].speed    = DEFAULT_ENEMY_SPEED ;
			actors[ nextActorSlot ].speed=ENEMY_MAX ;

			// actors[ nextActorSlot ].movespeed        = MOVESPEED_STOP ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_NORMAL ;
			// actors[ nextActorSlot ].movespeed        = MOVESPEED_FAST ;
		}
	}

	// Settings specific to the CharType.

	// ALL actor speed override.
	// actors[ nextActorSlot ].speed=0 ;
}
void gameInit_actorSystem(){
	SetUserRamTilesCount(0);

	// Reset the global counters.
	vsynccounter16b_1   = 0 ;
	vsynccounter8b_gen1 = 0 ;

	// Clear the sprites.
	for(unsigned char i=0; i<MAX_SPRITES; i++){ sprites[i].x=OFF_SCREEN ; }

	// Hide each actor and reset their individual timers.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		actors[i].isOnScreen          = 0 ;
		actors[i].charType            = NOENEMY;
		actors[i].vcounter8b_anim     = 0 ;
		actors[i].vcounter8b_general  = 0 ;
		actors[i].vcounter16b_general = 0 ;
	}

}
// ACTOR CONTROL

// ACTOR POSITION ADJUSTMENTS
void moveEnemies(){
	// if( vsyncCounter<1){
	// 	vsyncCounter++;
	// 	return;
	// }
	// else{
	// 	vsyncCounter=0;
	// }

	// Handles enemies emerging from the top pipe.
	// Handles enemy logic.
	// Handles enemy movement.

	// A FIGHTERFLY can only move if no other FIGHTERFLY are moving.
	// This means that 1 can always be jumping but it should not be the same one jumping each time.
	// It can be determined if a FIGHTERFLY is moving by searching all actor FIGHTERFLY and looking for xframe/yframes remaining.
	// Max of 3 enemies on screen at the same time.
	// Don't count the hard mode fireball.

	// Look through the actors array of structures to get some counts.
	unsigned char numEnemyOnscreen    = 0 ;
	unsigned char numMovingFighterFly = 0 ;
	// unsigned char pestAtTopRow        = false;
	// unsigned char charType            ;

	unsigned char pestOn_row0 = false ;
	unsigned char pestOn_row1 = false ;
	unsigned char pestOn_row2 = false ;
	unsigned char pestOn_row3 = false ;
	unsigned char pestOn_row4 = false ;

	unsigned char onscreen_SHELLSCRAPER  = 0 ;
	unsigned char onscreen_SIDESTEPPER   = 0 ;
	unsigned char onscreen_FIGHTERFLY    = 0 ;
	unsigned char onscreen_SLIPICE       = 0 ;

	unsigned char remaining_SHELLSCRAPER = 0 ;
	unsigned char remaining_SIDESTEPPER  = 0 ;
	unsigned char remaining_FIGHTERFLY   = 0 ;
	unsigned char remaining_SLIPICE      = 0 ;

	unsigned char yTilePos_underFeet     = 0 ;

	// DEBUG
	unsigned char font_index = pgm_read_byte(&(greenS0[2])) ;

	unsigned char enemyCount=0;

	// Get onscreen counts.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		// charType = actors[i].charType ;
		// yTilePos_underFeet = (actors[i].y >> 3) + (actors[i].height) ;

		// How many enemies are on screen? (Includes BONUSWAFFER.)
		if( actors[i].isEnemy == 1 && actors[i].isOnScreen == 1 && actors[i].charType != FIREBALL ) { numEnemyOnscreen++ ; }

		// Identify the FIGHTERFLY actors.
		if( actors[i].isEnemy == 1 && actors[i].isOnScreen == 1 && actors[i].charType == FIGHTERFLY ) {
			// Is this FIGHTERFLY moving?
			// if( actors[i].xFramesRemaining && actors[i].yFramesRemaining ) { numMovingFighterFly++ ; }
			// If not moving up or down...
			if( actors[i].yDir != Y_NONE ) { numMovingFighterFly++ ; }
		}

		// Determine which rows are occuiped by a pest. Two pests should not occupy the same row.
		if( actors[i].isEnemy == 1 && actors[i].isOnScreen == 1 && actors[i].charType != FIREBALL) {
			// If at top row or above (but not on) the 2nd row:
			// if( (actors[i].y >> 3) < 12 ) { pestAtTopRow = true; }
			// if ( (actors[i].y >> 3 > 0 ) && (actors[i].y >> 3 < 8 ) ){ pestAtTopRow = true; } // Row 1 (top)    :

			// What row is this active enemy on?
			if     ( actors[i].whichRow==0 ) { pestOn_row0 = true ; }
			else if( actors[i].whichRow==1 ) { pestOn_row1 = true ; }
			else if( actors[i].whichRow==2 ) { pestOn_row2 = true ; }
			else if( actors[i].whichRow==3 ) { pestOn_row3 = true ; }
			else if( actors[i].whichRow==4 ) { pestOn_row4 = true ; }
		}

		// Count the usages of the actor types.
		if     ( actors[i].charType==SHELLSCRAPER && actors[i].isOnScreen ) { onscreen_SHELLSCRAPER ++; }
		else if( actors[i].charType==SIDESTEPPER  && actors[i].isOnScreen ) { onscreen_SIDESTEPPER  ++; }
		else if( actors[i].charType==FIGHTERFLY   && actors[i].isOnScreen ) { onscreen_FIGHTERFLY   ++; }
		else if( actors[i].charType==SLIPICE      && actors[i].isOnScreen ) { onscreen_SLIPICE      ++; }
	}

	// Get local queue counts.
	for(unsigned char i=0; i<sizeof(phases_enemies_queue); i++){
		// Count the usages of the actor types.
		// charType = actors[i].charType ;

		// switch( phases_enemies_queue[i] ){
		// 	case SHELLSCRAPER : { remaining_SHELLSCRAPER ++; break; }
		// 	case SIDESTEPPER  : { remaining_SIDESTEPPER  ++; break; }
		// 	case FIGHTERFLY   : { remaining_FIGHTERFLY   ++; break; }
		// 	case SLIPICE      : { remaining_SLIPICE      ++; break; }
		// 	// default             : { break; }
		// };

		if     ( phases_enemies_queue[i] == SHELLSCRAPER ) { remaining_SHELLSCRAPER ++; }
		else if( phases_enemies_queue[i] == SIDESTEPPER  ) { remaining_SIDESTEPPER  ++; }
		else if( phases_enemies_queue[i] == FIGHTERFLY   ) { remaining_FIGHTERFLY   ++; }
		else if( phases_enemies_queue[i] == SLIPICE      ) { remaining_SLIPICE      ++; }
	}

	// Can we add a pest at one of the top pipes? (This used to be 3)
	// _emu_whisper(0, numEnemyOnscreen);
	// _emu_whisper(1, numEnemyOnscreen);

	// _emu_whisper(0,
	// 	0 &&
	// 	(
	// 	   pestOn_row0 << 1
	// 	| pestOn_row1 << 2
	// 	| pestOn_row2 << 3
	// 	| pestOn_row3 << 4
	// 	| pestOn_row4 << 5
	// 	)
	// );

	// DEBUG - indicate if a pest is on a row (does not count.)
	// unsigned char font_index = pgm_read_byte(&(greenS0[2])) ;
	// SetTile(8, 0 , (pestOn_row0) + font_index);
	// SetTile(8, 7 , (pestOn_row1) + font_index);
	// SetTile(8, 13, (pestOn_row2) + font_index);
	// SetTile(8, 19, (pestOn_row3) + font_index);
	// SetTile(8, 25, (pestOn_row4) + font_index);

	// ADD ENEMIES TO THE SCREEN.
	if( numEnemyOnscreen < 3 && (!pestOn_row0 && !pestOn_row1) ){
		// Restart the queue index if necessary.
		if( !(phases_enemies_queue_index<sizeof(phases_enemies_queue) ) ){ phases_enemies_queue_index = 0 ;}

		// Go through the queue.
		for(unsigned char i=phases_enemies_queue_index; i<sizeof(phases_enemies_queue); i++){
			// Skip this one if the next pest in the queue is NOENEMY.
			if( phases_enemies_queue[i] == NOENEMY ){ phases_enemies_queue_index++; continue; }

			// Put this next pest into circulation from the local queue!
			// Unless the max remaining for this actor type are already on the screen.
			if     ( phases_enemies_queue[i] == SHELLSCRAPER && onscreen_SHELLSCRAPER < remaining_SHELLSCRAPER ){ spawnActor( phases_enemies_queue[i] ); }
			else if( phases_enemies_queue[i] == SIDESTEPPER  && onscreen_SIDESTEPPER  < remaining_SIDESTEPPER  ){ spawnActor( phases_enemies_queue[i] ); }
			else if( phases_enemies_queue[i] == FIGHTERFLY   && onscreen_FIGHTERFLY   < remaining_FIGHTERFLY   ){ spawnActor( phases_enemies_queue[i] ); }
			// else if( phases_enemies_queue[i] == SLIPICE      && onscreen_SLIPICE      < remaining_SLIPICE      ){ spawnActor( phases_enemies_queue[i] ); }

			// Do we add a SLIPICE?
			// else if( game.phaseNum > 8 ) {
				// spawnActor( phases_enemies_queue[i] ;
			// }

			phases_enemies_queue_index++;

			break;
		}
	}

	// Work with actors that are NOT players.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		// Skip this iteration if the actor !isEnemy, !isOnScreen, or is NOENEMY
		if     ( !actors[i].isEnemy           ){ continue; }
		else if( !actors[i].isOnScreen        ){ continue; }
		else if(  actors[i].charType==NOENEMY ){ continue; }

		// if( actors[i].vcounter8b_general2 < 50 ){ continue; }

		// Is this the last active enemy in the queue? If so, set it to ENEMY_MAX.
		if(getEnemyCount_inLocalQueue() == 1){ actors[i].speed=ENEMY_MAX; }

		unsigned char canMoveLeft  = collisionDetect_sprite2Background(i, D_LEFT)  ;
		unsigned char canMoveRight = collisionDetect_sprite2Background(i, D_RIGHT) ;
		unsigned char shouldFall;
		shouldFall   = collisionDetect_sprite2Background(i, D_DOWN)  ;
		unsigned char actualRow = 99;

		// If the actor is on row 0 then it should fall down.
		if(actors[i].whichRow==0){
			shouldFall=true;
			actors[i].yDir = 1;
		}

		// unsigned char xtile    = actors[i].x >> 3                        ;
		// unsigned char ytile    = actors[i].y >> 3                        ;
		unsigned char charType = actors[i].charType                      ;
		yTilePos_underFeet     = (actors[i].y >> 3) + (actors[i].height) ;
		unsigned char speed    = actors[i].speed                         ;
		// unsigned char isEnemy  = actors[i].isEnemy                       ;

		// DETERMINE THE ROW THAT THE ENEMY IS OCCUPIING.

		// Handle pests.
		if     ( yTilePos_underFeet >= ROW0_START && yTilePos_underFeet <= ROW0_END ){ actors[i].whichRow=0; } // Row 0 - (initial fall from pipe) :
		else if( yTilePos_underFeet >= ROW1_START && yTilePos_underFeet <= ROW1_END ){ actors[i].whichRow=1; } // Row 1 - (top)    :
		else if( yTilePos_underFeet >= ROW2_START && yTilePos_underFeet <= ROW2_END ){ actors[i].whichRow=2; } // Row 2 -          :
		else if( yTilePos_underFeet >= ROW3_START && yTilePos_underFeet <= ROW3_END ){ actors[i].whichRow=3; } // Row 3 -          :
		else if( yTilePos_underFeet >= ROW4_START && yTilePos_underFeet <= ROW4_END ){ actors[i].whichRow=4; } // Row 4 - (bottom) :

		// We still need the actual row for the BONUSWAFFER...
		actualRow = actors[i].whichRow;

		// We don't want the bonus waffers to interfere with the enemies. This will help prevent that.
		if(actors[i].charType==BONUSWAFFER){ actors[i].whichRow=99; }

		// Is this enemy defeated?
		if(actors[i].isDefeated){
			// If the actor has reached the bottom...
			if( actors[i].y>>3 > (25-actors[i].height) ){
				// Defeat the enemy. This removes it from the local queue and removes it from the screen.
				removeActor(i);

				// Convert the enemy into a BONUSWAFFER unless this enemy was the last one.
				enemyCount=getEnemyCount_inLocalQueue();

				// Spawn a bonus waffer.
				if(enemyCount){
					spawnActor(BONUSWAFFER);
				}

				// We are done with this actor for now.
				continue;
			}

			// Not at bottom? Move enemy down quickly.
			else{
				actors[i].y+=2;

				// Set an invalid whichRow. We don't want this defeated enemy to interfere with other enemies.
				actors[i].whichRow=99;
			}

			// Skip the rest of this iteration.
			continue;
		}

		// Set the current row value for this actor.

		// If no more x frames AND no more y frames then re-align on x/y axis.
		if( actors[i].xFramesRemaining == 0 && actors[i].yFramesRemaining == 0){
			// Unless this is a FighterFly.
			if(charType==FIGHTERFLY){
				if(!shouldFall){ actors[i].y = ((actors[i].y >> 3) << 3) ; }
			}
			else{
				actors[i].x = ((actors[i].x >> 3) << 3) ;
				actors[i].y = ((actors[i].y >> 3) << 3) ;
			}

			// Also, check if the actor has reached either bottom pipe.
			if(
				// Is on bottom row or is the BONUSWAFFER.
				// (actors[i].whichRow == 4 || (charType==BONUSWAFFER && wafferActualRow==4 ) )
				actualRow==4

				// And has reached a bottom pipe.
				&& (
					   actors[i].x == BOUNDRY_BOTTOM_RIGHT_PIPE_COL
					|| actors[i].x == BOUNDRY_BOTTOM_LEFT_PIPE_COL
				   )
				// And the is not the FIREBALL.
				&& actors[i].charType != FIREBALL
			){
				// Hide the enemy.
				hideActor(i);

				// If this was a BONUSWAFFER then it is gone. Remove it completely.
				if(actors[i].charType == BONUSWAFFER){ removeActor(i); }

				// Defeat the enemy.
				// removeActor(i);
			}
		}

		// Should SLIPICE start melting?
		if( charType==SLIPICE      ) {
			// The floors must be frozen in order from the bottom up.

			// If one of the top 3 conditions match then the melting should begin.
			if     ( actors[i].whichRow==3 && (yTilePos_underFeet==19) && !game.row3Frozen )                                      { }
			else if( actors[i].whichRow==2 && (yTilePos_underFeet==13) && !game.row2Frozen && game.row3Frozen)                    { }
			else if( actors[i].whichRow==1 && (yTilePos_underFeet==7 ) && !game.row1Frozen && game.row2Frozen && game.row3Frozen ){ }

			// If no match then reset the counter repeatedly until one of the above 3 conditions match.
			else {
				actors[i].vcounter16b_general=0;
			}

			// Is the slipice in a melting state?
			if( (actors[i].vcounter16b_general >= SLIPICE_F1_DELAY) ){
				// No coordinate updates. Remove remaining xFrames. Keep remaining yFrames.
				actors[i].xFramesRemaining=0;

				continue;
			}
		}

		// Just moves left, right, or falls... or doesn't move because it is stunned.
		// Shellscrapper and Sidestepper can move anytime but can only fall to the next row if it isn't occupied.
		if(
			charType==SHELLSCRAPER ||
			charType==SIDESTEPPER  ||
			charType==SLIPICE      ||
			charType==BONUSWAFFER  ||
			charType==FIGHTERFLY
		) {
			// Use remaining yFrames if there are some.
			if( actors[i].yFramesRemaining > 0 ){
				// Deduct a yFrame.
				actors[i].yFramesRemaining --;

				// Move UP?
				if     (actors[i].yDir==0){ actors[i].y--; }

				// Move DOWN?
				else if(actors[i].yDir==1){ actors[i].y++; }
			}

			// No remaining yFrames? Realign on Y.
			else if( charType!=FIGHTERFLY){ actors[i].y = ( (actors[i].y >> 3) << 3 ); }

			// If enemy is onBack then it shouldn't move.
			if( actors[i].onBack ){
				// Fall check. No new x frames if the actor should be falling.
				if( shouldFall ) {
					// actors[i].yFramesRemaining=8;
					actors[i].y++;
					actors[i].yDir = 1;

					// REDUNDANT! EXACTLY DUPLICATE!
					// Use remaining x frames.
					// ENEMY SPEED CONTROL
					/*
					if( actors[i].xFramesRemaining > 0 && actors[i].vcounter8b_general > speed){
						// Move left?
						if     (actors[i].xDir==0){ actors[i].x--; }
						// Move right?
						else if(actors[i].xDir==1){ actors[i].x++; }

						actors[i].xFramesRemaining --;
						actors[i].vcounter8b_general=0;
					}
					*/

				}
				else{
					// actors[i].y = ( (actors[i].y >> 3) << 3 );
				}

				// Keep track of the actor's timers.
				continue;
			}

			// Should the actor turn around instead of falling?
			// Is there a pest on the row below this pest and this pest is set to fall?
			if(shouldFall==true){
				if     (
					   (actors[i].whichRow==0 && pestOn_row1 && yTilePos_underFeet==4 )
					|| (actors[i].whichRow==1 && pestOn_row2 && yTilePos_underFeet==7 )
					|| (actors[i].whichRow==2 && pestOn_row3 && yTilePos_underFeet==13 )
					|| (actors[i].whichRow==3 && pestOn_row4 && yTilePos_underFeet==19 )
				){
					// Clear the shouldFall flag.
					shouldFall = false;

					// Remove any remaining xFrames.
					actors[i].xFramesRemaining=0;

					// Is this actor currently facing left?
					if(actors[i].xDir==X_LEFT){
						// Adjustment needed or it ends up being one-off.
						actors[i].x+=1;
					}

					// If FIGHERFLY then set the jump timer high so that the next jump occurs without delay.
					if( charType==FIGHTERFLY){ actors[i].vcounter8b_general2=255; }

					// Re-align on x.
					actors[i].x = (actors[i].x >> 3) << 3;

					// Switch directions.
					actors[i].xDir = !actors[i].xDir;
				}
			}

			// Do we give more x frames?
			if( actors[i].xFramesRemaining == 0 ){
				// Fall check. No new x frames if the actor should be falling.
				if( shouldFall ) {
					actors[i].yFramesRemaining=8;
					// actors[i].yFramesRemaining+=8;
					actors[i].yDir = 1;
				}
				else{
					actors[i].yFramesRemaining=0;
					actors[i].yDir = 2;

					// Give more x frames.
					if     (
							(actors[i].xDir==0 && canMoveLeft || actors[i].xDir==1 && canMoveRight) &&
							 actors[i].yDir==2
						) {

						// Adjust for walking past the sides of the game field.
						// x, 1==right, 0==left.
						if     (actors[i].xDir==1 && actors[i].x == BOUNDRY_RIGHT_COL) { actors[i].x=BOUNDRY_LEFT_COL  ; break; }
						else if(actors[i].xDir==0 && actors[i].x == BOUNDRY_LEFT_COL ) { actors[i].x=BOUNDRY_RIGHT_COL ; break; }

						// Handle FIGHTERFLY jump behavior.
						if( charType==FIGHTERFLY){
							if ( ! actors[i].yFramesRemaining ) {
								if(
									// Is it going fast speed?
									(
										actors[i].speed == ENEMY_MAX &&
										actors[i].vcounter8b_general2 > SPEED_FIGHTERFLY_FAST
									)
									||
									// Is it going normal speed?
									(
										actors[i].speed != ENEMY_MAX &&
										actors[i].vcounter8b_general2 > SPEED_FIGHTERFLY_NORMAL
									)

								)  {
									// Only allow the movement if 0, or 1 FIGHTERFLY is already moving.
									if(numMovingFighterFly < 2){
									// if(numMovingFighterFly < 1){
										// WaitVsync(50);
										numMovingFighterFly=99;
										actors[i].vcounter8b_general2 = 0 ;

										// Give 8 more yFrames.
										actors[i].yFramesRemaining=8;

										// Set yDir to jumping.
										actors[i].yDir = 0;

										actors[i].isMoving         = 1 ;
										actors[i].xFramesRemaining = 8 ;
										actors[i].isStopping       = 0 ;
										// actors[i].isIdle           = 0 ;
									}
								}
							}
						}

						// All other pests...
						else{
							actors[i].isMoving         = 1 ;
							actors[i].xFramesRemaining = 8 ;
							actors[i].isStopping       = 0 ;
							// actors[i].isIdle           = 0 ;

						}

						actors[i].x = (actors[i].x >> 3) << 3;
					}

					// Switch direction.
					else if(actors[i].yDir!=1){
						// _emu_whisper(0,actors[i].x);
						actors[i].xDir = ! actors[i].xDir;
						actors[i].x = (actors[i].x >> 3) << 3;
						// _emu_whisper(1,actors[i].x);
						// WaitVsync(50);
					}

				}

			}

			// Use remaining x frames.

			// Allow movement if the speed counter has reached the limit.
			else if( actors[i].xFramesRemaining > 0 && actors[i].speed_counter >= DEFAULT_SPEED_COUNTER_LIMIT ){
				// Move left?
				if     (actors[i].xDir==0){ actors[i].x--; }
				// Move right?
				else if(actors[i].xDir==1){ actors[i].x++; }

				actors[i].xFramesRemaining --;
				actors[i].vcounter8b_general=0;
				actors[i].speed_counter=0;
			}
			// Otherwise, increment the speed_counter by the actor's speed setting.
			else{
				actors[i].speed_counter += speed;
			}

/*
			if( actors[i].xFramesRemaining > 0 && actors[i].vcounter8b_general > speed){
				// Move left?
				if     (actors[i].xDir==0){ actors[i].x--; }
				// Move right?
				else if(actors[i].xDir==1){ actors[i].x++; }

				actors[i].xFramesRemaining --;
				actors[i].vcounter8b_general=0;
			}
*/

		}

	}

}
void movePlayers(){
	unsigned int btnPressed ;
	unsigned int btnHeld    ;

	unsigned char isJumpButton ;
	unsigned char isLeftDir    ;
	unsigned char isRightDir   ;
	unsigned char canMoveLeft  ;
	unsigned char canMoveRight ;
	unsigned char shouldFall   ;
	unsigned char canJumpUp    ;

	unsigned char speed    ;

	// Work with actors that are players.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		if( actors[i].isEnemy ){ continue; }

		if(!actors[i].isOnScreen){ continue; }

		speed    = actors[i].speed                         ;
		// GET NEW INPUTS.

		// PLAYER: MARIO:
		if      ( actors[i].charType==MARIO && game.player1Lives > 0 ) {
			btnPressed = game.btnPressed1; btnHeld = game.btnHeld1;
		}
		// PLAYER: LUIGI:
		else if ( actors[i].charType==LUIGI && game.player2Lives > 0 ) {
			// btnPressed = game.btnPressed1; btnHeld = game.btnHeld1;
			btnPressed = game.btnPressed2; btnHeld = game.btnHeld2;
		}
		// NOT A PLAYER?
		else{ continue; }

		// Look for collisions with other actors.
		unsigned char defeated = collision_PlayerVsActor(i);
		// if(defeated){ break; }

		// If the player is in a defeated state then just move them down.
		if( actors[i].isDefeated==true ){
			// If the actor has reached the bottom...
			if( actors[i].y>>3 > (25-actors[i].height) ){
				// Remove the defeated flag.
				actors[i].isDefeated=false;

				// Does the player have any lives remaining?
				if     (actors[i].charType==MARIO){
					if(game.player1Lives) {
						game.player1Lives -- ;
						printNumber(PRINT_PLAYER1LIVES, game.player1Lives);
					}
					if (!game.player1Lives){ removeActor(i); continue; }
					else{
						// Put the player at the top respawn point.
						actors[i].x=MARIO_RESPAWN_X_TILE;
						actors[i].y=PLAYER_RESPAWN_Y_TILE;
						actors[i].whichRow=0;
						actors[i].vcounter16b_general=0;
						actors[i].xFramesRemaining = 0 ;
						actors[i].yFramesRemaining = 0 ;
					}
				}
				else if(actors[i].charType==LUIGI){
					if(game.player2Lives) {
						game.player2Lives -- ;
						printNumber(PRINT_PLAYER2LIVES, game.player2Lives);
					}
					if (!game.player2Lives){ removeActor(i); continue; }
					else{
						// Put the player at the top respawn point.
						actors[i].x=LUIGI_RESPAWN_X_TILE;
						actors[i].y=PLAYER_RESPAWN_Y_TILE;
						actors[i].whichRow=0;
						actors[i].vcounter16b_general=0;
						actors[i].xFramesRemaining = 0 ;
						actors[i].yFramesRemaining = 0 ;
					}
				}

			}
			// Not at bottom? Move enemy down quickly.
			else{
				actors[i].y+=2;
			}

			// Skip the rest of this for loop iteration.
			continue;
		}

		// ACT ON NEW INPUTS.
		// Determine what direction the player has been requested
		// isJumpButton = ( btnPressed & (BTN_A | BTN_B | BTN_X | BTN_Y | BTN_UP) ) ? 1 : 0 ;
		// isJumpButton = ( btnPressed & (BTN_UP | BTN_A) ) ? 1 : 0 ;
		// isJumpButton = ( btnPressed & BTN_UP)    ? 1 : 0 ;
		isJumpButton = ( btnPressed & BTN_A )    ? 1 : 0 ;
		isLeftDir    = ( btnHeld    & BTN_LEFT)  ? 1 : 0 ;
		isRightDir   = ( btnHeld    & BTN_RIGHT) ? 1 : 0 ;

		// Collision check for the new frames. Can the player move into the next tiles?
		canJumpUp    = collisionDetect_sprite2Background(i, D_UP)    ;
		shouldFall   = collisionDetect_sprite2Background(i, D_DOWN)  ;
		// && !(canJumpUp && shouldFall)
		canMoveLeft  = actors[i].xFramesRemaining == 0 && collisionDetect_sprite2Background(i, D_LEFT) ;
		canMoveRight = actors[i].xFramesRemaining == 0 && collisionDetect_sprite2Background(i, D_RIGHT)  ;

		// If jumping or falling then do NOT allow the player to change direction.
		// actors[i].xDir
		// actors[i].yDir

/*
		// If jumping or falling
		if( actors[i].yDir != Y_NONE ){
			// If the player CAN move left and is holding the left button and was already moving left...
			if     ( !(actors[i].xDir == X_LEFT && isLeftDir) && canMoveLeft ){
			if(canMoveLeft && isLeftDir && actors[i].xDir == X_LEFT){}
				// canMoveLeft=false;
				// canMoveRight=false;
			}
			// If the player CAN move right and is holding the right button and was already moving right...
			else if( !(actors[i].xDir == X_RIGHT && isRightDir)  && canMoveRight ){
				// canMoveLeft=false;
				// canMoveRight=false;
			}
		}
*/

		// // If facing the left direction.
		// if( actors[i].xDir==0 && actors[i].isMoving==1 && !canMoveLeft){
		// 		actors[i].isMoving         = 0 ;
		// 		actors[i].xFramesRemaining = 0;
		// 		//actors[i].isIdle           = 0 ;
		// 		actors[i].isStopping       = 0 ;
		// 		continue;
		// }
		// // If facing the right direction.
		// if( actors[i].xDir==1 && actors[i].isMoving==1 && !canMoveRight){
		// 		actors[i].isMoving         = 0 ;
		// 		actors[i].xFramesRemaining = 0;
		//		//actors[i].isIdle           = 0 ;
		// 		actors[i].isStopping       = 0 ;
		// 		continue;
		// }

		// Add xFrames if the player isn't current moving or currently stopping.
		// if( actors[i].xFramesRemaining == 0 && actors[i].isStopping==0 ){
		if( actors[i].xFramesRemaining == 0 && actors[i].isStopping==0 ){
			// Moving left or right? Allowed to move on the X axis?
			if     ( (isLeftDir && canMoveLeft) || (isRightDir && canMoveRight) ) {
				// Moving!
				// Which way?
				// Don't allow a change of xDir if falling or jumping.
				// if      ( isLeftDir  && ( actors[i].yDir == Y_NONE ) ) { actors[i].xDir=0; }
				// else if ( isRightDir && ( actors[i].yDir == Y_NONE ) ) { actors[i].xDir=1; }
				if      ( isLeftDir  ) { actors[i].xDir=0; }
				else if ( isRightDir ) { actors[i].xDir=1; }

				// Adjust for walking past the sides of the game field.
				// x, 1==right, 0==left.
				if     (actors[i].xDir==1 && actors[i].x==BOUNDRY_RIGHT_COL) { actors[i].x=BOUNDRY_LEFT_COL  ; break; }
				else if(actors[i].xDir==0 && actors[i].x==BOUNDRY_LEFT_COL ) { actors[i].x=BOUNDRY_RIGHT_COL ; break; }
				else{
					// Set the rest of the flags.
					actors[i].isMoving         = 1 ;
					actors[i].xFramesRemaining = 8 ;
					actors[i].isStopping       = 0 ;
					// actors[i].isIdle           = 0 ;
				}

				// Play the player movement sound but only if yDir ==2 AND not skiding.
				if( actors[i].yDir == 2 && actors[i].isStopping != 1){
					playSFX( SND_PLAYER_MOVEMENT );
				}

				actors[i].speed_counter=0;
			}
		}

		// Jump?
		// Jump requested?
		if( isJumpButton ){
			if( actors[i].yDir==Y_NONE && !shouldFall && actors[i].yFramesRemaining == 0 && canJumpUp){
				actors[i].yDir=Y_UP; // Jump!
				actors[i].yFramesRemaining=29; // Jump!

				playSFX( SND_PLAYER_JUMP );
				// actors[i].isIdle=0;
			}
		}

		// No gamepad buttons pressed? Check the isMoving flag and the xFramesRemaining flag, and the yDir flag.
		if( actors[i].isMoving==1 && actors[i].xFramesRemaining == 0 && actors[i].yDir == Y_NONE){
			// Skid? (DISABLED)
			// If not moving up or down...
			if ( 1==2 && actors[i].yDir == Y_NONE ){
				// and can still move into the next tiles in the actor's same direction...
				if( ( canMoveLeft && actors[i].xDir==0 ) || ( actors[i].xDir==1 && canMoveRight ) ){
					// SKID! Assign some more xFrames.
					actors[i].xFramesRemaining = 8;
					// Set the skid flag.
					actors[i].isStopping=1;
					// Unset the idle flag.
					// actors[i].isIdle=0;

					// Play the skid sound.
					playSFX( SND_PLAYER_SKID );
				}
			}
		}

		// ACT ON PREVIOUS INPUT THAT IS STILL IN PLAY.
		// Should we move the character on the X axis?
		// if( actors[i].xFramesRemaining > 0 ){
		if( actors[i].xFramesRemaining > 0 && actors[i].vcounter8b_general > speed){
			actors[i].vcounter8b_general=0;
			actors[i].xFramesRemaining --;
			// Move left?
			if     (actors[i].xDir==0){ actors[i].x--; }
			// Move right?
			else if(actors[i].xDir==1){ actors[i].x++; }
		}

		// Jump up.
		if( actors[i].yDir == Y_UP ) {
			if( actors[i].yFramesRemaining > 0 ){
				// Make sure we can still move up.
				if( canJumpUp ){ actors[i].yFramesRemaining--; actors[i].y-=2; }

				// Can't move up?
				else           {
					// Start falling.
					actors[i].yDir=1;
					actors[i].yFramesRemaining=255;

					// Since our jump was blocked do a bump-check for actors above.
					collision_PlayerVsActor_bumpCheck(i);
				}
			}
			// At the peak of the jump? Change directions and give more yFrames.
			else            { actors[i].yDir=1; actors[i].yFramesRemaining=255; }

		}
		// Fall down.
		else if( actors[i].yDir == Y_DOWN ) {
			if( actors[i].yFramesRemaining > 0 ){
				// Can we still fall?
				if( shouldFall ){ actors[i].yFramesRemaining--; actors[i].y+=2; }
				// Can't fall anymore? End the falling.
				else{ actors[i].yDir=2; actors[i].yFramesRemaining=0; }

			}
			// No more frames left. Actor should have landed.
			else{
					actors[i].yDir=2;
					actors[i].yFramesRemaining=0;

				// Prevent the skid if isMoving wasn't set.
				if(actors[i].isMoving==0){
					actors[i].isStopping=0;
				}
			}

		}

		// If there are no xframes or yframes remaining set the isIdle flag for this actor.
		if( actors[i].xFramesRemaining == 0 && actors[i].yFramesRemaining==0 && actors[i].yDir==2 ){
			// actors[i].isIdle=1;
			actors[i].x = ((actors[i].x >> 3) << 3);
			actors[i].y = ((actors[i].y >> 3) << 3);

			// Force falling when idle.
			// If not falling or jumping but shouldFall is set to true then make the actor fall.
			if ( actors[i].yDir==2 && shouldFall ){ actors[i].yDir=1; actors[i].yFramesRemaining=255; }
		}

		// Stop the skid.
		if( actors[i].isStopping==1 && actors[i].xFramesRemaining == 0 ){
			actors[i].isStopping=0; actors[i].isMoving=0;
		}
	}

}
void moveFireball(){
	// Simple left or right movement from one side of the screen to the other side.
	// Appears randomly at one of 8 different points on the sides of the screen.
	unsigned char x ;
	unsigned char xDir ;
	unsigned char isMoving ;

	// unsigned char inBounds_goingLeft;
	// unsigned char inBounds_goingRight;
	unsigned char inBounds=false;

	for(unsigned char i=0; i<NUM_ACTORS; i++){
		// There should be only 1 fireball.
		if( actors[i].charType==FIREBALL      ) {
			isMoving = actors[i].isMoving ;
			x        = actors[i].x        ;
			xDir     = actors[i].xDir     ;

			// This allows the fireball to spin for a bit at the start of each crossing.
			if(isMoving==0){ break; }

			// No Xframes left? Not stopped?
			if(actors[i].xFramesRemaining == 0){
				// Bounds check!
				if( xDir==0 && x > BOUNDRY_LEFT_COL  ) { inBounds=true; }
				if( xDir==1 && x < BOUNDRY_RIGHT_COL ) { inBounds=true; }

				// Is the fireball stopped?
				if( actors[i].isStopping==0 ){
					// Has the end has been reached?
					if( !inBounds ){
						// Set the isStopping flag for the fireball.
						actors[i].isStopping=1;
						// Reset the actor's general timer.
						actors[i].vcounter8b_general=0;
					}
					// No? The fireball still has some traveling to do. Give more xFrames.
					else{ actors[i].xFramesRemaining=8; }
				}
			}
			// Has frames left.
			else{
				// Take one away.
				actors[i].xFramesRemaining--;

				// If is moving left:
				if     (xDir==0){ actors[i].x--; }

				// If is moving right:
				else if(xDir==1){ actors[i].x++; }
			}

			// Done with the fireball. We can stop looking for fireballs since there should only be one.
			break;
		}
	}
}

void redrawAllActorSprites(){
	unsigned char nextSpriteNum =0 ;
	const    char * thisCharMap    ;
	unsigned char frameNum         ;

	//
	unsigned char charType            ;
	unsigned char vcounter8b_anim     ;
	unsigned char vcounter8b_general  ;
	unsigned int  vcounter16b_general ;
	// unsigned char isIdle              ;
	unsigned char x                   ;
	unsigned char y                   ;
	unsigned char xDir                ;
	unsigned char yDir                ;
	unsigned char xFramesRemaining    ;
	// unsigned char yFramesRemaining    ;
	unsigned char speed               ;
	unsigned char isDefeated          ;
	// unsigned char isEnemy             ;
	unsigned char whichRow            ;
	unsigned char onBack              ;
	unsigned char isMoving            ;
	unsigned char isStopping          ;
	unsigned char width               ;
	unsigned char height              ;
	unsigned char sprite_bank         ;

	unsigned char doAnim1 = false;
	unsigned char doAnim2 = false;
	unsigned char goFast = false;

	// Go through the list of actors and redraw their sprites.
	for(unsigned char i=0; i<NUM_ACTORS; i++){

		// Only work with on screen actors.
		if( actors[i].isOnScreen !=1 ) { continue; }

		// Redraw the sprites. (only the sprites that are supposed to be on screen.
		charType            = actors[i].charType            ;
		vcounter8b_anim     = actors[i].vcounter8b_anim     ;
		vcounter8b_general  = actors[i].vcounter8b_general  ;
		vcounter16b_general = actors[i].vcounter16b_general ;
		// isIdle              = actors[i].isIdle              ;
		x                   = actors[i].x                   ;
		y                   = actors[i].y                   ;
		xDir                = actors[i].xDir                ;
		yDir                = actors[i].yDir                ;
		xFramesRemaining    = actors[i].xFramesRemaining    ;
		// yFramesRemaining    = actors[i].yFramesRemaining    ;
		// speed               = actors[i].speed               ;
		isDefeated          = actors[i].isDefeated          ;
		// isEnemy             = actors[i].isEnemy             ;
		whichRow            = actors[i].whichRow            ;
		onBack              = actors[i].onBack              ;
		isMoving            = actors[i].isMoving            ;
		isStopping          = actors[i].isStopping          ;
		width               = actors[i].width               ;
		height              = actors[i].height              ;

		doAnim1 = false;
		doAnim2 = false;
		goFast = false;

		// if( actors[i].vcounter8b_general2 < 50 ){ continue; }

		if     ( charType==MARIO || charType==LUIGI ) {
			// This freezes the remaining actors (which should be players) when the phase ends.
			// if( ! ( game.gamestate2==GSTATE2_COINROOM || getEnemyCount_inLocalQueue() ) ){ continue; }

			sprite_bank=SPRITE_BANK0;

			// Set the animation frame and/or reset the animation counter?
			     if(vcounter8b_anim < P_FRAME1_ANIM) { frameNum = 0; }
			else if(vcounter8b_anim < P_FRAME2_ANIM){ frameNum = 1; }
			else if(vcounter8b_anim > P_FRAME2_ANIM){ frameNum = 0; actors[i].vcounter8b_anim = 0;  }

			// Determine which tilemap will be used for the mega sprite.

			// Is defeated?
			if ( isDefeated ) {
				thisCharMap = charType==MARIO ? marioIdle : luigiIdle;

				// Which of the two run frames should be used?
				// if     (charType==MARIO) { thisCharMap = frameNum ? marioRunF1 : marioRunF2 ; }
				// else if(charType==LUIGI) { thisCharMap = frameNum ? luigiRunF1 : luigiRunF2 ; }
			}

			// Jumping or falling?
			else if( yDir !=2 ){ thisCharMap = charType==MARIO ? marioJump : luigiJump; }

			// Stopping?
			else if( isStopping == 1 ){ thisCharMap = charType==MARIO ? marioStop : luigiStop; }

			// Standing idle?
			// ! isMoving

			// Not falling/jumping, no xFrames remain, not holding left/right.
			else if( yDir == 2 && xFramesRemaining==0 && ! (game.btnHeld1 & (BTN_LEFT | BTN_RIGHT))
				){ thisCharMap = charType==MARIO ? marioIdle : luigiIdle; }

			// Moving on X axis only?
			else if( (xDir==0 || xDir==1) && xFramesRemaining){
				// Which of the two run frames should be used?
				if     (charType==MARIO) { thisCharMap = frameNum ? marioRunF1 : marioRunF2 ; }
				else if(charType==LUIGI) { thisCharMap = frameNum ? luigiRunF1 : luigiRunF2 ; }
			}
			// Unmatched? Well, make it something at least.
			else{
				thisCharMap = charType==MARIO ? marioIdle : luigiIdle;
			}

			// Map it.
			MapSprite2( nextSpriteNum, thisCharMap,
				( ( xDir == 1 ? SPRITE_FLIP_X : 0 ) | ( onBack == 1 ? SPRITE_FLIP_Y : 0 ) ) | sprite_bank
			);

			// Move it.
			MoveSprite(nextSpriteNum, x, y, width, height);

			// Adjust next sprite number. (By the number of sprites required by this mega sprite.)
			nextSpriteNum += (1 * height);
		}

		else if( charType==SHELLSCRAPER || charType==SIDESTEPPER || charType==FIGHTERFLY || charType==BONUSWAFFER) {
			sprite_bank=SPRITE_BANK0;
			// Get the next animation frame.

			// If the enemy is on their back.
			// NOTE: if an enemy is in the defeated state then they remain on their back.
			if( onBack ){
				if     (charType==SHELLSCRAPER){
					if     ( actors[i].vcounter16b_general < SHELLSCRAPER_F1_OB ){ thisCharMap = shellscrapperF1 ; } // Frame 1
					else if( actors[i].vcounter16b_general < SHELLSCRAPER_F2_OB ){ thisCharMap = shellscrapperF2 ; } // Frame 2
					else if( actors[i].vcounter16b_general < SHELLSCRAPER_F3_OB ){ thisCharMap = shellscrapperF3 ; } // Frame 3
					else if( actors[i].vcounter16b_general >= SHELLSCRAPER_F3_OB ){
						// Not defeated in time. Turn over (onBack=0) and set the speed to 0 which is the fastest speed.
						thisCharMap = shellscrapperF1 ;
						actors[i].onBack=0;

						// Change to the enemies FAST speed.
						// actors[i].speed=0;
						actors[i].speed=ENEMY_MAX;
						actors[i].speed_counter=0;
						// actors[i].movespeed==MOVESPEED_STOP;
					}
				}
				else if(charType==SIDESTEPPER) {
					if     ( actors[i].vcounter16b_general < SIDESTEPPER_F1_OB ){ thisCharMap = sidestepperF1 ; } // Frame 1
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F2_OB ){ thisCharMap = sidestepperF2 ; } // Frame 2

					else if( actors[i].vcounter16b_general < SIDESTEPPER_F3_OB ){ thisCharMap = sidestepperF1 ; } // Frame 1
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F4_OB ){ thisCharMap = sidestepperF2 ; } // Frame 2
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F5_OB ){ thisCharMap = sidestepperF1 ; } // Frame 1
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F6_OB ){ thisCharMap = sidestepperF2 ; } // Frame 2
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F7_OB ){ thisCharMap = sidestepperF1 ; } // Frame 1
					else if( actors[i].vcounter16b_general < SIDESTEPPER_F8_OB ){ thisCharMap = sidestepperF2 ; } // Frame 2

					else if( actors[i].vcounter16b_general >= SIDESTEPPER_F8_OB ){
						// Not defeated in time. Turn over (onBack=0) and set the speed to 0 which is the fastest speed.
						thisCharMap = sidestepperF1 ;
						actors[i].onBack=0;

						// Change to the enemies FAST speed.
						// actors[i].speed=0;
						// actors[i].speed=0;
						actors[i].speed=ENEMY_MAX;
						actors[i].speed_counter=0;
					}
				}
				else if(charType==FIGHTERFLY)  {
					if     ( actors[i].vcounter16b_general < FIGHTERFLY_F1_OB ){ thisCharMap = fighterflyF2 ; } // Frame 1
					else if( actors[i].vcounter16b_general < FIGHTERFLY_F2_OB ){ thisCharMap = fighterflyF4 ; } // Frame 2
					else if( actors[i].vcounter16b_general >= FIGHTERFLY_F2_OB ){
						// Not defeated in time. Turn over (onBack=0) and set the speed to 0 which is the fastest speed.
						thisCharMap = fighterflyF2 ;
						actors[i].onBack=0;

						// Change to the enemies FAST speed.
						actors[i].speed=ENEMY_MAX;
						actors[i].speed_counter=0;
					}
				}
			}
			// Not on back
			else{
				// Reset the animation counter? (WEIRD! Takes MORE instructions than the else below.)
				// if     ( vcounter8b_anim >=32 ){ actors[i].vcounter8b_anim = 0; }

				//

				if(vcounter8b_anim < E_FRAME1_ANIM) { doAnim1 = true; }
				if(vcounter8b_anim < E_FRAME2_ANIM) { doAnim2 = true; }

				// Character types.
				if     (charType==SHELLSCRAPER){
					// Frame 1
					if      ( doAnim1){ thisCharMap = shellscrapperF1 ; }
					// Frame 2
					else if ( doAnim2){ thisCharMap = shellscrapperF2 ; }
					// Reset the animation counter?
					else                                      { thisCharMap = shellscrapperF1 ; actors[i].vcounter8b_anim = 0; }
				}
				else if     (charType==SIDESTEPPER){
					if      ( doAnim1){ thisCharMap = sidestepperF1 ; }
					else if ( doAnim2){ thisCharMap = sidestepperF2 ; }
					else                                        { thisCharMap = sidestepperF1 ; actors[i].vcounter8b_anim = 0; }
				}
				else if     (charType==FIGHTERFLY){
					if      ( doAnim1){ thisCharMap = fighterflyF1 ; }
					else if ( doAnim2){ thisCharMap = fighterflyF3 ; }
					else                                        { thisCharMap = fighterflyF1 ; actors[i].vcounter8b_anim = 0; }
				}
				else if     (charType==BONUSWAFFER){
					if      ( doAnim1){ thisCharMap = bonusWafferF1 ; }
					else if ( doAnim2){ thisCharMap = bonusWafferF2 ; }
					else                                      { thisCharMap = bonusWafferF1 ; actors[i].vcounter8b_anim = 0; }
				}

				// // Frame 1
				// if     ( vcounter8b_anim < E_FRAME1_ANIM){
				// 	if     (charType==SHELLSCRAPER){ thisCharMap = shellscrapperF1 ; }
				// 	else if(charType==SIDESTEPPER) { thisCharMap = sidestepperF1   ; }
				// 	else if(charType==FIGHTERFLY)  { thisCharMap = fighterflyF1    ; }//1
				// 	else if(charType==BONUSWAFFER) { thisCharMap = bonusWafferF1   ; }

				// }
				// // Frame 2
				// else if( vcounter8b_anim < E_FRAME2_ANIM){
				// 	if     (charType==SHELLSCRAPER){ thisCharMap = shellscrapperF2 ; }
				// 	else if(charType==SIDESTEPPER) { thisCharMap = sidestepperF2   ; }
				// 	else if(charType==FIGHTERFLY)  { thisCharMap = fighterflyF3    ; }//2
				// 	else if(charType==BONUSWAFFER) { thisCharMap = bonusWafferF2   ; }
				// }
				// // Reset the animation counter?
				// else {
				// 	actors[i].vcounter8b_anim = 0;
				// 	if     (charType==SHELLSCRAPER){ thisCharMap = shellscrapperF1 ; }
				// 	else if(charType==SIDESTEPPER) { thisCharMap = sidestepperF1   ; }
				// 	else if(charType==FIGHTERFLY)  { thisCharMap = fighterflyF1    ; }//1
				// 	else if(charType==BONUSWAFFER) { thisCharMap = bonusWafferF1    ; }
				// }
			}

			// DEBUG: Show the row the enemy is on.
			// switch (whichRow) {
			// 	case    0 : thisCharMap = greenS0  ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// 	case    1 : thisCharMap = yellowS1 ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// 	case    2 : thisCharMap = blueS2   ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// 	case    3 : thisCharMap = greenS3  ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// 	case    4 : thisCharMap = bwS4     ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// 	default   : thisCharMap = cursor1  ; xDir=0; sprite_bank=SPRITE_BANK1; width=1;height=1 break ;
			// };

			 // DEBUG: Show the Y tile of the enemy.
			// switch ( (y>>3)+height ) {
			// 	case  0 : { thisCharMap = num_00; break; }
			// 	case  1 : { thisCharMap = num_01; break; }
			// 	case  2 : { thisCharMap = num_02; break; }
			// 	case  3 : { thisCharMap = num_03; break; }
			// 	case  4 : { thisCharMap = num_04; break; }
			// 	case  5 : { thisCharMap = num_05; break; }
			// 	case  6 : { thisCharMap = num_06; break; }
			// 	case  7 : { thisCharMap = num_07; break; }
			// 	case  8 : { thisCharMap = num_08; break; }
			// 	case  9 : { thisCharMap = num_09; break; }
			// 	case 10 : { thisCharMap = num_10; break; }
			// 	case 11 : { thisCharMap = num_11; break; }
			// 	case 12 : { thisCharMap = num_12; break; }
			// 	case 13 : { thisCharMap = num_13; break; }
			// 	case 14 : { thisCharMap = num_14; break; }
			// 	case 15 : { thisCharMap = num_15; break; }
			// 	case 16 : { thisCharMap = num_16; break; }
			// 	case 17 : { thisCharMap = num_17; break; }
			// 	case 18 : { thisCharMap = num_18; break; }
			// 	case 19 : { thisCharMap = num_19; break; }
			// 	case 20 : { thisCharMap = num_20; break; }
			// 	case 21 : { thisCharMap = num_21; break; }
			// 	case 22 : { thisCharMap = num_22; break; }
			// 	case 23 : { thisCharMap = num_23; break; }
			// 	case 24 : { thisCharMap = num_24; break; }
			// 	case 25 : { thisCharMap = num_25; break; }
			// 	case 26 : { thisCharMap = num_26; break; }
			// 	case 27 : { thisCharMap = num_27; break; }
			// 	case 28 : { thisCharMap = num_28; break; }
			// 	case 29 : { thisCharMap = num_29; break; }
			// 	default : { thisCharMap = cursor1; break; }
			// };

			if(i==1){
				// _emu_whisper(0, (y>>3)+height );
				// _emu_whisper(1, whichRow);
			}
			// y= y+ (height-1) *8;

			// xDir=0;
			// sprite_bank=SPRITE_BANK1; width=1;height=1;

			// Map it.
			MapSprite2( nextSpriteNum, thisCharMap,
				( ( xDir == 1 ? SPRITE_FLIP_X : 0 ) | ( onBack == 1 ? SPRITE_FLIP_Y : 0 ) ) | sprite_bank
			);

			// Move it.
			MoveSprite(nextSpriteNum, x, y, width, height);

			// Adjust next sprite number.
			nextSpriteNum += (1 * height);

		}
		else if( charType==FIREBALL     ) {
			sprite_bank=SPRITE_BANK0;

			// Fireball only moves left/right. Disappears after it gets to the other side of the screen.

			// If the fireball is not on screen then just continue.
			if(actors[i].isOnScreen==0){ continue; }

			// Get the next animation frame.
			if     ( vcounter8b_anim < FIREBALL_F1_ANIM ){ thisCharMap = fireballF1 ;}
			else if( vcounter8b_anim < FIREBALL_F2_ANIM ){ thisCharMap = fireballF2 ;}
			else if( vcounter8b_anim < FIREBALL_F3_ANIM ){ thisCharMap = fireballF3 ;}
			else if( vcounter8b_anim < FIREBALL_F4_ANIM ){ thisCharMap = fireballF4 ;}
			// Reset the animation counter?
			else {
				actors[i].vcounter8b_anim = 0;
				thisCharMap = fireballF1;
			}

			// This allows the fireball to spin for a bit at the start of each crossing.
			if(isMoving==0){
				if( vcounter8b_general > FIREBALL_DELAY){ actors[i].isMoving=1; }
			}

			// If isStopping flag is set then the fireball reached the other side of the screen.
			// Let it spin for a little while before removing it.
			if( actors[i].isStopping==1 ){
				if( vcounter8b_general > FIREBALL_DELAY){
					// Remove the fireball. The next draw action will hide it.
					actors[i].vcounter8b_general = 0          ;
					actors[i].isOnScreen         = 0          ;
					actors[i].x                  = OFF_SCREEN ;
					actors[i].charType           = NOENEMY    ;

					spawnActor(FIREBALL);
				}
			}

			// Map it.
			MapSprite2( nextSpriteNum, thisCharMap,
				( ( xDir == 1 ? SPRITE_FLIP_X : 0 ) | ( onBack == 1 ? SPRITE_FLIP_Y : 0 ) ) | sprite_bank
			);
			// Move it.
			MoveSprite(nextSpriteNum, x, y, width, height);
			// Adjust next sprite number.
			nextSpriteNum += (1 * height);

		}
		else if( charType==SLIPICE      ) {
			sprite_bank=SPRITE_BANK0;
			// If the char is not on screen then just continue.
			if(actors[i].isOnScreen==0){
				// actors[i].x=OFF_SCREEN     ;
				// actors[i].charType=NOENEMY ;
				continue;
			}
			// Get the next animation frame.
			if( actors[i].vcounter16b_general < SLIPICE_F1_DELAY ){
				thisCharMap = slipiceF1    ;
			}
			else if( actors[i].vcounter16b_general < SLIPICE_F2_DELAY ){
				thisCharMap = slipiceF2    ;
			}
			else if( actors[i].vcounter16b_general < SLIPICE_F3_DELAY ){
				thisCharMap = slipiceF3    ;
			}
			else if( actors[i].vcounter16b_general < SLIPICE_F4_DELAY ){
				thisCharMap = slipiceF4    ;
			}
			else if( actors[i].vcounter16b_general >= SLIPICE_F4_DELAY ){
				// SLIPICE melted! Freeze this row.
				setRowType(actors[i].whichRow-1, 1); // 0 removes ice. 1 adds ice.

				if(actors[i].whichRow == 1) { game.row1Frozen = 1 ; }
				if(actors[i].whichRow == 2) { game.row2Frozen = 1 ; }
				if(actors[i].whichRow == 3) { game.row3Frozen = 1 ; }

				removeActor(i);

				continue;
			}

			// Map it.
			MapSprite2( nextSpriteNum, thisCharMap,
				( ( xDir == 1 ? SPRITE_FLIP_X : 0 ) | ( onBack == 1 ? SPRITE_FLIP_Y : 0 ) ) | sprite_bank
			);
			// Move it.
			MoveSprite(nextSpriteNum, x, y, width, height);

			// Adjust next sprite number.
			nextSpriteNum += (1 * height);

		}

	}

	// Clear the rest of the sprites in case some were not overwritten.
	for(unsigned char i=nextSpriteNum;i<MAX_SPRITES;i++){
		sprites[i].x=OFF_SCREEN;
		sprites[i].y=OFF_SCREEN;
	}

}
// ACTOR POSITION ADJUSTMENTS

// GAME CONTROL
void newPhase(unsigned char phaseNum){
	// Wrap if the phase number goes too high.
	if(phaseNum>=11){ phaseNum=0; game.phaseNum=0; }

	//
	// PLAY THE END OF PHASE MUSIC
	// RESET PLAYER POSITIONS
	// LOAD NEW PHASE DATA.
	//

	//
	// COPY ENEMY DATA FOR THIS PHASE TO THE "LOCAL" QUEUE.
	//
	// Pointers.
	const unsigned char * src_ptr = &phases_enemies[phaseNum] ;
	unsigned char       * dst_ptr = &phases_enemies_queue     ;

	// Copy the data for this phase from the source array[index] to the local queue.
	// memcpy( dst_ptr, src_ptr, sizeof(phases_enemies[phaseNum]) ) ;
	memcpy( dst_ptr, src_ptr, 6 ) ;

	// Reset sprite/actor system and internal timers (16 bit and the two 8 bit timers.)
	// Clears all actors and sprites and global timers.
	gameInit_actorSystem();

	// RELOAD THE SPRITES/ACTORS FOR THE PLAYERS.
	if(game.player1Lives){ spawnActor(MARIO); }
	if(game.player2Lives){ spawnActor(LUIGI); }

	// Display rows as frozen based on the row frozen statuses.
	setRowType(0, game.row1Frozen ? 1 : 0);
	setRowType(1, game.row2Frozen ? 1 : 0);
	setRowType(2, game.row3Frozen ? 1 : 0);

	// Is this a coin room or a normal phase? Check for all NOENEMY.
	unsigned char enemyFound=false;
	for(unsigned char i=0; i < sizeof(phases_enemies_queue); i++){
		// If there is an enemy then this is NOT a coin room.
		if(phases_enemies_queue[i] != NOENEMY){ enemyFound=true; break; }
	}
	if(enemyFound){
		// Set the gamestate2 to normal.
		game.gamestate2=GSTATE2_NORMAL   ;

		// Hide coins (in case some are there.)
		coinsShow(false);

		// Spawn fireball.
		if( game.hardMode==1){ spawnActor(FIREBALL); }

		// Clear the coin counter. (We are not in a coin room.)
		counter8b_gen1=0;
	}
	else          {
		// Set the gamestate2 to coinroom.
		game.gamestate2=GSTATE2_COINROOM ;

		// Draw the 8 coins to the screen.
		coinsShow(true);

		// Set the coin counter. (There should be 8 total coins.)
		counter8b_gen1=0;

		// Set the phase end timer.
		vsynccounter16b_1=0;
	}

	// Update the displayed phase number.
	printNumber(PRINT_PHASENUM, game.phaseNum+1);

	// Update the other displayed values.
	printNumber(PRINT_PLAYER1SCORE, game.player1Score);
	printNumber(PRINT_PLAYER1LIVES, game.player1Lives);
	printNumber(PRINT_PLAYER2SCORE, game.player2Score);
	printNumber(PRINT_PLAYER2LIVES, game.player2Lives);

	WaitVsync(25);
}
void setRowType(unsigned char row, unsigned char type){
	//

	if     (row==0){
		DrawMap2(0, 7 , type == 0 ? row1_noFreeze : row1_frozen);
		game.row1Frozen = type == 0 ? 0 : 1;
	}
	else if(row==1){
		DrawMap2(0, 13, type == 0 ? row2_noFreeze : row2_frozen);
		game.row2Frozen = type == 0 ? 0 : 1;
	}
	else if(row==2){
		DrawMap2(0, 19, type == 0 ? row3_noFreeze : row3_frozen);
		game.row3Frozen = type == 0 ? 0 : 1;
	}
}
void drawPow( unsigned char POW_state ){
	unsigned char noTile = 0;                         // 0 ; // Black tile.
	unsigned char orange = pgm_read_byte(&(pow2[2])); // 6 ; // Orange
	unsigned char yellow = pgm_read_byte(&(pow3[2])); // 7 ; // Yellow
	unsigned char gray   = pgm_read_byte(&(pow1[2])); // 8 ; // Gray
	unsigned char thisTile ;

	     if( POW_state==0 ){ thisTile = noTile ; }
	else if( POW_state==1 ){ thisTile = gray   ; }
	else if( POW_state==2 ){ thisTile = orange ; }
	else if( POW_state==3 ){ thisTile = yellow ; }
	// Invalid value?
	else{
		return;
	}

	// void Fill(int x,int y,int width,int height,int tile);
	Fill(11-1, 18, 4, 1, thisTile);
}
void powBlockHit(){
	// If the POW is unavailable then end this function.
	if( game.POW_state == 0 ){ return ; }

	// Reduce the POW value by 1.
	game.POW_state--;

	// Redraw the POW.
	drawPow( game.POW_state );

	// Play the bump sound.
	playSFX( SND_BUMPHEAD );

	// Flip over eligible enemies.
	for(unsigned char i=0; i<NUM_ACTORS; i++){
		// Skip some things.
		if     ( !actors[i].isOnScreen                         ){ continue; } // Skip the non-onScreen actors.
		else if( !actors[i].isEnemy                            ){ continue; } // Skip the non-enemies
		else if(  actors[i].charType   == FIREBALL             ){ continue; } // Skip the fireball.
		else if(  actors[i].charType   == BONUSWAFFER          ){ continue; } // Skip the bonuswaffer.
		else if(  actors[i].isDefeated                         ){ continue; } // Skip if the enemy is in the isDefeated state.
		else if(  actors[i].yFramesRemaining                   ){ continue; } // Skip if enemy is jumping or falling.
		else if(  collisionDetect_sprite2Background(i, D_DOWN) ){ continue; } // Skip if the enemy is NOT on the ground. ( EX: if shouldFall==true )

		bumpEnemy(i);
	}
}
void bumpEnemy(unsigned char i){
	// Enemy has been found via player bumping enemy from underneath.
	// POW block has been activated.

	unsigned char doFlipDown = false;

	// If the enemy is a SLIPICE hide and remove immediately.
	if(actors[i].charType==SLIPICE) {
		// Hides the enemy.
		hideActor(i);
		// Removes the enemy from the local queue.
		removeActor(i);
	}

	// Otherwise...
	else{
		// If the enemy is NOT on their back.
		if( ! actors[i].onBack ) {
			if(actors[i].charType==SIDESTEPPER){
				if(actors[i].speed==ENEMY_MAX) {
					// Flip the enemy.
					doFlipDown = true;
				}
				else{
					// Give 8 yFrames.
					actors[i].yFramesRemaining=8;

					// Give bump upward 8 y.
					actors[i].y-=8;

					// Change yDir to 0 (UP).
					actors[i].yDir=0;

					// Realign on X/Y
					// actors[i].x = ((actors[i].x >> 3) << 3) ;
					actors[i].y = ((actors[i].y >> 3) << 3) ;

					// Don't flip the enemy. Just increase the speed!
					actors[i].speed=ENEMY_MAX;
				}
			}
			else{
				// Flip the enemy.
				doFlipDown = true;
			}
		}
		// Enemy was on their back...
		else{
			// Reset counter.
			actors[i].vcounter16b_general=0;

			// Bump y upwards by 8.
			actors[i].y-=8;

			// Set dir to DOWN.
			actors[i].yDir=1;

			// Realign on X/Y
			// actors[i].x = ((actors[i].x >> 3) << 3) ;
			actors[i].y = ((actors[i].y >> 3) << 3) ;

			// Remove remaining xFrames.
			// actors[i].xFramesRemaining=0;

			// Flip the enemy.
			actors[i].onBack = !actors[i].onBack    ;

			// DEBUG: Switch direction.
			// actors[i].xDir = ! actors[i].xDir;
		}

		// If enemy was NOT onBack and doFlipDown was true...
		if(doFlipDown){
			// Reset the 16 bit counter. This will determine when the enemy recovers.
			actors[i].vcounter16b_general=0;

			// Give 8 yFrames.
			actors[i].yFramesRemaining=8;

			// Give bump upward 8 y.
			actors[i].y-=8;

			// Change yDir to 0 (UP).
			actors[i].yDir=0;

			// Flip the enemy.
			actors[i].onBack = !actors[i].onBack    ;

			// Realign on X/Y
			// actors[i].x = ((actors[i].x >> 3) << 3) ;
			actors[i].y = ((actors[i].y >> 3) << 3) ;

			// Remove all xFrames
			// actors[i].xFramesRemaining=0;
		}
	}

}
void assignPoints(unsigned char i, unsigned int newPoints){
	if     (actors[i].charType==MARIO){
		// Provide the player an extra life after reaching 20000 points. (once.)
		if(game.player1Score < 20000 && game.player1Score+newPoints > 20000){ game.player1Lives++; }

		// Add the points and update the points displayed.
		game.player1Score += newPoints;
		printNumber(PRINT_PLAYER1SCORE, game.player1Score);
	}
	else if(actors[i].charType==LUIGI){
		// Provide the player an extra life after reaching 20000 points. (once.)
		if(game.player2Score < 20000 && game.player2Score+newPoints > 20000){ game.player2Lives++; }

		// Add the points and update the points displayed.
		game.player2Score += newPoints;
		printNumber(PRINT_PLAYER2SCORE, game.player2Score);
	}
}
void twinkleCoins(){
	// Not actually sprites. They are normal vram tiles.
	// Swap each instance of coinF1 with coinF2 and vica-versa.
	unsigned char tileid;
	unsigned int vramsize = VRAM_TILES_V*VRAM_TILES_H;
	unsigned char tmp_TcoinF1 = TcoinF1[2]+RAM_TILES_COUNT;
	unsigned char tmp_TcoinF2 = TcoinF2[2]+RAM_TILES_COUNT;

	for(unsigned int tile=0; tile<vramsize; tile++){
		// if( counter8b_gen1 == 0 ) { break; }
		tileid = vram[tile]-RAM_TILES_COUNT;
		if     ( tileid == TcoinF1[2] ){ vram[tile]=tmp_TcoinF2; }
		else if( tileid == TcoinF2[2] ){ vram[tile]=tmp_TcoinF1; }
	}
}
void coinsShow(unsigned char showThem){
	// Draw the 8 coins to the screen.
	if(showThem==true){
		DrawMap2(6 -1, 3 , TcoinF1) ;
		DrawMap2(19-1, 3 , TcoinF2) ;
		DrawMap2(2 -1, 10, TcoinF1) ;
		DrawMap2(23-1, 10, TcoinF2) ;
		DrawMap2(9 -1, 18, TcoinF1) ;
		DrawMap2(16-1, 18, TcoinF2) ;
		DrawMap2(4 -1, 22, TcoinF1) ;
		DrawMap2(21-1, 22, TcoinF2) ;
	}
	// Un-draw the 8 coins to the screen.
	else{
		DrawMap2(6 -1, 3 , blackTile) ;
		DrawMap2(19-1, 3 , blackTile) ;
		DrawMap2(2 -1, 10, blackTile) ;
		DrawMap2(23-1, 10, blackTile) ;
		DrawMap2(9 -1, 18, blackTile) ;
		DrawMap2(16-1, 18, blackTile) ;
		DrawMap2(4 -1, 22, blackTile) ;
		DrawMap2(21-1, 22, blackTile) ;
	}
}
unsigned char checkForEndOfPhase(){
	// If the gamestate is GSTATE2_NORMAL.
	if(game.gamestate2==GSTATE2_NORMAL) {
		// Are all enemies defeated?
		for(unsigned char i=0; i < sizeof(phases_enemies_queue); i++){
			// Return a false if there is anything in the queue other than NOENEMY.
			if(phases_enemies_queue[i] != NOENEMY){ return 0; }
		}
		// The queue only has NOENEMY. Return a true to indicate that the phase is complete.
		return 1;
	}
	// If the gamestate is GSTATE2_COINROOM.
	if(game.gamestate2==GSTATE2_COINROOM) {
		// Are all coins collected?
		if( counter8b_gen1 == 8 ) {
			// Is there still time on the clock? If so, award bonus points to all active players.
			// This was in the arcade game but not the 2600 port. Also, the arcade version had 10 coins.
			if(game.player1Lives){ game.player1Score += 8000; printNumber(PRINT_PLAYER1SCORE, game.player1Score); }
			if(game.player2Lives){ game.player2Score += 8000; printNumber(PRINT_PLAYER2SCORE, game.player2Score); }

			printNumber(PRINT_PHASENUM, 15 - (vsynccounter16b_1/60) );

			// Clears all actors and sprites and global timers.
			gameInit_actorSystem();

			// Draw bonus graphic, wait a moment, then erase it.
			DrawMap2(7-1, 15, popUpMsg_bonus8000);
			WaitVsync(150);
			Fill(7-1, 15, 12, 2, 0x00);

			// Return that the phase is over.
			return 1;
		}

		// Is the timer completed? (60 vsyncs a second times 20 is 1200. Roughly 30 seconds.)
		else if( vsynccounter16b_1 >= COINROOM_MAX_TIME ) {
			printNumber(PRINT_PHASENUM, 0 );

			// Clears all actors and sprites and global timers.
			gameInit_actorSystem();

			// Draw time up graphic, wait a moment, then erase it.
			DrawMap2(9-1, 16, popUpMsg_timeUp);
			WaitVsync(150);
			Fill(9-1, 16, 8, 1, 0x00);

			// Hide the remaining coins.
			coinsShow(false);

			// Return that the phase is over.
			return 1;
		}

		// Phase is not complete.
		else {
			// Return that the phase is NOT over.
			return 0;
		}
	}

	// This shouldn't happen. Allow the phase to continue.
	return 0;
}
// GAME CONTROL

// GAME STATES (game.gamestate1)
void gstate_title(){
	ClearVram();
	newGameInit();

	// Draw the compile time.
	// N782_print( 5 , 27, COMPILEDATETIME2 ) ;
	// WaitVsync(150);

	// unsigned char y_positions[] = { 16, 17, 18 };
	unsigned char y_positions[] = { 16, 17, };
	unsigned char old_ypos = 1;

	unsigned char x_positions[] = { 6-1, 16-1 };
	unsigned char old_xpos = 1;

	// Default cursor position.
	unsigned char ypos = 0;
	unsigned char xpos = 0;

	unsigned char num_y_positions = sizeof(y_positions)-1;
	// unsigned char num_x_positions = sizeof(x_positions)-1;
	unsigned char nextTileId = 2;
	unsigned char titleTileIds[] = { 1, 3, 5};
	unsigned char titleTileIds_index = 0;

	// Reserve a ram tile.
	SetUserRamTilesCount(1);

	// Copy the flash tile id into ram tile id 0.
	CopyFlashTile(titleTileIds[titleTileIds_index]+42, 0);

	// Draw the title screen.
	DrawMap2(1-1, 0, titleScreen);

	// Replace flash tile id 0x32 with ram tile id 0.
	for(unsigned int i=0; i<(VRAM_TILES_H*VRAM_TILES_V); i++){
		if( vram[i]-RAM_TILES_COUNT == 0x32 ){ vram[i] = 0; }
	}

	// Draw the cursor.
	DrawMap2(2-1, y_positions[ypos], TcoinF1);

	// Draw the other cursor.
	Fill(x_positions[xpos], 21, 6, 1, pgm_read_byte(&(cursor1[2])));

	while(game.gamestate1==GSTATE_TITLE){
		WaitVsync(1);
		updateIndividualTimers();
		getInputs();

		// Did the user press a button?
		if( game.btnPressed1 ){
			// Store the previous ypos.
			old_xpos=xpos;
			old_ypos=ypos;

			// Debug exit!
			// if( (game.btnPressed1 & (BTN_SL                  ) ) ){ game.gamestate1 = GSTATE_TITLE; }

			// Menu cursor (game type select)
			if ( (game.btnPressed1 & (BTN_DOWN | BTN_UP ) ) ){
				// Adjust the cursor position value.
				if( (game.btnPressed1 & (BTN_DOWN  ) ) ){ ypos = ypos == num_y_positions ? 0               : ypos+1 ; }
				if( (game.btnPressed1 & (BTN_UP    ) ) ){ ypos = ypos == 0               ? num_y_positions : ypos-1 ; }
			}

			// Menu cursor (game difficulty)
			if ( (game.btnPressed1 & (BTN_LEFT | BTN_RIGHT ) ) ){
				// Adjust the cursor position value.
				if( (game.btnPressed1 & (BTN_LEFT  ) ) ){ xpos = 0; }
				if( (game.btnPressed1 & (BTN_RIGHT ) ) ){ xpos = 1; }
			}

			// STARTS THE GAME!
			if( (game.btnPressed1 & ( BTN_A | BTN_B | BTN_X | BTN_Y | BTN_START) ) ){
				// Valid choice! Set some initial game values.
				if(ypos<4){
					game.gamestate1 = GSTATE_PLAYING;
					game.gamestate2 = GSTATE2_NORMAL;
					// game.phaseNum=1;
				}

				// If the ypos is at one of the game start options...
				if( ypos==0 || ypos==1 || ypos==2 || ypos==3 ){
					// These are always set.
					game.phaseNum     = 0 ;
					game.hardMode     = 0 ;
					game.player1Lives = 0 ;
					game.player2Lives = 0 ;
					game.player1Score = 0 ;
					game.player2Score = 0 ;
					game.numPlayers   = 1 ;
					game.uzenet       = 0 ;

					// Depending on the user's menu choice, these may override the above.

					// Is hardmode set?
					if(xpos==1){ game.hardMode=1; }

					// 1 PLAYER LOCAL
					if     (ypos==0){
						// Some of the default settings above will be used.
						if(game.hardMode==1){ game.player1Lives = 3 ;}
						else                { game.player1Lives = 5 ;}
					}

					// 2 PLAYER LOCAL
					else if(ypos==1){
						game.numPlayers   = 2;

						// Some of the default settings above will be used.
						if(game.hardMode==1){ game.player1Lives = 3 ; game.player2Lives = 3;}
						else                { game.player1Lives = 5 ; game.player2Lives = 5;}
					}

					// 2 PLAYER UZENET
					else if(ypos==2){
						game.numPlayers   = 2;
						game.uzenet       = 1;

						// Some of the default settings above will be used.
						if(game.hardMode==1){ game.player1Lives = 3 ; game.player2Lives = 3;}
						else                { game.player1Lives = 5 ; game.player2Lives = 5;}
					}
				}

				// Wait until the user has released the action button.
				while( game.btnHeld1 ){ WaitVsync(1); getInputs(); }

				// Seems like a good time to seed the random number generator.
				// The seed should vary based on when and how long the user pressed the button.
				srand(vsynccounter16b_1);
			}

		}

		// No? Alternate the cursor frame and the background image color.
		else{
			// Erase the previous cursor if the ypos has changed.
			if(old_xpos != xpos){ Fill(x_positions[old_xpos], 21, 6, 1, 0x00); }
			if(old_ypos != ypos){ vram[ (y_positions[old_ypos]*VRAM_TILES_H) + 2-1]  = 0x00+RAM_TILES_COUNT; }

			// Alternate the cursor frame.
			if     (vsynccounter8b_gen1 < 32){
				DrawMap2(2-1, y_positions[ypos], TcoinF1);
				Fill(x_positions[xpos], 21, 6, 1, pgm_read_byte(&(cursor1[2])));
			}
			else if(vsynccounter8b_gen1 < 64){
				DrawMap2(2-1, y_positions[ypos], TcoinF2);
				Fill(x_positions[xpos], 21, 6, 1, pgm_read_byte(&(cursor1[2])));
			}
			else                             { vsynccounter8b_gen1=0; }

			// Title screen image - tile color changes
			if ( vsynccounter16b_1 > 64 ) {
				if( titleTileIds_index < sizeof( titleTileIds )-1 ){
					// Set the next tile.
					titleTileIds_index++;
				}
				else{
					// Set the next tile.
					titleTileIds_index=0;
				}

				// Reset the timer.
				vsynccounter16b_1=0;

				// Copy the next tile.
				CopyFlashTile( titleTileIds[titleTileIds_index]+42, 0 );
			}

		}
	}

}
void gstate_playing(){
	ClearVram();
	newGameInit();

	// FILL
	// Fill(0,0, VRAM_TILES_H, VRAM_TILES_V, pgm_read_byte(&(offBlackTile[2])) );
	Fill(0,0, VRAM_TILES_H, VRAM_TILES_V, pgm_read_byte(&(blackTile[2])) );
	// WaitVsync(50);


	// memset (vram, 1+RAM_TILES_COUNT, VRAM_TILES_V*VRAM_TILES_H);
	// Border the bottom rows. This is just for debug to indicate the borders.
	// memset (&vram[25*VRAM_TILES_H], 1+RAM_TILES_COUNT, 3*VRAM_TILES_H);
	// DrawMap2(0, 25, offBlackTile);
	// DrawMap2(25, 25, offBlackTile);

	// Draw the normal default map.
	DrawMap2(1-1, 0, levelScreen_default);
	// Fill(9,6, 6, 1, pgm_read_byte(&(offBlackTile[2])) );

	// Removes the "guard" on the top level.
	Fill(9,6, 6, 1, pgm_read_byte(&(blackTile[2])) );

	// Start the phase.
	// game.phaseNum=5;
	newPhase( game.phaseNum );

	// Play the intro for the first phase.
	playSFX( SND_FIRST_PHASE );

	game.POW_state = 3;

	// Display the player scores, player lives, phase number, and the pow block.
	printNumber(PRINT_PLAYER1SCORE, game.player1Score);
	printNumber(PRINT_PLAYER1LIVES, game.player1Lives);
	printNumber(PRINT_PLAYER2SCORE, game.player2Score);
	printNumber(PRINT_PLAYER2LIVES, game.player2Lives);
	drawPow( game.POW_state ) ;

	while(game.gamestate1==GSTATE_PLAYING){

		// _emu_whisper( 1, vsynccounter8b_gen1 );
		//  ****************
		//
		// DEBUGS
		//
		// Look for DEBUG actions.
		if( game.btnPressed1 && 0 ){
			// SL (left shift key) - Return to main menu.
			if( (game.btnPressed1 & (BTN_SL) ) ){ game.gamestate1 = GSTATE_TITLE; return; }

			// SR (right shift key) - Override collision detection. (not handled here.)
			// Used here: collisionDetect_sprite2Background
			if( (game.btnPressed1 & (BTN_SR) ) ){ }

			// START (enter key) - Increase MARIO speed.
			if( (game.btnPressed1 & (BTN_START) ) ){
				for(unsigned char i=0; i<NUM_ACTORS; i++){
					if(actors[i].charType==MARIO){ actors[i].speed++; }
				}
			}

			// SELECT (spacebar key) - Decrease MARIO speed.
			if( (game.btnPressed1 & (BTN_SELECT) ) ){
				for(unsigned char i=0; i<NUM_ACTORS; i++){
					if(actors[i].charType==MARIO){ actors[i].speed--; }
				}
			}

			// Y (q key) - Start next phase.
			if( (game.btnPressed1 & (BTN_Y ) ) ){
				if     (game.gamestate2==GSTATE2_NORMAL  ) { newPhase( ++ game.phaseNum ); }
				else if(game.gamestate2==GSTATE2_COINROOM) { vsynccounter16b_1=COINROOM_MAX_TIME; }
			}

			// X (w key) - Realign all actors on X/Y axis.
			if( (game.btnPressed1 & (BTN_X ) ) ){
				// Set player 1 to have 3 lives.
				// game.player1Lives=3;
				// printNumber(PRINT_PLAYER1LIVES, game.player1Lives);

				// Set 8 coins collected.
				// counter8b_gen1 = 8 ;

				// Realign all actors on X/Y axis.
				for(unsigned char i=0; i<NUM_ACTORS; i++){
					actors[i].x = actors[i].x >> 3 << 3 ; actors[i].y = ((actors[i].y >> 3)-1) << 3 ;
				}

			}

			// B (a key) - Hit the POW block.
			if( (game.btnPressed1 & (BTN_B ) ) ){
				// Hit the POW block.
				powBlockHit();

				// Set POW_state to 3 and redraw the POW block.
				game.POW_state = 3;
				drawPow( game.POW_state ) ;
			}

			// A (s key) -
			if( (game.btnPressed1 & (BTN_A ) ) ){
			}
		}

		// DEBUG: Display the pest queue. (yellow font, bottom center.)
		// debug_displayQueue();

		// DEBUG: Display the row freeze state.
		// debug_displayRowFreezeState();

		// Debug: Count on/off screen actors:
		// unsigned char slotsOnScreen  = 0;
		// for(unsigned char i=0; i<NUM_ACTORS; i++){
		// 	if   ( actors[i].isOnScreen==1 ) { slotsOnScreen ++ ; }
		// }
		// _emu_whisper (0, slotsOnScreen);
		// _emu_whisper (1, NUM_ACTORS);

		// Get input from the gamepads.
		getInputs();

		// Move players.
		movePlayers();

		// This is now handled via pre_VsyncCallBack.
		// Update all sprite positions.
		// redrawAllActorSprites();

		// Rules for normal phases.
		if(game.gamestate2==GSTATE2_NORMAL) { gstate2_normal(); }

		// Rules for the coin room.
		else if(game.gamestate2==GSTATE2_COINROOM) { gstate2_coinRoom(); }

		// Work with actors that are NOT players.
		// for(unsigned char i=0; i<NUM_ACTORS; i++){
		// 	// Skip this iteration if the actor !isEnemy, !isOnScreen, or is NOENEMY
		// 	if     ( !actors[i].isEnemy           ){ continue; }
		// 	else if( !actors[i].isOnScreen        ){ continue; }
		// 	else if(  actors[i].charType==NOENEMY ){ continue; }

		// 	// If they are set to MOVESPEED_FAST then gift them another speed 'tick'
		// 	if      ( actors[i].movespeed==MOVESPEED_STOP   ){ actors[i].speed_counter  = 0; } // Clear the counter.

		// 	// Only add 1 if this would be the second to last speed 'tick'.
		// 	if      (
		// 		   actors[i].movespeed==MOVESPEED_NORMAL
		// 		&& actors[i].speed_counter+2 >= DEFAULT_SPEED_COUNTER_LIMIT
		// 	){ actors[i].speed_counter += 2; }

		// 	else if ( actors[i].movespeed==MOVESPEED_FAST   ){ actors[i].speed_counter += 1; } // Add 1 to the counter.
		// }

		// Keep vsync.
		// getInputs();
		// if( (game.btnHeld1 & (BTN_DOWN ) ) ){ continue; }
		// else{
			WaitVsync(1);
		// }
	}

	// Getting here means that the gamestate1 is no longer GSTATE_PLAYING .
	//

}
// GAME STATES (game.gamestate1)

// GAME STATES 2 (game.gamestate2)
void gstate2_normal(){
	// Should the game end?
	if(game.player1Lives == 0 && game.player2Lives == 0){
		// GAME OVER screen.

		// Clears all actors and sprites and global timers.
		gameInit_actorSystem();

		// Draw the "Game Over" message.
		DrawMap2(8-1, 16, popUpMsg_gaveOver);

		WaitVsync(50);

		// Wait until the user has released the action button.
		while( ! game.btnPressed1 ){ WaitVsync(1); getInputs(); }

		// Change the gamestate1 to return the user to the title screen.
		game.gamestate1 = GSTATE_TITLE;
	}

	// Is the level complete? Check for no enemies remaining.
	else if ( checkForEndOfPhase() ){
		// Play the end of phase sound.
		playSFX( SND_PHASE_END );

		// Delay before the start of a phase.
		WaitVsync(50);

		// Increment game.phaseNum and load the new phase.
		newPhase( ++ game.phaseNum) ;

		// Restart the game loop. Don't finish the current loop.
		// continue;
	}

	// Game should continue. (Move the fireball, move the enemies.)
	else{
		// Move fireball (if there is a fireball.)
		if( game.hardMode==1){ moveFireball(); }

		// Move enemies.
		moveEnemies();
	}
}

void gstate2_coinRoom(){
	// vsynccounter16b_1   : Used for the timer for end of level.
	// vsynccounter8b_gen1 : Used for the timer for the coin twinkle.

	// Should the phase end? (time up or no coins remaining.)
	if ( checkForEndOfPhase() ){
		// Final update of the seconds counter occurs in checkForEndOfPhase.

		// Play end of phase music and wait until it is completed to continue.
		//

		// Restore the POW block.
		game.POW_state = 3;
		drawPow( game.POW_state ) ;

		// Change the gamestate2 back to normal.
		game.gamestate2=GSTATE2_NORMAL;

		// Remove the ice from the frozen floors.
		game.row1Frozen=0;
		game.row2Frozen=0;
		game.row3Frozen=0;

		// Play the end of phase sound.
		playSFX( SND_PHASE_END );

		// Delay before the start of a phase.
		WaitVsync(50);

		// Increment game.phaseNum and load the new phase.
		newPhase( ++ game.phaseNum) ;

		// Restart the game loop. Don't finish the current loop.
		// continue;
	}
	else{
		// Twinkle remaining coins?
		if(vsynccounter8b_gen1 > COINROOM_TWINKLE_DELAY){ twinkleCoins(); vsynccounter8b_gen1=0;}

		// Update the seconds counter?
		printNumber(PRINT_PHASENUM, 15 - (vsynccounter16b_1/60) );

		// There isn't a fireball in the coin rooms.
		// Move fireball (if there is a fireball.)
		// if( game.hardMode==1){ moveFireball(); }
	}
}
unsigned char gstate2_coinRoom_getCoin(unsigned char i, unsigned char tile_id, unsigned char checkedX, unsigned char checkedY){
	unsigned char coinFrame1 = pgm_read_byte(&(TcoinF1[2]));
	unsigned char coinFrame2 = pgm_read_byte(&(TcoinF2[2]));

	// Determine if the sprite is overlapping the coin background tiles.
	if( tile_id == coinFrame1 || tile_id == coinFrame2 ) {
		// Remove the COIN from the screen.
		// vram[ (checkedY*VRAM_TILES_H) + checkedX ] = 0x00+RAM_TILES_COUNT;
		SetTile(checkedX, checkedY, 0x00);

		// Assign points.
		assignPoints(i, COIN_POINTS);

		// Increase the collected coin count.
		counter8b_gen1 ++ ;

		// Play the coin collect sound.
		playSFX( SND_GET_COIN );

		// Return that this IS NOT a collision so that the player can jump through the coin.
		return 1;
	}

	else { return 0; }

}
// GAME STATES 2 (game.gamestate2)

// MAIN
int main(){
	// Initialize the game.
	gameInit();

	// Main loop.
	while(1){
		// Run the correct function based on what the current game state is.
		if      (game.gamestate1==GSTATE_TITLE)   { gstate_title();   }
		else if (game.gamestate1==GSTATE_PLAYING) { gstate_playing(); }

		// This shouldn't happen. There should always be a matching gamestate.
		else{
			WaitVsync(1);
		}

	}

	// __asm__ __volatile__ ("wdr");
	// __asm__ __volatile__ ("wdr");
}
// MAIN

	// __asm__ __volatile__ ("wdr");
	// __asm__ __volatile__ ("wdr");