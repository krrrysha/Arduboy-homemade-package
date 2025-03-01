#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

using uint24_t = __uint24;

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xff65;
constexpr uint24_t FX_DATA_BYTES = 39470;

constexpr uint24_t mapGfx = 0x000000;
constexpr uint16_t mapGfxWidth  = 816;
constexpr uint16_t mapGfxHeight = 368;

constexpr uint24_t whaleGfx = 0x0092A4;
constexpr uint16_t whaleGfxWidth  = 107;
constexpr uint16_t whaleGfxHeight = 69;

