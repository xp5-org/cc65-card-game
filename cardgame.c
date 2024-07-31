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


#define GRID_ROWS 2
#define GRID_COLS 2
#define CARD_WIDTH 3
#define CARD_HEIGHT 3
#define CARD_HORZ_SPACING 1
#define CARD_VERT_SPACING 2

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




typedef struct {
    int value;       // Card number (1-9 or 10-15)
    int cardType;    // Type of card (regular or special)
    int touched;
    int x, y;        // Pixel location of the top-left corner of the card
    //int boundaries[4]; // Left, Right, Top, Bottom boundaries of the card
} Card;

// Array to hold all the cards
Card grid[GRID_ROWS][GRID_COLS];

// Function prototypes
void initializeScreen(int *startX, int *startY);
void setCard(int row, int col, int cardValue, int cardType);


void initializeScreen(int *startX, int *startY) {
    int xx, yy; 
    // initialize grid of cards
    for (yy = 0; yy < GRID_ROWS; ++yy) {
        for (xx = 0; xx < GRID_COLS; ++xx) {
            // calculate pixel position of the top-left corner of the card
            grid[yy][xx].x = *startX + xx * (CARD_WIDTH + CARD_HORZ_SPACING); 
            grid[yy][xx].y = *startY + yy * (CARD_VERT_SPACING); 
        }
    }
}





void setCard(int row, int col, int cardValue, int cardType) {
    Card *card; // declare pointer to card
    card = &grid[row][col]; // assign pointer to specific card in grid
    card->value = cardValue; // set card value
    card->cardType = cardType; 
}



 

void drawRectangle(int x, int y, int width, int height, int value) {
    int i, j; 
    int screenPos; 
  
    if (value == 0) { // if card value is 0 skip drawing treat as blank or empty
        return;
    }

    for (i = 0; i < height; ++i) { // iterate through the rectangle
        for (j = 0; j < width; ++j) {
            screenPos = (y + i) * 40 + x + j; // calculate the screen memory location
            *((char*)0x0400 + screenPos) = 160; // set character code for white rectangle
            *((char*)0xD800 + screenPos) = 1;   // set color RAM to white
            if (i == 0 && j == 0) { 
                *((char*)0x0400 + screenPos) = value; // put character upper left corner
            }
            if (i == height - 1 && j == width - 1) { 
                *((char*)0x0400 + screenPos) = value; // put character lower right corner
            }
        }
    }
}


void clear_screen(unsigned char fill_char) {
    unsigned char* screen;
    int i;

    screen = (unsigned char*)0x0400; // Starting address of the screen memory
    for (i = 0; i < 1000; ++i) { // 25 rows * 40 columns = 1000 character cells
        screen[i] = fill_char;
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
  
    // calculate column index position
    *col = adjustedX / (CARD_WIDTH * 8 + CARD_HORZ_SPACING * 8);

    // calculate row index position
    *row = adjustedY / (CARD_VERT_SPACING * 8 );
}


unsigned char read_raster_line() {
    return *((unsigned char*)0xD012);
}


int main() {
  char output[34]; // buffer to hold chars for debug message at bottom of screen
  int x = 172;	  // sprite X start position (16-bit)
  byte y = 145;   // sprite Y start position (8-bit)
  byte bgcoll;	  // sprite background collision flags
  byte joy;	  // joystick flags
  int ix, iy;     // counters for card matrix init
  int row, col;   // for holding row & col debug print info
  int random_number;
  int startX = 5, startY = 5; // starting x & y offset for drawing the grid
  int xx, yy;     // Loop counters for drawing the grid
  initializeScreen(&startX, &startY); // initialize screen and get startX and startY
  srand(read_raster_line()); //this doesnt really work on an emulator because the cycles are exact
  srand(8765432); // manually setting a random number , need to fake it with user input later 

  // init card matrix with random values 1-26
    for (iy = 0; iy < GRID_COLS; ++iy) { // columns
        for (ix = 0; ix < GRID_ROWS; ++ix) { // rows
          random_number = rand() % 26 + 1;
            setCard(ix, iy, random_number, 1); // Set each card to have value 27 and card type 1
        } 
    }
  
    // Set some example cards, card type 0 is blankcard
    // int row, int col, int cardValue, int cardType
    //setCard(0, 0, 0, 1);   // Regular card number 1
    //setCard(0, 1, 0, 15); // Special card number 10
    //setCard(0, 2, 0, 15);   // Regular card number 1
    //setCard(8, 0, 3, 15);   // Regular card number 1
  
  
  
  
  memcpy((char*)0x3800, SPRITE_DATA, sizeof(SPRITE_DATA));    // copy sprite pattern to RAM address 0x3800
  POKE(0x400 + 0x3f8 + 0, 0x3800 / 64);    // set sprite #0 shape entry (224)
  VIC.spr_pos[0].x = 172;    // set position and color
  VIC.spr_pos[0].y = 145;    // set position and color
  VIC.spr_color[0] = COLOR_GREEN;
  VIC.spr_ena = 0b00000001; // enable sprite #0
  joy_install (joy_static_stddrv);    // install the joystick driver
  clearScreen(0);
  setScreenColor(2); // set red background color
  
  
  // Iterate over rows & cols and draw a card/rectangle for each entry
      for (yy = 0; yy < GRID_ROWS; ++yy) {
        for (xx = 0; xx < GRID_COLS; ++xx) {
          drawRectangle(grid[yy][xx].x, grid[yy][xx].y, CARD_WIDTH, CARD_HEIGHT, grid[yy][xx].value);
           // printf("C[%d][%d]: X = %d, Y = %d, ID = %d, Type = %d\n",
           //        yy, xx, grid[yy][xx].x, grid[yy][xx].y, grid[yy][xx].value, grid[yy][xx].cardType);
         // printf("%d", grid[yy][xx].value);
        }
    }
   

    while (1) { // Infinite loop
    
    joy = joy_read(0);    // get joystick data
    if (JOY_LEFT(joy)) { x -= 2; }   // move sprite left 2 pixel
    if (JOY_RIGHT(joy)) { x += 2; }  // move sprite right 2 pixel
    if (JOY_UP(joy)) { y -= 2; }     // move sprite up 2 pixel
    if (JOY_DOWN(joy)) { y += 2; }   // move sprite down 2 pixel
    
    waitvsync();    // wait for end of frame
    VIC.spr_pos[0].x = x; // set sprite registers based on x position
    VIC.spr_pos[0].y = y; // set sprite registers based on y position
      
    if (JOY_FIRE(joy)) {
        pixelToRowCol(x, y, &row, &col); // get x/y pixel pos of sprite and return the matching array row & col 
        snprintf(output, sizeof(output), "XY: %03d %03d RC: %03d %03d V: %03d", x, y, row, col, grid[row][col].value);
        putText(1, 24, output, 3); // debug message at bottom of screen
    }

    
    VIC.spr_hi_x = (x & 0x100) ? 1 : 0; // set X coordinate high bit
    
    bgcoll = VIC.spr_bg_coll; // grab and reset collision flags
    
    VIC.spr_color[0] = (bgcoll & 1) ?
      COLOR_YELLOW : COLOR_GREEN; // change color on collision with bgcoll
    }
    return 0;
}
