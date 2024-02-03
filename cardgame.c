#include <stdio.h>
#include <conio.h>
#include <string.h>

static int scrollPos = 0;


char myText[14]; // Declaration
void initializeMyText() {
    strcpy(myText, "Hello, World!"); // Initialize myText with the string
}

// Function to set the C64 background color
void setScreenColor(char color) {
    *((char*)0xD021) = color;
}

// Function to clear the screen with spaces and set background color
void clearScreen(char bgColor) {
    int i; 
    
    // Fill the screen with spaces (character code 32)
    for (i = 0; i < 1000; ++i) {
        *((char*)0x0400 + i) = 32;
        *((char*)0xD800 + i) = bgColor; // Set color RAM to the background color
    }
}

#include <string.h>

void convertToPETSCII(char* text) {
    int i;
    char character;

    for (i = 0; text[i] != '\0'; ++i) {
        character = text[i];

        // Convert ASCII characters to PETSCII codes
        if (character >= 65 && character <= 90) {
            // Uppercase letters
            text[i] = character + 128;
        } else if (character >= 97 && character <= 122) {
            // Lowercase letters
            text[i] = character - 32;
        } else {
            // Other characters remain unchanged
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


void drawRectangle(int x, int y, int width, int height) {
    int i, j; 

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            // Calculate the screen memory location
            int screenPos = (y + i) * 40 + x + j;
            *((char*)0x0400 + screenPos) = 160; // Set character code for white rectangle
            *((char*)0xD800 + screenPos) = 1;   // Set color RAM to white
            
            // Check for upper left corner to draw number 1
            if (i == 0 && j == 0) {
                *((char*)0x0400 + screenPos) = '1'; // Place number 1 in upper left corner
            }
            
            // Check for lower right corner to draw number 2
            if (i == height - 1 && j == width - 1) {
                *((char*)0x0400 + screenPos) = '2'; // Place number 2 in lower right corner
            }
        }
    }
}


void scrollText(const char* text, char color) {
    int len = strlen(text);
    int i; // Declaration moved to the beginning of the block
    int startPos; // Declaration moved to the beginning of the block

    // Clear the text area before writing
    for (i = 0; i < 40; ++i) {
        *((char*)0x0400 + i) = 32; // Clear previous text with spaces
        *((char*)0xD800 + i) = color; // Set color RAM to specified color
    }

    // Calculate the starting position for displaying the text
    startPos = 50 - (scrollPos % (len + 40));

    // Write the text at the calculated position
    for (i = 0; i < len; ++i) {
        *((char*)0x0400 + (startPos + i*2) % 40) = text[i]; // Write the text
    //    *((char*)0xD800 + (startPos + i * 2) % 40) = color; // Set color RAM to specified color
    }

    scrollPos--; // Increment the iterator for scrolling
}


int main() {
    int i; // Declarations moved to the beginning of the block
    clearScreen(0);

    // Set black background color
    setScreenColor(2);
  
    initializeMyText(); // Call the function to initialize myText
    convertToPETSCII(myText); // Pass myText to the conversion function
  
            for (i = 0; i < 3; ++i) {
            drawRectangle(4 + i * 4, 2, 3, 4); // Adjust the parameters for size and position
        }
  
              for (i = 0; i < 3; ++i) {
            drawRectangle(24 + i * 4, 2, 3, 4); // Adjust the parameters for size and position
        }
  
          for (i = 0; i < 8; ++i) {
            drawRectangle(4 + i * 4, 7, 3, 4); // Adjust the parameters for size and position
        }
              for (i = 0; i < 8; ++i) {
            drawRectangle(4 + i * 4, 9, 3, 4); // Adjust the parameters for size and position
        }
  
                for (i = 0; i < 8; ++i) {
            drawRectangle(4 + i * 4, 11, 3, 4); // Adjust the parameters for size and position
        }
  
                  for (i = 0; i < 8; ++i) {
            drawRectangle(4 + i * 4, 13, 3, 4); // Adjust the parameters for size and position
        }
  
                    for (i = 0; i < 8; ++i) {
            drawRectangle(4 + i * 4, 15, 3, 4); // Adjust the parameters for size and position
        }

    while (1) { // Infinite loop
        scrollText(myText, 3);
      delayHalfSecond();

        // Draw 8 white rectangles horizontally

    }

    return 0;
}
