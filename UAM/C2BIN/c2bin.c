// We have an array. Read through the thing an output each value as binary to a file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "screendata.c"

// This is for the result binary file.

// Bubble Bobble rounds (only)
// #include "C2BIN_BB01A.inc"

// Bubble Bobble rounds (level data only)
// #include "C2BIN_BBLEVELDATA.inc"


// Some maps from textscreens are bins.
// #include "C2BIN_bb_textscreens.inc"

// Ram tile Maps (only)
// #include "C2BIN_RTMAP_tatio2.inc"

// Streaming music
// #include "C2BIN_streamingmusic.inc"

// ****** INNER C2BIN CONFIG.
#define FNAME_SPIRAM        "BINS/SPIRAMSR.BIN" // File 7: spiramSR.bin : FNAME_SPIRAM:        Holds the combined SD data as one file.
#define FNAME_defineSPIRAM  "BINS/defineSR.def" // File 8: defineSR.def : FNAME_defineSPIRAM:  Holds defines for SPI RAM usage.

FILE * F_SPIRAM        ;
FILE * F_defineSPIRAM  ;

unsigned long musicDataSize = 0;

#define PROGMEM

// *******************
void bb_createFile_gamesave(FILE * defs, FILE * dest){
	// Fill the gamesave file with default (blank?) data.
	unsigned char gameSave[512]    = "0123456789ABCDEF";//{0,0,1,0,0};
	unsigned int i=0;
	for(i=16; i < sizeof(gameSave); i++){ gameSave[i] = 'X'; }
	fwrite(gameSave , 1 , sizeof(gameSave) , dest );
}

/*
void bb_createFile_levelTiles   (FILE * defs, FILE * dest){
	// Create the level graphic data.

	// Use this data structure to store the values needed for each level map.
	struct maplist_ {
		const char    name[15];
		const char    * tiles;
		const char    * map;
	};
	struct maplist_ maplist[] = {
			{ "ROUND_BB01_A",BB01A_tiles,BB01A_map } ,
			{ "ROUND_BB02_A",BB02A_tiles,BB02A_map } ,
			{ "ROUND_BB03_A",BB03A_tiles,BB03A_map } ,
			{ "ROUND_BB04_A",BB04A_tiles,BB04A_map } ,
			{ "ROUND_BB05_A",BB05A_tiles,BB05A_map } ,
			{ "ROUND_BB06_A",BB06A_tiles,BB06A_map } ,
			{ "ROUND_BB07_A",BB07A_tiles,BB07A_map } ,
			{ "ROUND_BB08_A",BB08A_tiles,BB08A_map } ,
			{ "ROUND_BB09_A",BB09A_tiles,BB09A_map } ,
			{ "ROUND_BB10_A",BB10A_tiles,BB10A_map } ,
			{ "ROUND_BB11_A",BB11A_tiles,BB11A_map } ,
			{ "ROUND_BB12_A",BB12A_tiles,BB12A_map } ,
			{ "ROUND_BB13_A",BB13A_tiles,BB13A_map } ,
			{ "ROUND_BB14_A",BB14A_tiles,BB14A_map } ,
			{ "ROUND_BB15_A",BB15A_tiles,BB15A_map } ,
			{ "ROUND_BB16_A",BB16A_tiles,BB16A_map } ,
			{ "ROUND_BB17_A",BB17A_tiles,BB17A_map } ,
			{ "ROUND_BB18_A",BB18A_tiles,BB18A_map } ,
			{ "ROUND_BB19_A",BB19A_tiles,BB19A_map } ,
			{ "ROUND_BB20_A",BB20A_tiles,BB20A_map } ,
			{ "ROUND_BB21_A",BB21A_tiles,BB21A_map } ,
			{ "ROUND_BB22_A",BB22A_tiles,BB22A_map } ,
			{ "ROUND_BB23_A",BB23A_tiles,BB23A_map } ,
			{ "ROUND_BB24_A",BB24A_tiles,BB24A_map } ,
			{ "ROUND_BB25_A",BB25A_tiles,BB25A_map } ,
			{ "ROUND_BB26_A",BB26A_tiles,BB26A_map } ,
			{ "ROUND_BB27_A",BB27A_tiles,BB27A_map } ,
			{ "ROUND_BB28_A",BB28A_tiles,BB28A_map } ,
			{ "ROUND_BB29_A",BB29A_tiles,BB29A_map } ,
			{ "ROUND_BB30_A",BB30A_tiles,BB30A_map } ,
			{ "ROUND_BB31_A",BB31A_tiles,BB31A_map } ,
			{ "ROUND_BB32_A",BB32A_tiles,BB32A_map } ,
			{ "ROUND_BB33_A",BB33A_tiles,BB33A_map } ,
			{ "ROUND_BB34_A",BB34A_tiles,BB34A_map } ,
			{ "ROUND_BB35_A",BB35A_tiles,BB35A_map } ,
			{ "ROUND_BB36_A",BB36A_tiles,BB36A_map } ,
			{ "ROUND_BB37_A",BB37A_tiles,BB37A_map } ,
			{ "ROUND_BB38_A",BB38A_tiles,BB38A_map } ,
			{ "ROUND_BB39_A",BB39A_tiles,BB39A_map } ,
			{ "ROUND_BB40_A",BB40A_tiles,BB40A_map } ,
			{ "ROUND_BB41_A",BB41A_tiles,BB41A_map } ,
			{ "ROUND_BB42_A",BB42A_tiles,BB42A_map } ,
			{ "ROUND_BB43_A",BB43A_tiles,BB43A_map } ,
			{ "ROUND_BB44_A",BB44A_tiles,BB44A_map } ,
			{ "ROUND_BB45_A",BB45A_tiles,BB45A_map } ,
			{ "ROUND_BB46_A",BB46A_tiles,BB46A_map } ,
			{ "ROUND_BB47_A",BB47A_tiles,BB47A_map } ,
			{ "ROUND_BB48_A",BB48A_tiles,BB48A_map } ,
			{ "ROUND_BB49_A",BB49A_tiles,BB49A_map } ,
			{ "ROUND_BB50_A",BB50A_tiles,BB50A_map } ,
		};

	//

	// Get the size of a level tile map.
	unsigned int mapsize  = sizeof(BB01A_map)   ;

	// Get the number of tiles within the level map.
	unsigned int tilesize = sizeof(BB01A_tiles)/2 ; // There should be 2 tiles. We only need one.)

	// Get the number of bytes that a level record will have.
	unsigned int recordSize = mapsize + tilesize;

	// Count the number of maps that are defined.
	unsigned int maplist_size = sizeof(maplist) / sizeof(maplist[0]);

	// Save the size of the round.
	fprintf(defs, "#define ROUNDSIZE \t\t%d\t// Bytes per level tile/map. \n", recordSize  );
	fprintf(defs, "#define NUMROUNDS \t\t%d\t// Number of levels present. \n", maplist_size  );
	fprintf(defs, "\n");

	unsigned char thismap = 0;
	for(thismap=0; thismap<maplist_size; thismap++){
		// Defs line.
		fprintf(defs, "#define %s\t%lu\t\t// Level %d of %d \n", maplist[thismap].name, ftell(dest)+musicDataSize, thismap+1, maplist_size );

		// Write the tile. (Only write the first tile.)
		// There should be two tiles. The second tile is not written.
		fwrite(maplist[thismap].tiles, 1, tilesize , dest );

		// Write the map.
		fwrite(maplist[thismap].map, 1, mapsize , dest );
	}

}

unsigned char countUniques(unsigned long size, const char * map ){
	int i, j;
	unsigned int uniCount=0;

	// printf("SIZE:  %lu:      ", size);

	for(i = 0; i < size; i++) {
		for (j=0; j<i; j++){
				if ( (unsigned char) map[i] == (unsigned char) map[j]) {  break; }
		}

		if (i == j){
				// No duplicate element found between index 0 to i
				// uniCount++;
				// printf("%d, ", (unsigned char) map[i]);
		// }
	 // }

	// printf("\n ");

	// return uniCount;
// }
*/

/*
void bb_createFile_flashtilemaps(FILE * defs, FILE * dest){
	// Write the offset to the defs file.
	// Write the data to the data file.

	fprintf(defs, "\n\n// C2BIN Offsets Definitions File: bubbleBobble_flashtile_maps\n");

	struct maplist_ {
	 const char name[35] ;
	 unsigned long size;
	 const char * map;
	 unsigned int uniqueTiles;
	};

	struct maplist_ maplist[] = {
		{ .name="SD_FT_MAP_mainmenu1              ", .size=sizeof(mainmenu1),               .map=mainmenu1               , .uniqueTiles=countUniques(sizeof(mainmenu1)              , mainmenu1               ) },
		{ .name="SD_FT_MAP_mainmenu2              ", .size=sizeof(mainmenu2),               .map=mainmenu2               , .uniqueTiles=countUniques(sizeof(mainmenu2)              , mainmenu2               ) },
		{ .name="SD_FT_MAP_go_screen              ", .size=sizeof(go_screen),               .map=go_screen               , .uniqueTiles=countUniques(sizeof(go_screen)              , go_screen               ) },
		{ .name="SD_FT_MAP_startIntroText         ", .size=sizeof(startIntroText),          .map=startIntroText          , .uniqueTiles=countUniques(sizeof(startIntroText)         , startIntroText          ) },
		{ .name="SD_FT_MAP_titletext_text         ", .size=sizeof(titletext_text),          .map=titletext_text          , .uniqueTiles=countUniques(sizeof(titletext_text)         , titletext_text          ) },
		{ .name="SD_FT_MAP_mainmenu1_super        ", .size=sizeof(mainmenu1_super),         .map=mainmenu1_super         , .uniqueTiles=countUniques(sizeof(mainmenu1_super)        , mainmenu1_super         ) },
		{ .name="SD_FT_MAP_mainmenu2_debug        ", .size=sizeof(mainmenu2_debug),         .map=mainmenu2_debug         , .uniqueTiles=countUniques(sizeof(mainmenu2_debug)        , mainmenu2_debug         ) },
		{ .name="SD_FT_MAP_endCredits1            ", .size=sizeof(endCredits1),             .map=endCredits1             , .uniqueTiles=countUniques(sizeof(endCredits1)            , endCredits1             ) },
		{ .name="SD_FT_MAP_endCredits2            ", .size=sizeof(endCredits2),             .map=endCredits2             , .uniqueTiles=countUniques(sizeof(endCredits2)            , endCredits2             ) },
		{ .name="SD_FT_MAP_text_happyEnd          ", .size=sizeof(text_happyEnd),           .map=text_happyEnd           , .uniqueTiles=countUniques(sizeof(text_happyEnd)          , text_happyEnd           ) },
		{ .name="SD_FT_MAP_bigHeart               ", .size=sizeof(bigHeart),                .map=bigHeart                , .uniqueTiles=countUniques(sizeof(bigHeart)               , bigHeart                ) },
		{ .name="SD_FT_MAP_parents                ", .size=sizeof(parents),                 .map=parents                 , .uniqueTiles=countUniques(sizeof(parents)                , parents                 ) },
		{ .name="SD_FT_MAP_badEndingText          ", .size=sizeof(badEndingText),           .map=badEndingText           , .uniqueTiles=countUniques(sizeof(badEndingText)          , badEndingText           ) },
		{ .name="SD_FT_MAP_titleScreen_bubbleField", .size=sizeof(titleScreen_bubbleField), .map=titleScreen_bubbleField , .uniqueTiles=countUniques(sizeof(titleScreen_bubbleField), titleScreen_bubbleField ) },
		{ .name="SD_FT_MAP_musicTest1             ", .size=sizeof(musicTest1),              .map=musicTest1              , .uniqueTiles=countUniques(sizeof(musicTest1)             , musicTest1              ) },
		{ .name="SD_FT_MAP_twoRowCurtains         ", .size=sizeof(twoRowCurtains),          .map=twoRowCurtains          , .uniqueTiles=countUniques(sizeof(twoRowCurtains)         , twoRowCurtains          ) },
	};

	unsigned int maplist_size = sizeof(maplist) / sizeof(maplist[0]);

	for(unsigned char i=0; i<maplist_size; i+=1){
		// Write the bbdefs.def line.
		fprintf(defs, "#define %s %lu \t// -- Map bytes: %lu (Unique tile ids: %u) \n", maplist[i].name, ftell(dest)+musicDataSize, maplist[i].size, maplist[i].uniqueTiles );
		// Write the data to the individual file.
		fwrite(maplist[i].map, 1, maplist[i].size, dest);
	}
}
void bb_createFile_ramtilemaps  (FILE * defs, FILE * dest){
	fprintf(defs, "\n\n// C2BIN Offsets Definitions File: bubbleBobble_ramtile_maps\n");

	// Write the offset to the defs file.
	// Write the number of tiles.
	// Write the tileset.
	// Write the tilemap.

	struct maplist_ {
	 const char name[30] ;
	 const char * tileset;
	 const char * tilemap;
	 unsigned long tileset_size;
	 unsigned long tilemap_size;
	};

	struct maplist_ maplist[] = {
		{ .name = "RTMAP_tatio2         " , .tileset = C2BIN_RTMAP_tatio2         , .tilemap = RTMAP_tatio2         , .tileset_size = sizeof(C2BIN_RTMAP_tatio2)           , .tilemap_size = sizeof(RTMAP_tatio2)          },
		// { .name = "RTMAP_info1          " , .tileset = C2BIN_RTMAP_info1          , .tilemap = RTMAP_info1          , .tileset_size = sizeof(C2BIN_RTMAP_info1)           , .tilemap_size = sizeof(RTMAP_info1)           },
		{ .name = "RTMAP_gfBubbleF1     " , .tileset = C2BIN_RTMAP_gfBubbleF1     , .tilemap = RTMAP_gfBubbleF1     , .tileset_size = sizeof(C2BIN_RTMAP_gfBubbleF1)      , .tilemap_size = sizeof(RTMAP_gfBubbleF1)      },
		{ .name = "RTMAP_gfBubbleF2     " , .tileset = C2BIN_RTMAP_gfBubbleF2     , .tilemap = RTMAP_gfBubbleF2     , .tileset_size = sizeof(C2BIN_RTMAP_gfBubbleF2)      , .tilemap_size = sizeof(RTMAP_gfBubbleF2)      },
		{ .name = "RTMAP_gfBubbleF3     " , .tileset = C2BIN_RTMAP_gfBubbleF3     , .tilemap = RTMAP_gfBubbleF3     , .tileset_size = sizeof(C2BIN_RTMAP_gfBubbleF3)      , .tilemap_size = sizeof(RTMAP_gfBubbleF3)      },
		{ .name = "RTMAP_gfBubbleF4     " , .tileset = C2BIN_RTMAP_gfBubbleF4     , .tilemap = RTMAP_gfBubbleF4     , .tileset_size = sizeof(C2BIN_RTMAP_gfBubbleF4)      , .tilemap_size = sizeof(RTMAP_gfBubbleF4)      },
		{ .name = "RTMAP_dinos          " , .tileset = C2BIN_RTMAP_dinos          , .tilemap = RTMAP_dinos          , .tileset_size = sizeof(C2BIN_RTMAP_dinos)           , .tilemap_size = sizeof(RTMAP_dinos)           },
		{ .name = "RTMAP_endingPlayersF1" , .tileset = C2BIN_RTMAP_endingPlayersF1, .tilemap = RTMAP_endingPlayersF1, .tileset_size = sizeof(C2BIN_RTMAP_endingPlayersF1) , .tilemap_size = sizeof(RTMAP_endingPlayersF1) },
		{ .name = "RTMAP_endingPlayersF2" , .tileset = C2BIN_RTMAP_endingPlayersF2, .tilemap = RTMAP_endingPlayersF2, .tileset_size = sizeof(C2BIN_RTMAP_endingPlayersF2) , .tilemap_size = sizeof(RTMAP_endingPlayersF2) },
		{ .name = "RTMAP_human          " , .tileset = C2BIN_RTMAP_human          , .tilemap = RTMAP_human          , .tileset_size = sizeof(C2BIN_RTMAP_human)           , .tilemap_size = sizeof(RTMAP_human)           },
		{ .name = "RTMAP_hugs           " , .tileset = C2BIN_RTMAP_hugs           , .tilemap = RTMAP_hugs           , .tileset_size = sizeof(C2BIN_RTMAP_hugs)            , .tilemap_size = sizeof(RTMAP_hugs)            },
		{ .name = "RTMAP_NICKSEN782LOGO1" , .tileset = C2BIN_RTMAP_NICKSEN782LOGO1, .tilemap = RTMAP_NICKSEN782LOGO1, .tileset_size = sizeof(C2BIN_RTMAP_NICKSEN782LOGO1) , .tilemap_size = sizeof(RTMAP_NICKSEN782LOGO1) },
		{ .name = "RTMAP_NICKSEN782LOGO5" , .tileset = C2BIN_RTMAP_NICKSEN782LOGO5, .tilemap = RTMAP_NICKSEN782LOGO5, .tileset_size = sizeof(C2BIN_RTMAP_NICKSEN782LOGO5) , .tilemap_size = sizeof(RTMAP_NICKSEN782LOGO5) },
		{ .name = "RTMAP_NICKSEN782LOGO4" , .tileset = C2BIN_RTMAP_NICKSEN782LOGO4, .tilemap = RTMAP_NICKSEN782LOGO4, .tileset_size = sizeof(C2BIN_RTMAP_NICKSEN782LOGO4) , .tilemap_size = sizeof(RTMAP_NICKSEN782LOGO4) },
		{ .name = "RTMAP_NICKSEN782LOGO3" , .tileset = C2BIN_RTMAP_NICKSEN782LOGO3, .tilemap = RTMAP_NICKSEN782LOGO3, .tileset_size = sizeof(C2BIN_RTMAP_NICKSEN782LOGO3) , .tilemap_size = sizeof(RTMAP_NICKSEN782LOGO3) },
		{ .name = "RTMAP_NICKSEN782LOGO2" , .tileset = C2BIN_RTMAP_NICKSEN782LOGO2, .tilemap = RTMAP_NICKSEN782LOGO2, .tileset_size = sizeof(C2BIN_RTMAP_NICKSEN782LOGO2) , .tilemap_size = sizeof(RTMAP_NICKSEN782LOGO2) },
		{ .name = "RTMAP_ThankYouDance  " , .tileset = C2BIN_RTMAP_ThankYouDance,   .tilemap = RTMAP_ThankYouDance,   .tileset_size = sizeof(C2BIN_RTMAP_ThankYouDance) ,   .tilemap_size = sizeof(RTMAP_ThankYouDance)   },
	};

	unsigned int maplist_size = sizeof(maplist) / sizeof(maplist[0]);
	unsigned long thisRecordSize=0;
	// unsigned int banksize = 0xFF00;

	for(unsigned char i=0; i<maplist_size; i+=1){
		// Defs line.
		fprintf(defs, "#define %s %lu    \t// %d/%d -- Uniques: %lu -- Map bytes: %lu, -- Dims: %d by %d \n", maplist[i].name, ftell(dest)+musicDataSize, i+1, maplist_size, maplist[i].tileset_size/64, maplist[i].tilemap_size,  maplist[i].tilemap[0],  maplist[i].tilemap[1] );

		// Number of tiles.
		fputc ( maplist[i].tileset_size/64, dest );

		// Tilemap.
		fwrite(maplist[i].tilemap, sizeof(const char), maplist[i].tilemap_size, dest);

		// Tileset.
		fwrite(maplist[i].tileset, sizeof(const char), maplist[i].tileset_size, dest);

	}

}
void bb_createFile_levelData    (FILE * defs, FILE * dest){
	// Data used for this function is found within: C2BIN_BBLEVELDATA.inc

	// Write the data for the enemies within each level.
	fprintf(defs, "\n\n// C2BIN Offsets Definitions File: bubbleBobble_rounds_data\n");

	fprintf(defs, "#define LEVELDATASIZE %lu\n", sizeof(levelData[0]) );


	unsigned int levelData_numBytes   = sizeof(levelData) ;
	unsigned int levelData_recordSize = sizeof(levelData[0]);

	struct levelData_ * lev ;

	for(unsigned char i=0; i<levelData_numBytes/levelData_recordSize; i+=1){
			// Get a handle on this level's data.
			lev = &levelData[i];
			fprintf(defs, "#define %s%d\t\t%lu\t// Total bytes: %lu \n", "levelData_", i+1, ftell(dest)+musicDataSize, sizeof(levelData[0]) );

			// Writes the entire record at once.
			fwrite(lev, levelData_recordSize, 1, dest);
	}

}
*/

int main ( int argc, char * argv [] )  {
	// See all values passed on the command line.
	// printf( "argc = %d\n", argc );
	// for( int i = 0; i < argc; ++i ) {
	// 	printf( "argv[ %d ] = %s\n", i, argv[ i ] );
	// }

	// Retrieve the music data size value. Use 0 if a value was not passed.
	if(argv[1]){ musicDataSize = (unsigned long) atol( argv[1] ); }
	else       { musicDataSize = (unsigned long) 0; }

	// printf( "XXX argv[1] = %s\n", argv[1] );
	// printf( "XXX musicDataSize = %d\n", musicDataSize );
	// exit(0);

	// Files:
	// Open the output files.
	F_SPIRAM        = fopen (FNAME_SPIRAM       , "w"); // File 7: spiramSR.bin : FNAME_SPIRAM:        Holds the combined SD data as one file.
	F_defineSPIRAM  = fopen (FNAME_defineSPIRAM , "w"); // File 8: defineSR.def : FNAME_defineSPIRAM:  Holds defines for SPI RAM usage.
	// Padding for the SD and SPI RAM files.
	unsigned int padBytes=0;

	// Write the SPIRAM file.
	fprintf(F_defineSPIRAM, "// C2BIN DEFINITIONS: SPI RAM (%s) \n\n", FNAME_defineSPIRAM);

	unsigned long offset_gamesave = ftell(F_SPIRAM);
	// bb_createFile_gamesave      ( F_defineSPIRAM, F_SPIRAM );

	// unsigned long offset_leveltiles = ftell(F_SPIRAM);
	// bb_createFile_levelTiles    ( F_defineSPIRAM, F_SPIRAM );

	// unsigned long offset_leveldata = ftell(F_SPIRAM);
	// bb_createFile_levelData     ( F_defineSPIRAM, F_SPIRAM );

	// unsigned long offset_flashtilemaps = ftell(F_SPIRAM);
	// bb_createFile_flashtilemaps ( F_defineSPIRAM, F_SPIRAM );

	// unsigned long offset_ramtilemaps = ftell(F_SPIRAM);
	// bb_createFile_ramtilemaps   ( F_defineSPIRAM, F_SPIRAM );

	// Seek to the end of each of the opened files.
	fseek(F_defineSPIRAM,  0L, SEEK_END);

	fseek(F_SPIRAM,        0L, SEEK_END);
	// padBytes = ftell(F_SPIRAM)        % 512 ;

	// Pad out bytes until the next 512 byte offset.
	// for(unsigned int i=0; i<padBytes; i++){ fputc ( 'X', F_SPIRAM        ); }

	// Write the section offsets to the end of the define file.
	fprintf(F_defineSPIRAM, "\n\n");
	fprintf(F_defineSPIRAM, "// ***** SECTION OFFSETS *****\n");
	fprintf(F_defineSPIRAM, "#define offset_music         %lu \n", 0UL                                    );
	fprintf(F_defineSPIRAM, "#define offset_gamesave      %lu \n", musicDataSize   + offset_gamesave      );
	// fprintf(F_defineSPIRAM, "#define offset_leveltiles    %lu \n", musicDataSize   + offset_leveltiles    );
	// fprintf(F_defineSPIRAM, "#define offset_leveldata     %lu \n", musicDataSize   + offset_leveldata     );
	// fprintf(F_defineSPIRAM, "#define offset_flashtilemaps %lu \n", musicDataSize   + offset_flashtilemaps );
	// fprintf(F_defineSPIRAM, "#define offset_ramtilemaps   %lu \n", musicDataSize   + offset_ramtilemaps   );
	// fprintf(F_defineSPIRAM, "#define offset_endofspiram   %lu \n", ftell(F_SPIRAM) + musicDataSize        );

	unsigned long totalBinaryUsageBytes  = (ftell(F_SPIRAM) + musicDataSize);
	double totalBinaryUsagePercent = (totalBinaryUsageBytes / (float) 131072UL)*100;

	// Output some summary data to the console.
	printf("\n");
	printf(" ------------------ C2BIN END ------------------\n");
	printf(" ** Compile Date: %s \n", __DATE__);
	printf(" ** Compile Time: %s \n", __TIME__);
	printf(" -----------------------------------------------\n" );
	// printf(" Definition file size: SD offsets    : (%s) : (%lu bytes)\n", FNAME_defineSD, ftell(F_defineSD) );
	printf(" Definition file size: SPIRAM offsets: (%s) : (%lu bytes)\n", FNAME_SPIRAM, ftell(F_defineSPIRAM) );
	printf(" -----------------------------------------------\n" );
	printf(" File size for SPI RAM:\n" );
	printf("  File 1 : (%s) : (%lu bytes)\n", FNAME_SPIRAM, ftell(F_SPIRAM) );
	printf(" -----------------------------------------------\n" );
	printf(" SPIRAM Section Offsets\n" );
	printf(" offset_music         : 0x%05lX (%lu)  \n", 0UL,0UL                                    );
	printf(" offset_gamesave      : 0x%05lX (%lu)  \n", (musicDataSize   + offset_gamesave)      , (musicDataSize   + offset_gamesave)      );
	// printf(" offset_leveltiles    : 0x%05lX (%lu)  \n", (musicDataSize   + offset_leveltiles)    , (musicDataSize   + offset_leveltiles)    );
	// printf(" offset_leveldata     : 0x%05lX (%lu)  \n", (musicDataSize   + offset_leveldata)     , (musicDataSize   + offset_leveldata)     );
	// printf(" offset_flashtilemaps : 0x%05lX (%lu)  \n", (musicDataSize   + offset_flashtilemaps) , (musicDataSize   + offset_flashtilemaps) );
	// printf(" offset_ramtilemaps   : 0x%05lX (%lu)  \n", (musicDataSize   + offset_ramtilemaps)   , (musicDataSize   + offset_ramtilemaps)   );
	printf(" offset_endofspiram   : 0x%05lX (%lu)  \n", (totalBinaryUsageBytes), totalBinaryUsageBytes        );
	printf(" -----------------------------------------------\n" );
	printf(" TOTAL BINARY USAGE   : ( %lu of %lu ) %.2f%% \n", totalBinaryUsageBytes , (131072UL), totalBinaryUsagePercent ) ;
	// printf(" END OF FILE PAD BYTES: %d \n",  padBytes );
	printf(" -----------------------------------------------\n" );

	// Close the output files.
	fclose(F_defineSPIRAM);
	fclose(F_SPIRAM);

	// Final console output string. (Do not change. Used as a token to detect C2BIN completion.)
	printf("// C 2 Bin - DONE!\n");

}
