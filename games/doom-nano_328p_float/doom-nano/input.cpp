#include <Arduino.h>
#include "input.h"
#include "constants.h"
#include <Arduboy2.h>
extern Arduboy2Base arduboy;

void input_setup() {
}

bool input_left() {
  return arduboy.pressed(LEFT_BUTTON);
};

bool input_right() {
  return arduboy.pressed(RIGHT_BUTTON);
};

bool input_up() {
  return arduboy.pressed(UP_BUTTON);
};

bool input_down() {
  return arduboy.pressed(DOWN_BUTTON);
};

bool input_fire() {
  return arduboy.pressed(A_BUTTON);
};

bool input_start() {
  return arduboy.pressed(B_BUTTON);
}

