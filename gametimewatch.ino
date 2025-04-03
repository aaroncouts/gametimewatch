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


#elif defined(ARDUINO_LILYGO_T_DISPLAY)
  // Lilygo T-Display
  const int yPos[6] = {60, 90, 120, 150, 180, 210};
  //const int yPos[6] = {60, 82, 104, 126, 148, 170}; // for debug mode
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

//#elif defined(LILYGO_T_DISPLAY_S3)
#else
  #warning "device type not found"
#endif


const short unsigned int* ic[6] = {i0c, i1c, i2c, i3c, i4c, i5c};
const uint16_t icWidth = ICONWIDTH;
const uint16_t icHeight = ICONHEIGHT;

//randomSeed(analogRead(0));
int initialScreenArray[6][6] = {{5, 0, 0, 0, 0, 0},
                                {5, 0, 0, 0, 0, 0},
                                {5, 0, 0, 0, 0, 0},
                                {5, 0, 0, 0, 0, 0},
                                {5, 0, 0, 0, 0, 0},
                                {5, 0, 0, 0, 0, 0}};
int screenArray[6][6];
                          
unsigned long previousMillis = 0;

int rightUpButtonState;
int rightUpLastButtonState = buttonNotPressed;
int leftDownButtonState;
int leftDownLastButtonState = buttonNotPressed;
//The following variables are unsigned longs because the time, measured in
//milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long rightUpLastDebounceTime = 0; //The last time the output pin was toggled.
unsigned long leftDownLastDebounceTime = 0; //The last time the output pin was toggled.
unsigned long debounceDelay = 20; //The debounce time; increase if the output flickers

int dead = 0;
int score = 0;
int numRounds = 0;
const int verticalPositions = 6; // six vertical positions on screen
const int horizontalPositions = 6; // six horizontal positions on screen
const int theKillPosition = (horizontalPositions - 1); // Your player position / only column where you can get killed
const int maxWallRounds = 6; // there can be up to six rounds of "walls" when in "wall" mode.
const int maxBulletsInWall = 5; // there can be up to five bullets in a "wall" round
const int minChainBullets = 4; // there can be a as few as four bullets 
const int maxChainBullets = 20; // there can be a single round of 20 bullets in a "chain" of bullets.
const int maxBulletsInRound = 30; // the most bullets possible in a round is maxWallBullets * maxWallRounds
int bulletsRemaining = 0;

struct bullet {
  int x;
  int y;
  int prevx;
  int prevy;
  int vx;
  int vy;
  int icon;
  int lastFlag;
};

struct bullet rounds[maxBulletsInRound];

int playeryPos = 0;

const long initialTick = 500;
long tick = initialTick; // milliseconds.  after each tick the bullets will move 
int tickAdjustment = 10; 
int tickLowerLimit = 200;

int highScore = 0;

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

  randomSeed(analogRead(1));
  start();
} // end void setup()


void loop() {
  // First we'll check the left and right buttons.
  int rightUpReading = digitalRead(rightUpButtonPin); //Read the state of the switch into a local variable:
  if (rightUpReading == 0) {
    // When the button is released, regardless of bounce, overwrite the L with blank
    img.fillRect(110,0,20,20,TFT_BLACK);
    img.pushSprite(0,0);
  }
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
        move(-1);

      }
    }
  }
  rightUpLastButtonState = rightUpReading; //Save the reading. Next time through the loop, it'll be the lastButtonState:

  int leftDownReading = digitalRead(leftDownButtonPin); //Read the state of the switch into a local variable:
  if (leftDownReading == 0) {
    // When the button is released, regardless of bounce, overwrite the L with blank
    img.fillRect(100,0,20,20,TFT_BLACK);
    img.pushSprite(0,0);
  }
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
        img.drawString("L",102,0,2);
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


    if (bulletsRemaining == 0) {
      // No bullets left in round!  Start a new round.
      if (int(random(0, 2) == 0)) {
        bulletsRemaining = generateChain();       
      } else {
        bulletsRemaining = generateWalls();
      }
      if (tick >= tickLowerLimit) {
        tick -= tickAdjustment;
        img.drawString(String(tick),102,20,1);
      }
    } else {
      moveBullets();
    }

    img.drawString(String(score),20,0,4);
    drawScreen();

    if (dead == 1) {
      img.drawString("Dang!",95,0,2);
      char highScoreMessage[50];
      if (score > highScore) {
        sprintf(highScoreMessage, "New high score!  %d", score);
        img.drawString(highScoreMessage, 0, 35, 2);
        highScore = score;
      } else {
        sprintf(highScoreMessage, "High score is %d", highScore);
        img.drawString(highScoreMessage, 0, 35, 2);
      }
      img.pushSprite(0,0);
      delay(2000);
      start();
    }
  }
} // end void loop()

void start() {
  score = 0;
  dead = 0;
  tick = initialTick;
  bulletsRemaining = 0;

  int i;
  int j;
  for (i = 0; i < verticalPositions; i++) {
    for (j = 0; j < horizontalPositions; j++) {
      screenArray[i][j] = initialScreenArray[i][j];
    }
  }
  screenArray[playeryPos][(horizontalPositions - 1)] = 5; // put player icon in playeryPos

  img.fillRect(0,0,135,59,TFT_BLACK);
  img.drawString("Get ready...",0,0,4);
  img.pushSprite(0,0);
  delay(2000);
  img.fillRect(0,0,135,30,TFT_BLACK);
  img.drawString("0",20,0,4);
  drawScreen();
} // end void start()

int generateWalls() {
  int bulletsRemainingGW = 0;
  int i; // wall iterator
  int numWalls = int(random(0, maxWallRounds)) + 1;

  for (i = 0; i < numWalls; i++) {
    int wall[6];
    int j; // bullet iterator within wall
    int numBulletsInThisWall = 0;
    // First we're going to randomly generate walls until we get a wall with at least 1 bullet and no more than the max
    while ((numBulletsInThisWall == 0) || (numBulletsInThisWall > maxBulletsInWall)) {
      numBulletsInThisWall = 0;
      for (j = 0; j < verticalPositions; j++) {
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
    int x = (-1 * (i * horizontalPositions)) - i;
    for (j = 0; j < verticalPositions; j++) {
      if (wall[j] == 1) {
        // if it's a bullet and not a blank, create the bullet in the rounds array
        rounds[bulletsRemainingGW].x = x;
        rounds[bulletsRemainingGW].y = j;
        rounds[bulletsRemainingGW].vx = 1;
        rounds[bulletsRemainingGW].icon = 1;
        rounds[bulletsRemainingGW].lastFlag = 1; // Every bullet thinks it's the last bullet in the round, at least for a little while
        if (bulletsRemainingGW > 0) {
          // Since this bullet is the last bullet in the round (at least for now), the previous bullet can't be.
          rounds[(bulletsRemainingGW - 1)].lastFlag = 0;
        }
        bulletsRemainingGW++;
      }
    }
  }
  return (bulletsRemainingGW - 1); // At this point, we added one to bulletsRemainingGW at the end of the loop above but
                                   // did not create a new entry in rounds[].  So we have to subtract one so this function
                                   // returns not the number of bullets, but the index of the last populated entry in 
                                   // rounds[].
} // end int generateWalls()

int generateChain() {
  int numBulletsInThisChain = int(random(minChainBullets, (maxChainBullets + 1)));  
  int j; // bullet iterator
  for (j = 0; j <= numBulletsInThisChain; j++) {
    rounds[j].x = (j * -1);
    rounds[j].y = int(random(0, verticalPositions)); 
    rounds[j].vx = 1;
    rounds[j].icon = 1;
    rounds[j].lastFlag = 0;
  }
  rounds[numBulletsInThisChain].lastFlag = 1;
  return numBulletsInThisChain;
} // end int generateChain


void drawScreen() {
  int ixx, ixy;
  for (ixx = 0; ixx <= 5; ixx++) {
    for (ixy = 0; ixy <= 5; ixy++) {
      img.pushImage((ixx * icWidth), yPos[ixy], icWidth, icHeight, ic[screenArray[ixy][ixx]]);
    }
  }
  img.pushSprite(0,0);
} // end void drawScreen()

void moveBullets() {
  int j = 0;
  do {
    if ((rounds[j].x >= 0) and (rounds[j].x <= theKillPosition)) { 
      // Current bullet position is in-screen.  We'll remove the icon from the current location.
      if (screenArray[rounds[j].y][rounds[j].x] == 4) {
        screenArray[rounds[j].y][rounds[j].x] = 5;
      } else if (screenArray[rounds[j].y][rounds[j].x] == 1) {
        screenArray[rounds[j].y][rounds[j].x] = 0;
        if (rounds[j].x == theKillPosition) { 
          // if the removal location is NOT occupied by the player's icon, score this bullet and remove from count
          score++;
          bulletsRemaining--;
        }
      } else if (rounds[j].x == theKillPosition) {
        // edge case, player has moved into the position occupied by the bullet and essentially erased the bullet
        // by doing so, the screen icon wouldn't be 1 or 4, but we still need to score and remove the bullet
        // from the count.
        score++;
        bulletsRemaining--;
      }
    } // end if

    // advance position of bullet
    rounds[j].x += rounds[j].vx;

    if ((rounds[j].x >= 0) and (rounds[j].x <= theKillPosition)) {
      // New bullet position is in in-screen.  We'll add the icon to the new location.
      if (screenArray[rounds[j].y][rounds[j].x] == 5) {
        screenArray[rounds[j].y][rounds[j].x] = 4;
        if (rounds[j].x == theKillPosition) { 
          // if the new location is occupied by the player's icon, player is dead.
          dead = 1;
        }
      } else if (screenArray[rounds[j].y][rounds[j].x] == 0) {
        screenArray[rounds[j].y][rounds[j].x] = 1;
      }
    }
    j++;
  } while (rounds[j].lastFlag == 0);

} // end void moveBullets()

void move(int direction) {
  if (((playeryPos == 0) && (direction == -1)) || ((playeryPos == 5) && (direction == 1))) {
    return;
  }
  screenArray[playeryPos][5] = 0;
  playeryPos += direction;
  screenArray[playeryPos][5] = 5;
  drawScreen();
} // end void move(int direction)
