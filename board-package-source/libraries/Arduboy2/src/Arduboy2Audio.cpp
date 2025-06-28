/**
 * @file Arduboy2Audio.cpp
 * \brief
 * The Arduboy2Audio class for speaker and sound control.
 */

#include "Arduboy2.h"

bool Arduboy2Audio::audio_enabled = false;

void Arduboy2Audio::on()
{
  // fire up audio pins by seting them as outputs
#ifndef ELBEARBOY
#ifdef ARDUBOY_10
  bitSet(SPEAKER_1_DDR, SPEAKER_1_BIT);
  bitSet(SPEAKER_2_DDR, SPEAKER_2_BIT);
#else
  bitSet(SPEAKER_1_DDR, SPEAKER_1_BIT);
#endif
#else
	PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * SPEAKER_1_BIT)); // установка вывода 3 порта 0 (в режим 0xb10). Timer Connect!
	//PAD_CONFIG->PORT_1_CFG |= (0b10 << (2 * SPEAKER_2_BIT)); // Нужно проверять, что там на порту!!!. установка вывода 1 порта 1 (в режим 0xb10). Timer Connect!
#endif	
  audio_enabled = true;
}

void Arduboy2Audio::off()
{
  audio_enabled = false;
  // shut off audio pins by setting them as inputs
#ifndef ELBEARBOY
#ifdef ARDUBOY_10
  bitClear(SPEAKER_1_DDR, SPEAKER_1_BIT);
  bitClear(SPEAKER_2_DDR, SPEAKER_2_BIT);
#else
  bitClear(SPEAKER_1_DDR, SPEAKER_1_BIT);
#endif
#else
	PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPEAKER_1_BIT)); // установка вывода 3 порта 0 (в режим 0xb00).  Timer Disconnect!
	//PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * SPEAKER_2_BIT)); // Нужно проверять, что там на порту!!!. установка вывода 1 порта 1 (в режим 0xb00).  Timer Disconnect!
#endif
}

void Arduboy2Audio::toggle()
{
  if (audio_enabled)
    off();
  else
    on();
}

void Arduboy2Audio::saveOnOff()
{
#ifndef ELBEARBOY
  EEPROM.update(Arduboy2Base::eepromAudioOnOff, audio_enabled);
#else
	Arduboy2Core::update_eeprom_1st_page_word(Arduboy2Base::eepromAudioOnOff, audio_enabled);
#endif
}

void Arduboy2Audio::begin()
{
#ifndef ELBEARBOY
  if (EEPROM.read(Arduboy2Base::eepromAudioOnOff))
#else
  if (Arduboy2Core::read_eeprom_byte(Arduboy2Base::eepromAudioOnOff))
#endif
    on();
  else
    off();
}

bool Arduboy2Audio::enabled()
{
  return audio_enabled;
}
