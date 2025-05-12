/*
  Mystic Balloon: http://www.team-arg.org/mybl-manual.html

  Arduboy version 1.7.2:  http://www.team-arg.org/mybl-downloads.html

  MADE by TEAM a.r.g. : http://www.team-arg.org/more-about.html

  2016-2018 - GAVENO - CastPixel - JO3RI - Martian220

  Game License: MIT : https://opensource.org/licenses/MIT

*/

//determine the game
#define GAME_ID 34

#include "globals.h"
#include "menu.h"
#include "game.h"
#include "inputs.h"
#include "player.h"
#include "enemies.h"
#include "elements.h"
#include "levels.h"

//typedef void (*FunctionPointer) ();
typedef void (*FunctionPointer) (void);

/*
//const FunctionPointer PROGMEM  mainGameLoop[] = {
//const FunctionPointer mainGameLoop[] = {
FunctionPointer mainGameLoop[] = {
  stateMenuIntro,
  stateMenuMain,
  stateMenuHelp,
  stateMenuPlaySelect,
  stateMenuInfo,
  stateMenuSoundfx,
  stateGameNextLevel,
  stateGamePlaying,
  stateGamePause,
  stateGameOver,
  stateMenuPlayContinue,
  stateMenuPlayNew,
};
*/

void setup()
{
  
  arduboy.boot();                                           // begin with the boot logo en setting up the device to work
  arduboy.audio.begin();
  arduboy.bootLogoSpritesSelfMasked();
  arduboy.setFrameRate(60);                                 // set the frame rate of the game at 60 fps
    EEPROM.begin();
  loadSetEEPROM();
}

void loop() {

  if (!(arduboy.nextFrame()))  return;
  if (gameState < STATE_GAME_NEXT_LEVEL && arduboy.everyXFrames(10))sparkleFrames = (++sparkleFrames) % 5;
  arduboy.pollButtons();
  arduboy.clear();
  
  //((FunctionPointer) pgm_read_word (&mainGameLoop[gameState]))();
  //(FunctionPointer) (&mainGameLoop[gameState]))();


  switch (gameState) {
case STATE_MENU_INTRO:
  stateMenuIntro(); break;
case STATE_MENU_MAIN:
  stateMenuMain(); break;
case STATE_MENU_HELP: 
  stateMenuHelp();break;
case STATE_MENU_PLAY: 
  stateMenuPlaySelect(); break;
case STATE_MENU_INFO:
  stateMenuInfo(); break;
case STATE_MENU_SOUNDFX:
  stateMenuSoundfx(); break;
case STATE_GAME_NEXT_LEVEL:
  stateGameNextLevel(); break;
case STATE_GAME_PLAYING:   
  stateGamePlaying(); break;
case STATE_GAME_PAUSE:  
  stateGamePause(); break;  
case STATE_GAME_OVER: 
  stateGameOver(); break;
case STATE_GAME_PLAYCONTNEW:
  stateMenuPlayContinue(); break;
  break;
default:
stateMenuPlayNew(); break;
  }
 
  arduboy.display();
}

