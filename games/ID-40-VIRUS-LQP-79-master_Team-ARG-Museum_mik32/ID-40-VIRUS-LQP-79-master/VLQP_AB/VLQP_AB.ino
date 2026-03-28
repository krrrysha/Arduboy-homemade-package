/*
  VIRUS LQP-79: http://www.team-arg.org/zmbt-manual.html

  Arduboy version 1.6.0:  http://www.team-arg.org/zmbt-downloads.html

  MADE by TEAM a.r.g. : http://www.team-arg.org/more-about.html

  2016 - FUOPY - JO3RI - STG - CASTPIXEL - JUSTIN CYR

  Game License: MIT : https://opensource.org/licenses/MIT

*/

//determine the game
#define GAME_ID                  40

#include "globals.h"
#include "menu.h"
#include "player.h"
#include "enemies.h"
#include "game.h"
#include "elements.h"
#include "bitmaps.h"
#include "level.h"


//typedef void (*FunctionPointer) ();
typedef void (*FunctionPointer) (void);


/*
const FunctionPointer PROGMEM mainGameLoop[] =
{
  stateMenuIntro,
  stateMenuMain,
  stateMenuHelp,
  stateMenuPlay,
  stateMenuInfo,
  stateMenuSoundfx,
  stateGamePrepareLevel,
  stateGameNextLevel,
  stateGamePlaying,
  stateGameOver,
  stateGamePause,
  stateGameEnd,
  stateGameNew,
  stateGameContinue,
  stateGameMayhem,
};
*/

void setup()
{
  arduboy.boot();                                           // begin with the boot logo en setting up the device to work
  arduboy.audio.begin();
  arduboy.bootLogoSpritesSelfMasked();
  arduboy.setFrameRate(60);
  gameID = GAME_ID;
  //Serial.begin(9600);
  EEPROM.begin();

}

void loop() {

  if (!(arduboy.nextFrame())) return;

  arduboy.pollButtons();

  arduboy.clear();

  //((FunctionPointer) pgm_read_word (&mainGameLoop[gameState]))();
  

  switch (gameState) {

    case STATE_MENU_INTRO:
      stateMenuIntro(); break;
    case STATE_MENU_MAIN:
      stateMenuMain(); break;
    case STATE_MENU_HELP:
      stateMenuHelp(); break;
    case STATE_MENU_PLAY:
      stateMenuPlay(); break;
    case STATE_MENU_INFO:
      stateMenuInfo(); break;
    case STATE_MENU_SOUNDFX:
      stateMenuSoundfx(); break;
    case STATE_GAME_PREPARE_LEVEL:
      stateGamePrepareLevel(); break;
    case STATE_GAME_NEXT_LEVEL:
      stateGameNextLevel(); break;
    case STATE_GAME_PLAYING:
      stateGamePlaying(); break;
    case STATE_GAME_OVER:
      stateGameOver(); break;
    case STATE_GAME_PAUSE:
      stateGamePause(); break;
    case STATE_GAME_END:
      stateGameEnd(); break;
    case STATE_GAME_NEW:
      stateGameNew(); break;
    case STATE_GAME_CONTINUE:
      stateGameContinue(); break;
    //case STATE_GAME_MAYHEM:
    default:
      stateGameMayhem(); break;
  }

  arduboy.display();

  //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
}

