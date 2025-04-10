/*
 This sketch demonstrates loading images from arrays stored in program (FLASH) memory.

 Works with TFT_eSPI library here:
 https://github.com/Bodmer/TFT_eSPI

 This sketch does not use/need any fonts at all...

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h (or User_Setup_Select.h for specific boards)
 in the TFT_eSPI library folder.

*/

#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Include the header files that contain the icons
#include "gametimewatch_icons.h"

TFT_eSprite img = TFT_eSprite(&tft);

const int numButtons = 4;
struct buttonConfig {
  int buttonPressed;
  int buttonNotPressed;
  int pin;
  int buttonPinMode;
  int powerPinFlag; // On the T-display, a couple buttons are wired to GPIO pins rather than to the power pin.
                    // I'll be setting those pins to HIGH to provide power to the button.  If this comment stays
                    // here, it worked.
  int powerPin;
};

#if defined(ARDUINO_ESP8266_WEMOS_D1MINI)
  // ESP8266 specific code here
  // For the D1 Mini I had the left button on pin D1 and right on D2
  // Both buttons were wired with pulldown resistors.
  const int yPos[6] = {32, 48, 64, 80, 96, 112};
  // For the D1 Mini + Wemos TFT 1.4 display I'm using the left/down button (pin 0) and right/up (pin 35)
  // and pullup mode
  const struct buttonConfig buttonConfigs[numButtons] = 
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = D1, .buttonPinMode = INPUT, .powerPinFlag = 0 },
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = D2, .buttonPinMode = INPUT, .powerPinFlag = 0 },
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = D3, .buttonPinMode = INPUT, .powerPinFlag = 0 },
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = D4, .buttonPinMode = INPUT, .powerPinFlag = 0 }
  };

  #define ROTATION 2
  #define MAXXDIM 128
  #define MAXYDIM 128

#elif defined(ARDUINO_LILYGO_T_DISPLAY)
  // Lilygo T-Display
  const int yPos[6] = {60, 90, 120, 150, 180, 210};
  //const int yPos[6] = {60, 82, 104, 126, 148, 170}; // for debug mode
  // For the TTGO T-Display I'm using the left/down button (pin 0) and right/up (pin 35)
  // and pullup mode.
  // Also for this board, my Select and Fire buttons are wired directly to GPIO pins rather than the power pin.
  const struct buttonConfig buttonConfigs[numButtons] = {
    { .buttonPressed = LOW, .buttonNotPressed = HIGH, .pin = 35, .buttonPinMode = INPUT_PULLUP, .powerPinFlag = 0 },
    { .buttonPressed = LOW, .buttonNotPressed = HIGH, .pin = 0,  .buttonPinMode = INPUT_PULLUP, .powerPinFlag = 0 },
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = 39, .buttonPinMode = INPUT, .powerPinFlag = 1, .powerPin = 33 },
    { .buttonPressed = HIGH, .buttonNotPressed = LOW, .pin = 17, .buttonPinMode = INPUT, .powerPinFlag = 1, .powerPin = 15 }
  };

  #define ROTATION 0
  #define MAXXDIM 135
  #define MAXYDIM 240

//#elif defined(LILYGO_T_DISPLAY_S3)
#else
  #warning "device type not found"
#endif

const int rightUpButton = 0;
const int leftDownButton = 1;
const int selectButton = 2;
const int fireButton = 3;

struct buttonState {
  int currState;
  int lastState;
  unsigned long lastDebounceTime; //The last time the output pin was toggled.
  int pressedFlag;
};

struct buttonState buttonStates[numButtons] = {
  {.currState = buttonConfigs[rightUpButton].buttonNotPressed, .lastState = buttonConfigs[rightUpButton].buttonNotPressed, .lastDebounceTime = 0, .pressedFlag = 0 },
  {.currState = buttonConfigs[leftDownButton].buttonNotPressed, .lastState = buttonConfigs[leftDownButton].buttonNotPressed, .lastDebounceTime = 0, .pressedFlag = 0 },
  {.currState = buttonConfigs[selectButton].buttonNotPressed, .lastState = buttonConfigs[selectButton].buttonNotPressed, .lastDebounceTime = 0, .pressedFlag = 0 },
  {.currState = buttonConfigs[fireButton].buttonNotPressed, .lastState = buttonConfigs[fireButton].buttonNotPressed, .lastDebounceTime = 0, .pressedFlag = 0 }
};

//The following variables are unsigned longs because the time, measured in
//milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long debounceDelay = 20; //The debounce time; increase if the output flickers
unsigned long previousMillis = 0;

const short unsigned int* ic[6] = {i0c, i1c, i2c, i3c, i4c, i5c};
const uint16_t icWidth = ICONWIDTH;
const uint16_t icHeight = ICONHEIGHT;

struct bullet {
  int x;
  int y;
  //int prevx;
  //int prevy;
  int vx;
  int vy;
  int icon;
  int lastFlag;
};

struct firingSquadConst {
  const int initialScreenArray[6][6];
  const int verticalPositions;
  const int horizontalPositions;
  const int theKillPosition;
  const int maxWallRounds;
  const int maxBulletsInWall;
  const int minChainBullets;
  const int maxChainBullets;
  const int maxBulletsInRound;
  const long initialTick;
  const int tickAdjustment;
  const int tickLowerLimit;
};

struct firingSquadConst firingSquadConsts = {
  .initialScreenArray = {{5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0},
                         {5, 0, 0, 0, 0, 0}},
  .verticalPositions = 6, // six vertical positions on screen
  .horizontalPositions = 6, // six horizontal positions on screen
  .theKillPosition = 5, // Your player X position / only column where you can get killed
  .maxWallRounds = 6, // there can be up to six rounds of "walls" when in "wall" mode.
  .maxBulletsInWall = 5, // there can be up to five bullets in a "wall" round
  .minChainBullets = 4, // there can be a as few as four bullets 
  .maxChainBullets = 20, // there can be a single round of 20 bullets in a "chain" of bullets.
  .maxBulletsInRound = 30, // the most bullets possible in a round is maxWallBullets * maxWallRounds
  .initialTick = 500, 
  .tickAdjustment = 10,
  .tickLowerLimit = 200
};


struct firingSquadVar {
  int screenArray[6][6];
  struct bullet rounds[30]; // should match maxBulletsInRound; can't use struct element value.
                            // If I was a real man I'd use malloc.
  int dead;
  int score;
  int bulletsRemaining;
  int playerYPos;
  long adjustableTick;
  int highScore;
  char gameMode[20];
};

struct firingSquadVar firingSquadVars = {
  .screenArray = {{}},
  .rounds = {},
  .dead = 0,
  .score = 0,
  .bulletsRemaining = 0,
  .playerYPos = 0,
  .adjustableTick = firingSquadConsts.initialTick, // milliseconds.  after each tick the bullets will move 
  .highScore = 0,
  .gameMode = "firingSquad"
};

void setup()
{
  tft.init();

  tft.fillScreen(TFT_BLACK);
  
  // Swap the colour byte order when rendering
  tft.setSwapBytes(true);
  //delay(2000);

  Serial.begin(115200);
  int b;
  for (b = 0; b < numButtons; b++) {
    pinMode(buttonConfigs[b].pin, buttonConfigs[b].buttonPinMode); //Initialize the pushbutton pin as an input.
    if (buttonConfigs[b].powerPinFlag == 1) {
      pinMode(buttonConfigs[b].powerPin, OUTPUT);
      digitalWrite(buttonConfigs[b].powerPin, HIGH);
    }
  }

  tft.setRotation(ROTATION);
  img.createSprite(MAXXDIM,MAXYDIM);
  img.setSwapBytes(true);

  randomSeed(analogRead(1));
  start();
} // end void setup()


void loop() {
  int b;
  for (b = 0; b < numButtons; b++) {
    checkButton(b);
  }
  if (strcmp(firingSquadVars.gameMode, "buttonStatus") == 0) {
    loopButtonStatus();
  } else if (strcmp(firingSquadVars.gameMode, "firingSquad") == 0) {
    loopFiringSquad();
  }
}

void checkButton(int buttonID) {
  // Setting the pressedFlag to zero to ensure only one "pressed" event occurs even if
  // button is held down.
  buttonStates[buttonID].pressedFlag = 0;
  // First we'll check the left and right buttons.
  int reading = digitalRead(buttonConfigs[buttonID].pin); //Read the state of the switch into a local variable:
  //Check to see if you just pressed the button
  //(i.e. the input went from LOW to HIGH), and you've waited long enough
  //since the last press to ignore any noise:
  if (reading != buttonStates[buttonID].lastState) {
    buttonStates[buttonID].lastDebounceTime = millis(); //Reset the debouncing timer
  }
  if ((millis() - buttonStates[buttonID].lastDebounceTime) > debounceDelay) {
    //Whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    if (reading != buttonStates[buttonID].currState) { //If the button state has changed:

      buttonStates[buttonID].currState = reading;

      if (buttonStates[buttonID].currState == buttonConfigs[buttonID].buttonPressed) { //Only toggle the LED if the new button state is high
        buttonStates[buttonID].pressedFlag = 1;
      } else {
        buttonStates[buttonID].pressedFlag = 0;
      }
    }
  }
  buttonStates[buttonID].lastState = reading; //Save the reading. Next time through the loop, it'll be the lastButtonState:
}

void loopButtonStatus() {
  char aaronMessage[50];
  sprintf(aaronMessage, "R/U button: %d", buttonStates[rightUpButton].pressedFlag);
  img.drawString(aaronMessage, 0, 50, 2);
  sprintf(aaronMessage, "L/D button: %d", buttonStates[leftDownButton].pressedFlag);
  img.drawString(aaronMessage, 0, 80, 2);
  sprintf(aaronMessage, "Select button: %d", buttonStates[selectButton].pressedFlag);
  img.drawString(aaronMessage, 0, 110, 2);
  sprintf(aaronMessage, "Fire button: %d", buttonStates[fireButton].pressedFlag);
  img.drawString(aaronMessage, 0, 140, 2);
  img.pushSprite(0,0);
}

void loopFiringSquad() {
  // First check whether the movement buttons were pressed, move if necessary.
  if (buttonStates[rightUpButton].pressedFlag == 1) {
    img.drawString("R",112,0,2);
    img.pushSprite(0,0);
    move(-1);
  } else if (buttonStates[leftDownButton].pressedFlag == 1) {
    img.drawString("L",102,0,2);
    img.pushSprite(0,0);
    move(1);
  } else {
    img.fillRect(100,0,35,20,TFT_BLACK);
    img.pushSprite(0,0);
  }

  // Next, we'll check the game tick.
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= firingSquadVars.adjustableTick) {
    //Serial.print("We're in the tick. lineX:");
    // We're in the game tick.
    previousMillis = currentMillis;


    if (firingSquadVars.bulletsRemaining == 0) {
      // No bullets left in round!  Start a new round.
      if (int(random(0, 2) == 0)) {
        firingSquadVars.bulletsRemaining = generateChain();       
      } else {
        firingSquadVars.bulletsRemaining = generateWalls();
      }
      if (firingSquadVars.adjustableTick >= firingSquadConsts.tickLowerLimit) {
        firingSquadVars.adjustableTick -= firingSquadConsts.tickAdjustment;
        img.drawString(String(firingSquadVars.adjustableTick),102,20,1);
      }
    } else {
      moveBullets();
    }

    img.drawString(String(firingSquadVars.score),20,0,4);
    drawScreen();

    if (firingSquadVars.dead == 1) {
      img.drawString("Dang!",95,0,2);
      char highScoreMessage[50];
      if (firingSquadVars.score > firingSquadVars.highScore) {
        sprintf(highScoreMessage, "New high score! %d", firingSquadVars.score);
        img.drawString(highScoreMessage, 0, 35, 2);
        firingSquadVars.highScore = firingSquadVars.score;
      } else if (firingSquadVars.highScore > 0) {
        sprintf(highScoreMessage, "High score is %d", firingSquadVars.highScore);
        img.drawString(highScoreMessage, 0, 35, 2);
      }
      img.pushSprite(0,0);
      delay(2000);
      start();
    }
  }
} // end void loop()

void start() {
  firingSquadVars.score = 0;
  firingSquadVars.dead = 0;
  firingSquadVars.adjustableTick = firingSquadConsts.initialTick;
  firingSquadVars.bulletsRemaining = 0;

  int i;
  int j;
  for (i = 0; i < firingSquadConsts.verticalPositions; i++) {
    for (j = 0; j < firingSquadConsts.horizontalPositions; j++) {
      firingSquadVars.screenArray[i][j] = firingSquadConsts.initialScreenArray[i][j];
    }
  }
  firingSquadVars.screenArray[firingSquadVars.playerYPos][(firingSquadConsts.horizontalPositions - 1)] = 5; // put player icon in firingSquadVars.playerYPos

  img.fillRect(0,0,135,59,TFT_BLACK);
  img.drawString("Get ready...",0,0,4);
  img.pushSprite(0,0);
  delay(2000);
  img.fillRect(0,0,135,30,TFT_BLACK);
  img.drawString("0",20,0,4);
  drawScreen();
} // end void start()

int generateWalls() {
  int numBulletsInThisWallSet = 0;
  int i; // wall iterator
  int numWalls = int(random(0, firingSquadConsts.maxWallRounds)) + 1;

  for (i = 0; i < numWalls; i++) {
    int wall[6];
    int j; // bullet iterator within wall
    int numBulletsInThisWall = 0;
    // First we're going to randomly generate walls until we get a wall with at least 1 bullet and no more than the max
    while ((numBulletsInThisWall == 0) || (numBulletsInThisWall > firingSquadConsts.maxBulletsInWall)) {
      numBulletsInThisWall = 0;
      for (j = 0; j < firingSquadConsts.verticalPositions; j++) {
        int r = int(random(0, 2)); // 50/50 chance of a y position having a bullet.
        /*Serial.print("newLine j:"); Serial.print(j); Serial.print(" r:"); Serial.print(r); Serial.print("\n");*/
        if (r == 1) {
          numBulletsInThisWall++;
        }
        wall[j] = r;
      }
    }
    
    // since we broke out of the while loop, we must have a qualifying wall.  Let's assign positions to our bullets within this wall.

    // the x position of each wall except the first one is going to be negative (as in, not visible and off to the left)
    // the "- i" at the end is to give a one-position buffer between walls
    int x = (-1 * (i * firingSquadConsts.horizontalPositions)) - i;
    for (j = 0; j < firingSquadConsts.verticalPositions; j++) {
      if (wall[j] == 1) {
        // if it's a bullet and not a blank, create the bullet in the firingSquadVars.rounds array
        firingSquadVars.rounds[numBulletsInThisWallSet].x = x;
        firingSquadVars.rounds[numBulletsInThisWallSet].y = j;
        firingSquadVars.rounds[numBulletsInThisWallSet].vx = 1;
        firingSquadVars.rounds[numBulletsInThisWallSet].icon = 1;
        firingSquadVars.rounds[numBulletsInThisWallSet].lastFlag = 1; // Every bullet thinks it's the last bullet in the round, at least for a little while
        if (numBulletsInThisWallSet > 0) {
          // Since this bullet is the last bullet in the round (at least for now), the previous bullet can't be.
          firingSquadVars.rounds[(numBulletsInThisWallSet - 1)].lastFlag = 0;
        }
        numBulletsInThisWallSet++;
      }
    }
  }
  return (numBulletsInThisWallSet - 1); // At this point, we added one to numBulletsInThisWallSet at the end of the loop above but
                                   // did not create a new entry in firingSquadVars.rounds[].  So we have to subtract one so this function
                                   // returns not the number of bullets, but rather the index of the last populated entry in 
                                   // firingSquadVars.rounds[].
} // end int generateWalls()

int generateChain() {
  int numBulletsInThisChain = int(random(firingSquadConsts.minChainBullets, (firingSquadConsts.maxChainBullets + 1)));  
  int j; // bullet iterator
  for (j = 0; j <= numBulletsInThisChain; j++) {
    firingSquadVars.rounds[j].x = (j * -1);
    firingSquadVars.rounds[j].y = int(random(0, firingSquadConsts.verticalPositions)); 
    firingSquadVars.rounds[j].vx = 1;
    firingSquadVars.rounds[j].icon = 1;
    firingSquadVars.rounds[j].lastFlag = 0;
  }
  firingSquadVars.rounds[numBulletsInThisChain].lastFlag = 1;
  return numBulletsInThisChain;
} // end int generateChain


void drawScreen() {
  int ixx, ixy;
  for (ixx = 0; ixx <= 5; ixx++) {
    for (ixy = 0; ixy <= 5; ixy++) {
      img.pushImage((ixx * icWidth), yPos[ixy], icWidth, icHeight, ic[firingSquadVars.screenArray[ixy][ixx]]);
    }
  }
  img.pushSprite(0,0);
} // end void drawScreen()

void moveBullets() {
  int j = 0;
  do {
    if ((firingSquadVars.rounds[j].x >= 0) and (firingSquadVars.rounds[j].x <= firingSquadConsts.theKillPosition)) { 
      // Current bullet position is in-screen.  We'll remove the icon from the current location.
      if (firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] == 4) {
        firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] = 5;
      } else if (firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] == 1) {
        firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] = 0;
        if (firingSquadVars.rounds[j].x == firingSquadConsts.theKillPosition) { 
          // if the removal location is NOT occupied by the player's icon, firingSquadVars.score this bullet and remove from count
          firingSquadVars.score++;
          firingSquadVars.bulletsRemaining--;
        }
      } else if (firingSquadVars.rounds[j].x == firingSquadConsts.theKillPosition) {
        // edge case, player has moved into the position occupied by the bullet and essentially erased the bullet
        // by doing so, the screen icon wouldn't be 1 or 4, but we still need to firingSquadVars.score and remove the bullet
        // from the count.
        firingSquadVars.score++;
        firingSquadVars.bulletsRemaining--;
      }
    } // end if

    // advance position of bullet
    firingSquadVars.rounds[j].x += firingSquadVars.rounds[j].vx;

    if ((firingSquadVars.rounds[j].x >= 0) and (firingSquadVars.rounds[j].x <= firingSquadConsts.theKillPosition)) {
      // New bullet position is in in-screen.  We'll add the icon to the new location.
      if (firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] == 5) {
        firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] = 4;
        if (firingSquadVars.rounds[j].x == firingSquadConsts.theKillPosition) { 
          // if the new location is occupied by the player's icon, player is firingSquadVars.dead.
          firingSquadVars.dead = 1;
        }
      } else if (firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] == 0) {
        firingSquadVars.screenArray[firingSquadVars.rounds[j].y][firingSquadVars.rounds[j].x] = 1;
      }
    }
    j++;
  } while (firingSquadVars.rounds[j].lastFlag == 0);

} // end void moveBullets()

void move(int direction) {
  if (((firingSquadVars.playerYPos == 0) && (direction == -1)) || ((firingSquadVars.playerYPos == 5) && (direction == 1))) {
    return;
  }
  firingSquadVars.screenArray[firingSquadVars.playerYPos][5] = 0;
  firingSquadVars.playerYPos += direction;
  firingSquadVars.screenArray[firingSquadVars.playerYPos][5] = 5;
  drawScreen();
} // end void move(int direction)
