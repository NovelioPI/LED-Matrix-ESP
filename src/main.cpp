#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"

// Define the main direction for scrolling double height.
// if 1, scroll left; if 0, scroll right
#define SCROLL_LEFT 1

// // Hardware adaptation parameters for scrolling
// bool invertUpperZone = false;
textEffect_t scrollUpper, scrollLower;

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers may not work with your hardware and may need changing
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_ZONES 2
#define ZONE_SIZE 8
#define MAX_DEVICES (NUM_ZONES * ZONE_SIZE)

#define ZONE_UPPER  1
#define ZONE_LOWER  0

#define PAUSE_TIME 0
#define SCROLL_SPEED 50

#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D8

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

const char *msgL[] =
{
  "Selamat Datang",
};
char *msgH; // allocated memory in setup()

void setup(void)
{
  uint8_t max = 0;

#if SCROLL_LEFT // not invert and scroll left
    scrollUpper = PA_SCROLL_LEFT;
    scrollLower = PA_SCROLL_LEFT;
#else           // not invert and scroll right
    scrollUpper = PA_SCROLL_RIGHT;
    scrollLower = PA_SCROLL_RIGHT;
#endif

  // work out the size of buffer required
  for (uint8_t i = 0; i<ARRAY_SIZE(msgL); i++)
    if (strlen(msgL[i]) > max) max = strlen(msgL[i]);

  msgH = (char *)malloc(sizeof(char)*(max + 2));

  // initialise the LED display
  P.begin(NUM_ZONES);

  // Set up zones for 2 halves of the display
  P.setZone(ZONE_LOWER, 0, ZONE_SIZE-1);
  P.setZone(ZONE_UPPER, ZONE_SIZE, MAX_DEVICES-1);
  P.setIntensity(0);
  P.displayClear();
  P.setFont(BigFont); 
  P.setCharSpacing(P.getCharSpacing() * 2); // double height --> double spacing
}

void createHString(char *pH, const char *pL)
{
  for (; *pL != '\0'; pL++)
    *pH++ = *pL | 0x80;   // offset character

  *pH = '\0'; // terminate the string
}

void loop(void)
{
  static uint8_t cycle = 0;

  P.displayAnimate();

  if (P.getZoneStatus(ZONE_LOWER) && P.getZoneStatus(ZONE_UPPER))
  {
    // set up the string
    createHString(msgH, msgL[cycle]);

    P.displayClear();

    P.displayZoneText(ZONE_LOWER, msgL[cycle], PA_LEFT, SCROLL_SPEED, PAUSE_TIME, scrollLower, scrollLower);
    P.displayZoneText(ZONE_UPPER, msgH, PA_LEFT, SCROLL_SPEED, PAUSE_TIME, scrollUpper, scrollUpper);

    // prepare for next pass
    cycle = (cycle + 1) % ARRAY_SIZE(msgL);

    // synchronize the start and run the display
    P.synchZoneStart();
  }
}