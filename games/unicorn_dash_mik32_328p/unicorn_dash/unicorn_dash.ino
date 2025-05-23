/*
    Unicorn Dash
    By Kirill Korolkov
    Version 0.9.5
    February, 2018

*/

#include <Arduboy2.h>
#include "Tinyfont.h"

#define GROUND_HEIGHT       46
#define OBSTACLE_DELAY_MAX  512
#define OBSTACLE_DELAY_MIN  96
#define BONUS_DELAY_MAX     1024
#define BONUS_DELAY_MIN     512
#define UINT_MAX ((unsigned int)~0)

enum State {
    Intro,
    Play,
    Pause,
    Over,
    Credits
};

struct Position {
    int x;
    int y;
};
struct Velocity {
    int x;
    int y;
};
struct Size {
    unsigned int width;
    unsigned int height;
};

struct Object {
    Position pos;
    Velocity vel;
    Rect     box;
    unsigned int frame;
    unsigned int type;
    boolean action;
};
struct Star {
    Position pos;
    unsigned int frame;
};
struct Particle {
    Position pos;
    unsigned int life;
    unsigned int lifeCount;
};

const byte spriteLogoA[] PROGMEM = {
    0x3e, 0x63, 0xc9, 0x9d, 0x39, 0x7b, 0xf2, 0xf6, 0xf4, 0xe4, 0xc8, 0xd8, 0x90, 0xb0, 0xa0, 0x20, 0x60, 0xc0, 0x80, 0x80, 0xc0, 0x60, 0x20, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0x20, 0x60, 0x40, 0x40, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1, 0x7, 0xc, 0x19, 0x73, 0xc7, 0x9f, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfd, 0xfd, 0xfc, 0xff, 0xff, 0xff, 0x9f, 0xf, 0xf, 0xf, 0xf, 0x1f, 0x1f, 0x1e, 0x3c, 0x3d, 0x7d, 0xf9, 0xfb, 0xf2, 0xe6, 0xcc, 0x98, 0x30, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0x1f, 0xc3, 0xf8, 0xff, 0xff, 0xf3, 0x5, 0x3, 0x45, 0x8b, 0x17, 0xaf, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xe0, 0xc0, 0x1, 0x3, 0xf, 0x1f, 0x7f, 0xff, 0xfe, 0xf8, 0xf3, 0xc6, 0x1c, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0x1f, 0xc0, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xfc, 0xfc, 0xfe, 0xff, 0x7f, 0x3f, 0x9f, 0xf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf0, 0x80, 0x00, 0x1, 0xf, 0xff, 0xff, 0xff, 0xfc, 0x1, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7, 0x70, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x8f, 0xe3, 0xf9, 0xfc, 0xfe, 0x3f, 0x87, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x80, 0xf0, 0xff, 0xff, 0xff, 0x1f, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0xc0, 0x41, 0x47, 0x4c, 0x49, 0x6b, 0x2b, 0xab, 0xab, 0xbb, 0x99, 0xdd, 0xcc, 0xee, 0xe7, 0xf7, 0xf3, 0xf9, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9f, 0xc7, 0xf0, 0xf8, 0xfe, 0x7f, 0x3f, 0x8f, 0xe7, 0x31, 0x1c, 0x7, 0x00, 0x00, 0x00, 0x00, 0x7, 0xc, 0x19, 0x13, 0x37, 0x27, 0x6f, 0xcf, 0x9f, 0xbf, 0xbf, 0x3f, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x7f, 0x3f, 0xbf, 0xbf, 0xbf, 0x9f, 0xcf, 0x67, 0x27, 0x33, 0x19, 0xc, 0x6, 0x3, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1, 0x1, 0x3, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0x1, 0x1, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const byte spriteLogoB[] PROGMEM = {
    0xf, 0x10, 0x10, 0x1f, 0x00, 0x1f, 0x1, 0x1, 0x1e, 0x00, 0x1d, 0x00, 0x1f, 0x11, 0x11, 0x11, 0x00, 0xdf, 0x51, 0x51, 0xdf, 0x00, 0xdf, 0x49, 0x49, 0xd6, 0x00, 0xdf, 0x41, 0x41, 0x5e, 0x00, 0xc0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7, 0x4, 0x4, 0x3, 0x00, 0x7, 0x2, 0x2, 0x7, 0x00, 0x5, 0x5, 0x5, 0x7, 0x00, 0x7, 0x1, 0x1, 0x7
};

const byte spriteBackgroundA[] PROGMEM  = {
    0xc0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0xc0, 0x00, 0xc0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
};
const byte spriteBackgroundB[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x4, 0x2, 0x2, 0x2, 0x4, 0x18, 0x20, 0x10, 0x10, 0x10, 0x20, 0x18, 0x4, 0x2, 0x2, 0x2, 0x4, 0x18, 0x20, 0x10, 0x10, 0x10, 0x20, 0x8a, 0x55, 0xa2, 0x54, 0x8a, 0x55, 0xa2, 0x54, 0x8a, 0x55, 0xa2, 0x54, 0x8a, 0x55, 0xa2, 0x54, 0x8a, 0x55, 0xa2, 0x54, 0x8a, 0x55, 0xa2, 0x54,
};

const byte spriteMoon[] PROGMEM = {
    0xe0, 0xf8, 0xfc, 0xfe, 0xe, 0x3, 0x1, 0x1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7, 0x1f, 0x3f, 0x7f, 0x7f, 0xfc, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0x70, 0x78, 0x38, 0x1c, 0x7,
};

const byte spriteStar_0[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x8, 0x00, 0x00, 0x00 };
const byte spriteStar_1[] PROGMEM  = { 0x00, 0x00, 0x00, 0x8, 0x1c, 0x8, 0x00, 0x00 };
const byte spriteStar_2[] PROGMEM  = { 0x00, 0x00, 0x00, 0x8, 0x14, 0x8, 0x00, 0x00 };
const byte spriteStar_3[] PROGMEM  = { 0x00, 0x00, 0x8, 0x8, 0x36, 0x8, 0x8, 0x00 };
const byte spriteStar_4[] PROGMEM  = { 0x00, 0x00, 0x8, 0x00, 0x2a, 0x00, 0x8, 0x00 };
const byte *animationFramesStar[] = { spriteStar_0, spriteStar_1, spriteStar_2, spriteStar_3, spriteStar_4 };

const byte spriteUnicorn_0[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18, 0x8, 0x7c, 0x1c, 0x1c, 0x1e, 0x7f, 0x2, 0x3, 0x3, 0x00, 0x00, 0x00,
};
const byte spriteUnicorn_1[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x20, 0x30, 0x10, 0x38, 0x78, 0x38, 0x38, 0x38, 0x7c, 0x3e, 0x5, 0x6, 0x6, 0x00, 0x00,
};
const byte spriteUnicorn_2[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0xc0, 0xe0, 0x50, 0x68, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2, 0x3, 0x1, 0xf, 0x3, 0x3, 0x3, 0xf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const byte spriteUnicornMask_0[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0xa0, 0xd0, 0x20, 0x00, 0x00, 0x00, 0x10, 0x28, 0x24, 0x74, 0x82, 0x62, 0x22, 0x61, 0x80, 0x7d, 0x4, 0x4, 0x3, 0x00, 0x00,
};
const byte spriteUnicornMask_1[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0xa0, 0x40, 0x00, 0x20, 0x50, 0x48, 0x28, 0x44, 0x84, 0x44, 0x44, 0x44, 0x82, 0x41, 0x3a, 0x9, 0x9, 0x6, 0x00,
};
const byte spriteUnicornMask_2[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x40, 0x20, 0x10, 0xa8, 0x94, 0x9a, 0x64, 0x00, 0x00, 0x00, 0x2, 0x5, 0x4, 0xe, 0x10, 0xc, 0x4, 0xc, 0x10, 0xf, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const byte *animationFramesUnicorn[] = { spriteUnicorn_0, spriteUnicorn_1, spriteUnicorn_2 };
const byte *animationFramesUnicornMask[] = { spriteUnicornMask_0, spriteUnicornMask_1, spriteUnicornMask_2 };

const byte spriteStar[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4, 0x6c, 0x7c, 0x3f, 0x37, 0x7c, 0x6c, 0x4, 0x00, 0x00, 0x00, 0x00,
};
const byte spriteStarMask[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4, 0x6a, 0x92, 0x83, 0x40, 0x48, 0x83, 0x92, 0x6a, 0x4, 0x00, 0x00, 0x00,
};
const byte spriteGhost[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x33, 0x3f, 0x3f, 0x33, 0x3f, 0x3e, 0x18, 0x10, 0x00, 0x00, 0x00,
};
const byte spriteGhostMask[] PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x21, 0x4c, 0x40, 0x40, 0x4c, 0x40, 0x41, 0x26, 0x28, 0x18, 0x00, 0x00,
};
const byte *spritesObject[] = { spriteGhost, spriteStar };
const byte *spritesObjectMask[] = { spriteGhostMask, spriteStarMask };

boolean btnPressed, dpadPressed;

unsigned int counterState, counterBackgroundA, counterBackgroundB = 0;
int scoreBonusDuration = 0;

long score, scoreHI = 0;

Arduboy2 arduboy;
Tinyfont arduboyTinyFont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

State state = Intro;

Object unicorn, objects[3];
Particle particles[24];
Star stars[9] = {
    { { 2,  13  }, 1 },
    { { 23,  3  }, 4 },
    { { 27,  24 }, 2 },
    { { 42,  7  }, 0 },
    { { 59,  16 }, 4 },
    { { 77,  8  }, 1 },
    { { 92,  21 }, 0 },
    { { 109, 9  }, 3 },
    { { 116, 17 }, 0 }
};

// ------------------------------------------------------------------------------------
void setup() {
    arduboy.begin();
    EEPROM.begin();
    particlesReset();
    statePlayReset();

    scoreHI = EEPROM.read(EEPROM_STORAGE_SPACE_START);
}
void loop() {
    arduboy.clear();

    if (!arduboy.nextFrame()) {
        return;
    }

    if (arduboy.everyXFrames(32)) {
        randomSeed(analogRead(0)); // reset random
    }

    stateUpdate();

    arduboy.display();
}

// ------------------------------------------------------------------------------------
void stateUpdate() {
    counterState = ((counterState + 1) > UINT_MAX) ? 0 : counterState;

    switch (state) {
        case State::Credits:
            stateCreditsUpdate();
            break;

        case State::Play:
            statePlayUpdate();
            break;

        case State::Pause:
            statePauseUpdate();
            break;

        case State::Over:
            stateOverUpdate();
            break;

        default:
            stateIntroUpdate();
            break;
    }
}
void stateSwitch(State stateNew) {
    counterState = 0;
    counterBackgroundA, counterBackgroundB = 0;

    switch (stateNew) {
        case State::Intro:
            statePlayReset();
            break;
    }

    state = stateNew;
};

// STATE INTRO: -----------------------------------------------------------------------
void stateIntroUpdate() {
    counterState += 1;

    arduboy.drawBitmap(39, 0, spriteLogoA, 50, 58, WHITE);
    arduboy.drawBitmap(4, 16, spriteLogoB, 36, 11, WHITE);

    arduboyTinyFont.setCursor(44, 60);
    for (byte i = (8 - intLength(scoreHI)); i > 0; i--) {
        arduboyTinyFont.print("0");
    }

    arduboyTinyFont.print(scoreHI);

    if ((arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON)) && !btnPressed) {
        if (arduboy.pressed(B_BUTTON)) {
            stateSwitch(State::Play);
        }
        else if (arduboy.pressed(A_BUTTON)) {
            stateSwitch(State::Credits);
        }
    }
    btnPressed = (arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON));

    // ZzZ: DEMO
    // if (counterState >= 300) {
    //     stateSwitch(State::Play);
    // }
}

// STATE CREDITS: ---------------------------------------------------------------------=
void stateCreditsUpdate() {
    if (arduboy.everyXFrames(3) && counterState <= 100) {
        // if (arduboy.everyXFrames(3) && counterState <= 300) {
        counterState += 1;
    }

    arduboy.setCursor(43, arduboy.height() - counterState);
    arduboy.print("CREDITS");
    arduboy.setCursor(37, arduboy.height() * 2 - counterState);
    arduboy.print("a game by");
    arduboy.setCursor(19,  arduboy.height() * 2 + 10 - counterState);
    arduboy.print("KIRILL KOROLKOV");

    // arduboy.setCursor(19, arduboy.height() * 2 - counterState);
    // arduboy.print("project manager");
    // arduboy.setCursor(4,  arduboy.height() * 2 + 10 - counterState);
    // arduboy.print("MARISKA VAN DER PEET");
    // arduboy.setCursor(37, arduboy.height() * 3 - counterState);
    // arduboy.print("team lead");
    // arduboy.setCursor(13, arduboy.height() * 3 + 10 - counterState);
    // arduboy.print("MARTIJN STELLINGA");
    // arduboy.setCursor(37, arduboy.height() * 4  - counterState);
    // arduboy.print("developer");
    // arduboy.setCursor(19, arduboy.height() * 4 + 10 - counterState);
    // arduboy.print("KIRILL KOROLKOV");
    // arduboy.setCursor(27, arduboy.height() * 5 - counterState);
    // arduboy.print("in memory of");
    // arduboy.setCursor(19, arduboy.height() * 5 + 10 - counterState);
    // arduboy.print("THOMAS DE BRUIN");
    // arduboy.setCursor(37, arduboy.height() * 5 + 20 - counterState);
    // arduboy.print("2014-2017");

    if (arduboy.pressed(B_BUTTON) && !btnPressed) {
        stateSwitch(State::Intro);
    }
    btnPressed = arduboy.pressed(B_BUTTON);
}

// STATE Over: ---------------------------------------------------------------------
void stateOverUpdate() {
    arduboy.setCursor(37, 18);
    arduboy.print("GAME OVER");

    for (byte i = 0; i < 3; i++) {
        if (objects[i].type == 0) {
            objectRender(objects[i]);
        }
    }

    unicornRender();
    scoreRender();

    if (arduboy.pressed(A_BUTTON) && !btnPressed) {
        if (score > scoreHI) {
            scoreHI = score;
            EEPROM.update(EEPROM_STORAGE_SPACE_START, scoreHI);
        }

        stateSwitch(State::Intro);
    }
    btnPressed = arduboy.pressed(A_BUTTON);
}

// STATE PLAY: ------------------------------------------------------------------------
void statePlayUpdate() {
    counterState += 1;

    if ((arduboy.pressed(LEFT_BUTTON) || arduboy.pressed(RIGHT_BUTTON))) {
        if (arduboy.pressed(LEFT_BUTTON) && unicorn.action) {
            unicorn.pos.x -= 1;
        }
        if (arduboy.pressed(RIGHT_BUTTON) && unicorn.action) {
            unicorn.pos.x += 1;
        }
    }

    if ((arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON)) && !btnPressed) {
        if (arduboy.pressed(A_BUTTON)) {
            stateSwitch(State::Pause);
        }
        if (arduboy.pressed(B_BUTTON) && unicorn.action) {
            unicorn.vel.y = -3;
            counterState = 0;
        }
    }
    btnPressed = (arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON));

    backgroundLayerOneRender();
    backgroundLayerTwoRender();
    backgroundLayerThreeRender();

    objectsUpdate();
    unicornUpdate();

    scoreUpdate();
}
void statePlayReset() {
    score = 0;
    scoreBonusDuration = 0;

    unicornSetup();
    objectsSetup();
}

// STATE PAUSE: -----------------------------------------------------------------------
void statePauseUpdate() {
    arduboy.setCursor(49, (arduboy.height() / 2) - 4);
    arduboy.print("PAUSE");

    scoreRender();

    if ((arduboy.pressed(A_BUTTON) || arduboy.pressed(DOWN_BUTTON)) && !btnPressed) {
        if (arduboy.pressed(DOWN_BUTTON)) {
            stateSwitch(State::Intro);
        }
        else if (arduboy.pressed(A_BUTTON)) {
            stateSwitch(State::Play);
        }
    }
    btnPressed = (arduboy.pressed(A_BUTTON) || arduboy.pressed(DOWN_BUTTON));
}

// UNICORN ----------------------------------------------------------------------------
void unicornSetup() {
    unicorn = { { 48, GROUND_HEIGHT }, { 0, 0 }, { 0, 0, 7, 8 }, 0, 7, true };
}
 void unicornUpdate() {
    // ZzZ: DEMO
    // for (byte i = 0; i < 3; i++) {
    //     int dx = objects[i].pos.x - unicorn.pos.x;
    //
    //     if (dx > 0 && dx <= 24) {
    //         if (unicorn.action) {
    //             unicorn.vel.y = -3;
    //             counterState = 0;
    //         }
    //     }
    // }

    if (counterState%10 == 0) {
        unicorn.vel.y += 1;
    }

    unicorn.pos.y = min(unicorn.pos.y + unicorn.vel.y, GROUND_HEIGHT);
    unicorn.pos.x = min(unicorn.pos.x + unicorn.vel.x, arduboy.width() - 12);
    unicorn.pos.x = max(unicorn.pos.x + unicorn.vel.x, -4);

    unicorn.box.x = unicorn.pos.x + 4;
    unicorn.box.y = unicorn.pos.y - 9;

    if (unicorn.pos.y >= GROUND_HEIGHT) {
        unicorn.action = true;
        unicorn.vel.y = 0;
    }
    else {
        unicorn.action = false;
    }

    unicornRender();
    particlesRender();
}
void unicornRender() {
    if (unicorn.action) {
        if (arduboy.everyXFrames(6)) {
            unicorn.frame = ((unicorn.frame + 1) > 2) ? 0 : unicorn.frame + 1;
        }
    }
    else {
        unicorn.frame = 0;
    }

    arduboy.drawBitmap(unicorn.pos.x, unicorn.pos.y - 15, animationFramesUnicornMask[unicorn.frame], 16, 16, BLACK);
    arduboy.drawBitmap(unicorn.pos.x, unicorn.pos.y - 15, animationFramesUnicorn[unicorn.frame], 16, 16, WHITE);
}

// OBJECTS ---------------------------------------------------------------------------
void objectsSetup() {
    objects[0] = { { -16, GROUND_HEIGHT }, { -1, 0 }, { 0, 0, 8, 8 }, 0, 0, true };
    objects[1] = { { -16, GROUND_HEIGHT }, { -1, 0 }, { 0, 0, 8, 8 }, 0, 0, true };
    objects[2] = { { -16, 22 },            { -1, 0 }, { 0, 0, 8, 8 }, 0, 1, true };

    for (byte i = 0; i < 3; i++) {
        objects[i] = objectReset(objects[i]);
    }
}
void objectsUpdate() {
    for (byte i = 0; i < 3; i++) {
        objects[i].pos.x += objects[i].vel.x;
        objects[i].box.x = objects[i].pos.x + 4;
        objects[i].box.y = objects[i].pos.y - 8;

        if (objects[i].pos.x >= -8) {
            Rect boxA = { unicorn.box.x, unicorn.box.y, unicorn.box.width, unicorn.box.height };
            Rect boxB = { objects[i].box.x, objects[i].box.y, objects[i].box.width, objects[i].box.height };

            if (arduboy.collide(boxA, boxB)) {
                if (objects[i].type == 0) {
                    state = State::Over;
                }
                else if (objects[i].type == 1){
                    objects[i] = objectReset(objects[i]);

                    if (scoreBonusDuration <= 0) {
                        particlesReset();
                    }
                    scoreBonusDuration += 50;
                }
            }

            objectRender(objects[i]);
        }
        else {
            objects[i] = objectReset(objects[i]);
        }
    }
}
void objectRender(Object object) {
    arduboy.drawBitmap(object.pos.x, object.pos.y - 15, spritesObjectMask[object.type], 16, 16, BLACK);
    arduboy.drawBitmap(object.pos.x, object.pos.y - 15, spritesObject[object.type], 16, 16, WHITE);
    // arduboy.drawRect(object.box.x, object.box.y, object.box.width, object.box.height, 1); // Collision Box Debug
}
Object objectReset(Object object){
    if (object.type == 0) {
        object.pos.x = arduboy.width() + random(OBSTACLE_DELAY_MIN, OBSTACLE_DELAY_MAX);

        for (byte i = 0; i < 3; i++) {
            if (objects[i].type == 0) {
                if (objects[i].pos.x >= arduboy.width()) {
                    if ((rand()%4) > 2) {
                        object.pos.x = objects[i].pos.x + 10;
                        object.pos.y = GROUND_HEIGHT;
                    }
                    else {
                        object.pos.x = objects[i].pos.x + random(OBSTACLE_DELAY_MIN, OBSTACLE_DELAY_MAX);
                        object.pos.y = GROUND_HEIGHT - random(0, 12);
                    }
                }
            }
        }
    }
    if (object.type == 1) {
        object.pos.x = arduboy.width() + random(BONUS_DELAY_MIN, BONUS_DELAY_MAX);
    }

    return object;
}

// BACKGROUND -------------------------------------------------------------------------
void backgroundLayerOneRender() {
    arduboy.drawBitmap(12, 4, spriteMoon, 16, 16, WHITE);

    for (byte i = 0; i < 9; i++) {
        if (arduboy.everyXFrames(8)) {
            stars[i].frame = ((stars[i].frame + 1) > 4) ? 0 : stars[i].frame + 1;
        }

        arduboy.drawBitmap(stars[i].pos.x, stars[i].pos.y, animationFramesStar[stars[i].frame], 8, 8, WHITE);
    }
}
void backgroundLayerTwoRender() {
    if (arduboy.everyXFrames(2)) {
        counterBackgroundA = ((counterBackgroundA + 1) >= 24) ? 0 : counterBackgroundA + 1;
    }

    for (byte i = 0; i <= 6; i++) {
        arduboy.drawBitmap(((i * 24) - counterBackgroundA), 28, spriteBackgroundA, 24, 36, WHITE);
    }
}
void backgroundLayerThreeRender() {
    if (arduboy.everyXFrames(1)) {
        counterBackgroundB = ((counterBackgroundB + 1) >= 24) ? 0 : counterBackgroundB + 1;
    }

    for (byte i = 0; i <= 6; i++) {
        arduboy.drawBitmap(((i * 24) - counterBackgroundB), 41, spriteBackgroundB, 24, 24, BLACK);
    }
}

// SCORE -----------------------------------------------------------------------------
void scoreUpdate() {
    if (arduboy.everyXFrames(16)) {
        scoreBonusDuration = max(0, (scoreBonusDuration - 1));

        if (scoreBonusDuration > 0){
            score += 10;
        }

        score = min((long) 99999999, (score + 1));
    }

    scoreRender();
}
void scoreRender() {
    if (score > scoreHI) {
        arduboyTinyFont.setCursor(77, 4);
        arduboyTinyFont.print("HI");
    }
    else {
        arduboyTinyFont.setCursor(87, 4);
    }

    for (byte i = (8 - intLength(score)); i > 0; i--) {
        arduboyTinyFont.print("0");
    }

    arduboyTinyFont.print(score);
}

// HELPERS ----------------------------------------------------------------------------
int intLength(int i) {
    int j = 0;

    for(; i; i /= 10) j++;

    return (j == 0) ? 1 : j;
}

void particlesRender() {
    if (scoreBonusDuration <= 0) {
        return;
    }

    for (byte i = 0; i < 24; i++) {
        particles[i].pos.x -= 1;

        if (particles[i].lifeCount > particles[i].life) {
            particles[i].life = 8 + rand()%32;
            particles[i].lifeCount = 0;

            particles[i].pos.y = unicorn.pos.y - 3 + ((rand()%2 > 0) ? (rand()%4 * -1) : rand()%4);
            particles[i].pos.x = unicorn.pos.x + 4;
        }
        else {
            particles[i].lifeCount += 1;
        }

        arduboy.drawPixel(particles[i].pos.x, particles[i].pos.y - 1, 0);
        arduboy.drawPixel(particles[i].pos.x, particles[i].pos.y, 1);
    }
}
void particlesReset() {
    for (int i = 0; i < 24; ++i) {
        particles[i] = { { 0, 0 }, 0 };
    }
};
