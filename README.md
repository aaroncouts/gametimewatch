# gametimewatch

## Background
This project is based on the GCE Gametime Watch from 1981, designed by Tom Sloper.  

### Some videos on the watch:
- [Brief demonstration of all four games](https://www.youtube.com/watch?v=-Bf1ShRP0sU&pp=ygUVZ2NlICJnYW1lIHRpbWUiIHdhdGNo)
- [Demonstration of Firing Squad game](https://www.youtube.com/watch?v=c8RhkI4UkEM)
- [Longer demonstration of Missle Strike game](https://www.youtube.com/watch?v=Ct3c1ywy9_s&pp=ygUVZ2NlICJnYW1lIHRpbWUiIHdhdGNo)
- [Designer Tom Sloper speaking at the 2002 Classic Gaming Expo (audio only)](https://www.youtube.com/watch?v=JeE5WQ8B9oU&t=261s&pp=ygUVZ2NlICJnYW1lIHRpbWUiIHdhdGNo0gcJCb0Ag7Wk3p_U)

### Blog entries from the game designer:
- [Game designer Tom Sloper's blog entry "#18: How I Got My Start; The GAME TIME Watch"](https://www.sloperama.com/advice/lesson18.html)
- [Game designer Tom Sloper's blog entry "#68 'Game Time watch' vs. 'Game & Watch'"](https://www.sloperama.com/advice/gamewatch.html)

## About this project

This project is an attempt to replicate the games on the 1981 Game Time Watch designed by Tom Sloper.

### V1

V1 is just the Firing Squad game.  It is running on a Wemos D1 Mini clone (ESP8266) with the Wemos TFT 1.4 Shield.  It's not great:
- The original game had two types of rounds, which I'll call "wall of bullets" and "string of bullets".  Currently only "wall of bullets" is present, and it doesn't behave the same way as the original.
- The hardware platform is not good.
  - ESP8266 works fine but is out of date (ESP32 much more common).
  - D1 Mini doesn't have enough pins (I think).  I need 2-3 more buttons.
  - The Wemos TFT 1.4 Shield isn't available anymore and maybe not very common in the first place.
  - The Wemos TFT shield doesn't play well with the TFT_eSPI library (work in progress).

[Here's a video of V1!](https://www.youtube.com/watch?v=KtHXBW3t9KA)

... So my next step is to implement it on the Lilygo TTGO.  The TTGO has a rectangular screen with the right proportions, and supposedly the two on-board buttons, which are nicely located, can be used for gameplay.


  
