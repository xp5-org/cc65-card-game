#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <tgi.h>
#include <conio.h>
#include "common.h"
#include <stdio.h>
#include <cbm_petscii_charmap.h>




const char SPRITE_DATA[64] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x00, 0x3C, 0x00, 0x00,
    0x7E, 0x00, 0x00, 0xFF, 0x00, 0x01, 0xFF, 0x80,
    0x03, 0xFF, 0xC0, 0x07, 0xFF, 0xE0, 0x0F, 0xFF,
    0xF0, 0x1F, 0xFF, 0xF8, 0x00, 0xFF, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



// set the background color
void setScreenColor(char color) {
    *((char*)0xD021) = color;
}

// clear the screen with spaces and set background color
void clearScreen(char bgColor) {
    int i; 
    
    // Fill the screen with spaces (character code 32)
    for (i = 0; i < 1000; ++i) {
        *((char*)0x0400 + i) = 32;
        *((char*)0xD800 + i) = bgColor; // Set color RAM to the background color
    }
}


void convertToPETSCII(char* text) {
    int i;
    char character;

    for (i = 0; text[i] != '\0'; ++i) {
        character = text[i];

        // convert ASCII characters to PETSCII codes
        if (character >= 65 && character <= 90) {
            // handle uppercase letters
            text[i] = character + 128;
        } else if (character >= 97 && character <= 122) {
            // handle lowercase letters
            text[i] = character - 32;
        } else {
            // other characters unchanged
        }
    }
}


void delayHalfSecond() {
    // Calculate an approximate delay count for half a second
    // Adjust this value based on your system's speed
    int delayCount = 10000; // This value might need adjustments for accuracy
    
    // Perform a delay using No-op instructions
    while (delayCount > 0) {
        asm("nop"); // Execute a No-op instruction
        
        // Decrease the delay counter
        delayCount--;
    }
}


#define GRID_ROWS 8
#define GRID_COLS 8
#define CARD_WIDTH 3
#define CARD_HEIGHT 4
#define CARD_SPACING 1 // Spacing between each card
#define CARD_VERT_SPACING 2

/// Define a structure for a card
typedef struct {
    int value;       // Card number (1-9 or 10-15)
    int cardType;    // Type of card (regular or special)
    int touched;
    int x, y;        // Pixel location of the top-left corner of the card
    int boundaries[4]; // Left, Right, Top, Bottom boundaries of the card
} Card;

// Array to hold all the cards
Card grid[GRID_ROWS][GRID_COLS];

// Function prototypes
void initializeScreen(int *startX, int *startY);
void setCard(int row, int col, int cardValue, int cardType);


// Define card types
#define REGULAR_CARD 1
#define SPECIAL_CARD 15

// Array to map card values to card types
int cardTypeMapping[16] = {
    REGULAR_CARD,  // Card 0 (not used, assuming cards are numbered from 1 to 15)
    REGULAR_CARD,  // Card 1
    REGULAR_CARD,  // Card 2
    REGULAR_CARD,  // Card 3
    REGULAR_CARD,  // Card 4
    REGULAR_CARD,  // Card 5
    REGULAR_CARD,  // Card 6
    REGULAR_CARD,  // Card 7
    REGULAR_CARD,  // Card 8
    REGULAR_CARD,  // Card 9
    SPECIAL_CARD,  // Card 10 (Special card type)
    SPECIAL_CARD,  // Card 11 (Special card type)
    SPECIAL_CARD,  // Card 12 (Special card type)
    SPECIAL_CARD,  // Card 13 (Special card type)
    SPECIAL_CARD,  // Card 14 (Special card type)
    SPECIAL_CARD   // Card 15 (Special card type)
};

void initializeScreen(int *startX, int *startY) {
    int xx, yy; // Loop counters

    // Initialize grid of cards
    for (yy = 0; yy < GRID_ROWS; ++yy) {
        for (xx = 0; xx < GRID_COLS; ++xx) {
            // Calculate pixel position of the top-left corner of the card
            grid[yy][xx].x = 5 + xx * (CARD_WIDTH + CARD_SPACING); // Offset startX by 50 pixels
            grid[yy][xx].y = 5 + yy * (CARD_VERT_SPACING); // Offset startY by 50 pixels

            // Initialize card properties
           // grid[yy][xx].value = 0; // Initialize card value (replace with actual values)
           // grid[yy][xx].cardType = REGULAR_CARD; // Initialize card type (default to regular)
           // grid[yy][xx].touched = 0; // Initialize touched status
        }
    }

    // Update startX and startY with the coordinates of the top-left corner of the grid
    *startX = 5; // does this do anything?? no because its defined as 5 + xx above
    *startY = 5; // does this do anything??
}



void setCard(int row, int col, int cardValue, int cardType) {
    Card *card; // Declare pointer to Card

    // Assign pointer to specific card in grid
    card = &grid[row][col];

    // Set card value and type
    card->value = cardValue;
    card->cardType = cardType;
}



 

void drawRectangle(int x, int y, int width, int height, int value) {
    int i, j; 
    int screenPos; // Declare screenPos at the beginning
    
    // Check if value is 0 and return early if true
    if (value == 0) {
        return;
    }

    // Iterate through the rectangle area
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            // Calculate the screen memory location
            screenPos = (y + i) * 40 + x + j;
            *((char*)0x0400 + screenPos) = 160; // Set character code for white rectangle
            *((char*)0xD800 + screenPos) = 1;   // Set color RAM to white
            
            // Check for upper left corner to draw number 1
            if (i == 0 && j == 0) {
                *((char*)0x0400 + screenPos) = value; // Place first character of value in upper left corner
            }
            
            // Check for lower right corner to draw number 2
            if (i == height - 1 && j == width - 1) {
                *((char*)0x0400 + screenPos) = value; // Place last character of value in lower right corner
            }
        }
    }
}






void putText(int x, int y, const char* text, char color) {
    int len = strlen(text);
    int i;

    // Calculate the base address for the start position
    char* textBase = (char*)0x0400 + y * 40 + x;
    char* colorBase = (char*)0xD800 + y * 40 + x;

    // Clear the top of screen text area before writing
    //for (i = 0; i < 40; ++i) {
    //    *((char*)0x0400 + i) =  2; // Clear previous text with spaces
    //    *((char*)0xD800 + i) = color; // Set color RAM to specified color
    //}

    // Write the text at the specified position
    for (i = 0; i < len; ++i) {
        *(textBase + i) = text[i]; // Write the text
        *(colorBase + i) = color;  // Set color RAM to specified color
    }
}

void pixelToRowCol(int pixelX, int pixelY, int *row, int *col) {
    // adjust pixel coordinates by startX and startY offsets
    int adjustedX = pixelX - (5 * 8) - 12;  // Adjust for startX
    int adjustedY = pixelY - (5 * 8) - 48;  // Adjust for startY
    // take away for sprite height?
  
    // calculate column index
    *col = adjustedX / (CARD_WIDTH * 8 + CARD_SPACING * 8);

    // calculate row index
    // each card is 2 rows tall, and each row is 8 pixels high
    *row = adjustedY / (CARD_VERT_SPACING * 8 );
}


unsigned char read_raster_line() {
    return *((unsigned char*)0xD012);
}


int main() {
   // int i; // Declarations moved to the beginning of the block
  char output[34];
    // variables
  int x = 172;	// sprite X position (16-bit)
  byte y = 145; // sprite Y position (8-bit)
  byte bgcoll;	// sprite background collision flags
  byte joy;	// joystick flags
  int ix, iy;
      int row, col;
  int random_number;

  
  // begin card grid stuff
     int startX, startY;
    int xx, yy; // Loop counters

    // Initialize screen and get startX and startY
    initializeScreen(&startX, &startY);
    srand(read_raster_line());
    srand(8765432);

    for (iy = 0; iy < GRID_ROWS; ++iy) {
        for (ix = 0; ix < GRID_COLS; ++ix) {
          random_number = rand() % 26 + 1;
            setCard(ix, iy, random_number, 1); // Set each card to have value 27 and card type 1
          //printf("%d\n", random_number);
        }
    }
  
    // Set some example cards, card type 0 is blankcard
    // int row, int col, int cardValue, int cardType
    //setCard(0, 0, 0, 1);   // Regular card number 1
    //setCard(0, 1, 0, 15); // Special card number 10
    //setCard(0, 2, 0, 15);   // Regular card number 1
    //setCard(0, 3, 0, 15);   // Regular card number 1
  
  
  
  // copy sprite pattern to RAM address 0x3800
  memcpy((char*)0x3800, SPRITE_DATA, sizeof(SPRITE_DATA));
  // set sprite #0 shape entry (224)
  POKE(0x400 + 0x3f8 + 0, 0x3800 / 64);
  // set position and color
  VIC.spr_pos[0].x = 172;
  VIC.spr_pos[0].y = 145;
  VIC.spr_color[0] = COLOR_GREEN;
  // enable sprite #0
  VIC.spr_ena = 0b00000001;
  
  // install the joystick driver
  joy_install (joy_static_stddrv);
    clearScreen(0);
  
  

    // Set black background color
    setScreenColor(2);
  
  
  // Iterate over rows & cols and draw a card/rectangle for each entry
      for (yy = 0; yy < GRID_ROWS; ++yy) {
        for (xx = 0; xx < GRID_COLS; ++xx) {
          drawRectangle(grid[yy][xx].x, grid[yy][xx].y, 3, 4, grid[yy][xx].value);
           // printf("C[%d][%d]: X = %d, Y = %d, ID = %d, Type = %d\n",
           //        yy, xx, grid[yy][xx].x, grid[yy][xx].y, grid[yy][xx].value, grid[yy][xx].cardType);
         // printf("%d", grid[yy][xx].value);
        }
    }
   

    while (1) { // Infinite loop
          // get joystick bits
    joy = joy_read(0);
    // move sprite based on joystick
    if (JOY_LEFT(joy)) { x -= 1; }   // move left 1 pixel
    if (JOY_RIGHT(joy)) { x += 1; }  // move right 1 pixel
    if (JOY_UP(joy)) { y -= 1; }     // move up 1 pixel
    if (JOY_DOWN(joy)) { y += 1; }   // move down 1 pixel
    // wait for end of frame
    waitvsync();
    // set sprite registers based on position
    VIC.spr_pos[0].x = x;
    VIC.spr_pos[0].y = y;

    if (JOY_FIRE(joy)) { 
        pixelToRowCol(x, y, &row, &col);
        // format output text for row/col debug at bottom
        snprintf(output, sizeof(output), "XY: %03d %03d RC: %03d %03d V: %03d", x, y, row, col, grid[row][col].value);
        putText(1, 24, output, 3);
    }
    
    // set X coordinate high bit
    VIC.spr_hi_x = (x & 0x100) ? 1 : 0;
    // grab and reset collision flags
    bgcoll = VIC.spr_bg_coll;
    // change color when we collide with background
    VIC.spr_color[0] = (bgcoll & 1) ?
      COLOR_YELLOW : COLOR_GREEN;
    }

    return 0;
}
