// Icon images are stored in tabs ^ e.g. Alert.h etc.above this line
// more than one icon can be in a header file

// Arrays containing FLASH images can be created with UTFT library tool:
// (libraries\UTFT\Tools\ImageConverter565.exe)
// Convert to .c format then copy into a new tab

/*
 This sketch demonstrates loading images from arrays stored in program (FLASH) memory.

 Works with TFT_eSPI library here:
 https://github.com/Bodmer/TFT_eSPI

 This sketch does not use/need any fonts at all...

 Code derived from ILI9341_due library example

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
*/

#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Include the header files that contain the icons
#include "gametimewatch_icons.h"

TFT_eSprite img = TFT_eSprite(&tft);

#if defined(ARDUINO_ESP8266_WEMOS_D1MINI)
  // ESP8266 specific code here
  // For the D1 Mini I had the left button on pin D1 and right on D2
  // Both buttons were wired with pulldown resistors.
  const int yPos[6] = {32, 48, 64, 80, 96, 112};
  // For the D1 Mini + Wemos TFT 1.4 display I'm using the left/down button (pin 0) and right/up (pin 35)
  // and pullup mode
  const int buttonPressed = HIGH;
  const int buttonNotPressed = LOW;
  const int rightUpButtonPin = D1;
  const int leftDownButtonPin = D2;

  #define ROTATION 2
  #define MAXXDIM 128
  #define MAXYDIM 128
  #define PINMODE INPUT
  const char *aaron = "Get Ready (D1Mini)...";

#elif defined(ARDUINO_LILYGO_T_DISPLAY)
  // Lilygo T-Display
  const int yPos[6] = {60, 90, 120, 150, 180, 210};
  // For the TTGO T-Display I'm using the left/down button (pin 0) and right/up (pin 35)
  // and pullup mode
  const int buttonPressed = LOW;
  const int buttonNotPressed = HIGH;
  const int rightUpButtonPin = 35;
  const int leftDownButtonPin = 0;

  #define ROTATION 0
  #define MAXXDIM 135
  #define MAXYDIM 240
  #define PINMODE INPUT_PULLUP
  const char *aaron = "Get ready (T-Display)...";
//#elif defined(LILYGO_T_DISPLAY_S3)
#else
  #warning "device type not found"
#endif


const short unsigned int* ic[6] = {i0c, i1c, i2c, i3c, i4c, i5c};
const uint16_t icWidth = ICONWIDTH;
const uint16_t icHeight = ICONHEIGHT;

//randomSeed(analogRead(0));
int screenArray[6][6] = {{5, 0, 0, 0, 0, 5},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0}};
                          
unsigned long previousMillis = 0;
const long initialTick = 500; 



int rightUpButtonState;
int rightUpLastButtonState = buttonNotPressed;
int leftDownButtonState;
int leftDownLastButtonState = buttonNotPressed;
//The following variables are unsigned longs because the time, measured in
//milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long rightUpLastDebounceTime = 0; //The last time the output pin was toggled.
unsigned long leftDownLastDebounceTime = 0; //The last time the output pin was toggled.
unsigned long debounceDelay = 20; //The debounce time; increase if the output flickers

void setup()
{
  tft.init();

  tft.fillScreen(TFT_BLACK);
  // Swap the colour byte order when rendering
  tft.setSwapBytes(true);
  //delay(2000);

  Serial.begin(115200);
  pinMode(rightUpButtonPin, PINMODE); //Initialize the pushbutton pin as an input.
  pinMode(leftDownButtonPin, PINMODE); //Initialize the pushbutton pin as an input.

  tft.setRotation(ROTATION);
  img.createSprite(MAXXDIM,MAXYDIM);
  img.setSwapBytes(true);

  drawScreen();
  start();
  randomSeed(analogRead(1));
}

int line[6] = {0,
               0,
               0,
               0,
               0,
               0};

int lineX = 5;
int oldLineX;
int numBullets = 0;
int dead = 0;
int score = 0;
int numRounds = 0;

int playeryPos = 0;

long tick = 500;
int tickAdjustment = 4; 
int tickLowerLimit = 100;

void loop() {
  // First we'll check the left and right buttons.
  int rightUpReading = digitalRead(rightUpButtonPin); //Read the state of the switch into a local variable:
  //Check to see if you just pressed the button
  //(i.e. the input went from LOW to HIGH), and you've waited long enough
  //since the last press to ignore any noise:
  if (rightUpReading != rightUpLastButtonState)
  {
    rightUpLastDebounceTime = millis(); //Reset the debouncing timer
  }
  if ((millis() - rightUpLastDebounceTime) > debounceDelay)
    //Whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
  {
    if (rightUpReading != rightUpButtonState) //If the button state has changed:
    {
      rightUpButtonState = rightUpReading;

      if (rightUpButtonState == buttonPressed) //Only toggle the LED if the new button state is high
      {
        //ledState = !ledState;
        //Serial.print("Right/Up button\n");
        img.drawString("R",112,0,2);
        img.pushSprite(0,0);
        delay(10);
        img.fillRect(110,0,20,20,TFT_BLACK);
        img.pushSprite(0,0);
        move(-1);

      }
    }
  }
  rightUpLastButtonState = rightUpReading; //Save the reading. Next time through the loop, it'll be the lastButtonState:

  int leftDownReading = digitalRead(leftDownButtonPin); //Read the state of the switch into a local variable:
  //Check to see if you just pressed the button
  //(i.e. the input went from LOW to HIGH), and you've waited long enough
  //since the last press to ignore any noise:
  if (leftDownReading != leftDownLastButtonState)
  {
    leftDownLastDebounceTime = millis(); //Reset the debouncing timer
  }
  if ((millis() - leftDownLastDebounceTime) > debounceDelay)
    //Whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
  {
    if (leftDownReading != leftDownButtonState) //If the button state has changed:
    {
      leftDownButtonState = leftDownReading;

      if (leftDownButtonState == buttonPressed) //Only toggle the LED if the new button state is high
      {
        //ledState = !ledState;
        //Serial.print("Left/Down button\n");
        img.drawString("L",77,0,2);
        img.pushSprite(0,0);
        delay(10);
        img.fillRect(75,0,20,20,TFT_BLACK);
        img.pushSprite(0,0);
        move(1);
      }
    }
  }
  leftDownLastButtonState = leftDownReading; //Save the reading. Next time through the loop, it'll be the lastButtonState:

  // Next, we'll check the game tick.
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= tick) {
    //Serial.print("We're in the tick. lineX:");
    // We're in the game tick.
    previousMillis = currentMillis;
    oldLineX = lineX;
    lineX++;
    if (lineX == 6) {
      newLine();
      lineX = 0;

    }
    renderColumn(oldLineX, -1);
    renderColumn(lineX, 1);
    //Serial.print(lineX);
    //Serial.print("  oldLineX:");
    //Serial.print(oldLineX);
    //Serial.print("\n");
    drawScreen();
    if ((numRounds > 0) && (lineX == 5) && (dead == 0)) {
      score += numBullets;
      img.drawString(String(score),20,0,2);
    } else if (dead == 1) {
      img.drawString("Dang!",75,0,2);
      img.pushSprite(0,0);
      delay(2000);
      start();
    }
    numRounds++;
    if (tick >= tickLowerLimit) {
      tick -= tickAdjustment;
    }
  }
}

void start() {
  score = 0;
  dead = 0;
  numRounds = 0;
  tick = initialTick;
  img.fillRect(18,0,110,20,TFT_BLACK);
  //img.drawString("Get ready...",20,0,2);
  img.drawString(aaron,20,0,2);
  img.pushSprite(0,0);
  delay(2000);
  img.fillRect(18,0,110,20,TFT_BLACK);
  img.drawString("0",20,0,2);
  img.pushSprite(0,0);
}

void newLine() {
  int j;
  numBullets = 0;
  while ((numBullets == 0) || (numBullets == 6)) {
    for (j = 0; j <= 5; j++) {
      int r = int(random(0, 2));
      /*Serial.print("newLine j:"); Serial.print(j); Serial.print(" r:"); Serial.print(r); Serial.print("\n");*/
      if (r == 1) {
        numBullets++;
      }
      line[j] = r;
    }
  }
}


void drawScreen() {
  int ixx, ixy;
  for (ixx = 0; ixx <= 5; ixx++) {
    for (ixy = 0; ixy <= 5; ixy++) {
      img.pushImage((ixx * icWidth), yPos[ixy], icWidth, icHeight, ic[screenArray[ixy][ixx]]);
    }
  }
  img.pushSprite(0,0);
}

void renderColumn(int rx, int direction) {
  int ixy;
  for (ixy = 0; ixy <= 5; ixy++) {
    if (direction == -1) {
      // We're removing stuff
      if (screenArray[ixy][rx] == 4) {
        screenArray[ixy][rx] = 5;
      } else if (screenArray[ixy][rx] == 1) {
        screenArray[ixy][rx] = 0;
      }
    } else if ((direction == 1) && (line[ixy] == 1)) {
      // We're adding stuff
      if (screenArray[ixy][rx] == 5) {
        screenArray[ixy][rx] = 4;
        if (rx == 5) {
          dead = 1;
        }
      } else if (screenArray[ixy][rx] == 0) {
        screenArray[ixy][rx] = 1;
      }
    }
  }
}

void move(int direction) {
  if (((playeryPos == 0) && (direction == -1)) || ((playeryPos == 5) && (direction == 1))) {
    return;
  }
  screenArray[playeryPos][5] = 0;
  playeryPos += direction;
  screenArray[playeryPos][5] = 5;
  drawScreen();
}
