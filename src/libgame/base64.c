// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2021 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// base64.c
// ============================================================================

/*

  https://github.com/superwills/NibbleAndAHalf
  base64.h -- Fast base64 encoding and decoding.
  version 1.0.0, April 17, 2013 143a

  Copyright (C) 2013 William Sherif

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  William Sherif
  will.sherif@gmail.com

  YWxsIHlvdXIgYmFzZSBhcmUgYmVsb25nIHRvIHVz

*/

// ----------------------------------------------------------------------------
// Base64 encoder/decoder code was altered for integration in Rocks'n'Diamonds
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "base64.h"


static const char *b64encode =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encoded_size(int unencoded_size)
{
  int mod = unencoded_size % 3;
  int pad = (mod > 0 ? 3 - mod : 0);

  return 4 * (unencoded_size + pad) / 3 + 1;
}

void base64_encode(char *encoded_data,
		   const void *unencoded_ptr, int unencoded_size)
{
  const byte *unencoded_data = (const byte *)unencoded_ptr;
  char *ptr = encoded_data;
  int i;

  int mod = unencoded_size % 3;
  int pad = (mod > 0 ? 3 - mod : 0);

  for (i = 0; i <= unencoded_size - 3; i += 3)
  {
    byte byte0 = unencoded_data[i];
    byte byte1 = unencoded_data[i + 1];
    byte byte2 = unencoded_data[i + 2];

    *ptr++ = b64encode[byte0 >> 2];
    *ptr++ = b64encode[((byte0 & 0x03) << 4) + (byte1 >> 4)];
    *ptr++ = b64encode[((byte1 & 0x0f) << 2) + (byte2 >> 6)];
    *ptr++ = b64encode[byte2 & 0x3f];
  }

  if (pad == 1)
  {
    byte byte0 = unencoded_data[i];
    byte byte1 = unencoded_data[i + 1];

    *ptr++ = b64encode[byte0 >> 2];
    *ptr++ = b64encode[((byte0 & 0x03) << 4) + (byte1 >> 4)];
    *ptr++ = b64encode[((byte1 & 0x0f) << 2)];
    *ptr++ = '=';
  }
  else if (pad == 2)
  {
    byte byte0 = unencoded_data[i];

    *ptr++ = b64encode[byte0 >> 2];
    *ptr++ = b64encode[(byte0 & 0x03) << 4];
    *ptr++ = '=';
    *ptr++ = '=';
  }

  *ptr++= '\0';
}

static const byte b64decode[] =
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	//   0
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	//  16
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,	//  32
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,	//  48

   0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,	//  64
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,	//  80
   0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	//  96
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,	// 112

   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 128
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 144
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 160
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 176

   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 192
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 208
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 224
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 240
};

int base64_decoded_size(const char *encoded_data)
{
  int encoded_size = strlen(encoded_data);

  if (encoded_size < 2)
    return 0;

  int pad = 0;

  if (encoded_data[encoded_size - 1] == '=')
    pad++;
  if (encoded_data[encoded_size - 2] == '=')
    pad++;

  return 3 * encoded_size / 4 - pad;
}

void base64_decode(byte *decoded_data, const char *encoded_ptr)
{
  const byte *encoded_data = (const byte *)encoded_ptr;
  byte *ptr = decoded_data;
  int encoded_size = strlen(encoded_ptr);
  int i;

  if (encoded_size < 2)
    return;

  int pad = 0;

  if (encoded_data[encoded_size - 1] == '=')
    pad++;
  if (encoded_data[encoded_size - 2] == '=')
    pad++;

  for (i = 0; i <= encoded_size - 4 - pad; i += 4)
  {
    byte byte0 = b64decode[encoded_data[i]];
    byte byte1 = b64decode[encoded_data[i + 1]];
    byte byte2 = b64decode[encoded_data[i + 2]];
    byte byte3 = b64decode[encoded_data[i + 3]];

    *ptr++ = (byte0 << 2) | (byte1 >> 4);
    *ptr++ = (byte1 << 4) | (byte2 >> 2);
    *ptr++ = (byte2 << 6) | (byte3);
  }

  if (pad == 1)
  {
    byte byte0 = b64decode[encoded_data[i]];
    byte byte1 = b64decode[encoded_data[i + 1]];
    byte byte2 = b64decode[encoded_data[i + 2]];

    *ptr++ = (byte0 << 2) | (byte1 >> 4);
    *ptr++ = (byte1 << 4) | (byte2 >> 2);
  }
  else if (pad == 2)
  {
    byte byte0 = b64decode[encoded_data[i]];
    byte byte1 = b64decode[encoded_data[i + 1]];

    *ptr++ = (byte0 << 2) | (byte1 >> 4);
  }
}
