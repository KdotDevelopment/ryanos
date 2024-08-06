#include "math.hpp"

/*
 * This file was written in 2023 by Martin Dvorak <jezek2@advel.cz>
 * You can download latest version at http://public-domain.advel.cz/
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this file to the
 * public domain worldwide. This software is distributed without any
 * warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see:
 * http://creativecommons.org/publicdomain/zero/1.0/ 
 */

#include <stdint.h>
#include "../lib/standard.hpp"

// Math code available at http://public-domain.advel.cz/ under CC0 license


static void mul128(uint64_t result_out[4], uint64_t a[2], uint64_t b[2])
{
   uint32_t src1[4];
   uint32_t src2[4];
   uint32_t result[8];
   uint64_t mul[16];
   uint64_t tmp1, tmp2;

   src1[0] = a[0];
   src1[1] = a[0] >> 32;
   src1[2] = a[1];
   src1[3] = a[1] >> 32;
   src2[0] = b[0];
   src2[1] = b[0] >> 32;
   src2[2] = b[1];
   src2[3] = b[1] >> 32;

   //                                     a0*b0
   //                               a0*b1
   //                         a0*b2 a1*b0
   //                   a0*b3 a1*b1
   //                   a1*b2 a2*b0
   //             a1*b3 a2*b1
   //             a2*b2 a3*b0
   //       a2*b3 a3*b1
   //       a3*b2
   // a3*b3

   mul[ 0] = (uint64_t)src1[0] * (uint64_t)src2[0];
   mul[ 1] = (uint64_t)src1[0] * (uint64_t)src2[1];
   mul[ 2] = (uint64_t)src1[0] * (uint64_t)src2[2];
   mul[ 3] = (uint64_t)src1[0] * (uint64_t)src2[3];
   mul[ 4] = (uint64_t)src1[1] * (uint64_t)src2[0];
   mul[ 5] = (uint64_t)src1[1] * (uint64_t)src2[1];
   mul[ 6] = (uint64_t)src1[1] * (uint64_t)src2[2];
   mul[ 7] = (uint64_t)src1[1] * (uint64_t)src2[3];
   mul[ 8] = (uint64_t)src1[2] * (uint64_t)src2[0];
   mul[ 9] = (uint64_t)src1[2] * (uint64_t)src2[1];
   mul[10] = (uint64_t)src1[2] * (uint64_t)src2[2];
   mul[11] = (uint64_t)src1[2] * (uint64_t)src2[3];
   mul[12] = (uint64_t)src1[3] * (uint64_t)src2[0];
   mul[13] = (uint64_t)src1[3] * (uint64_t)src2[1];
   mul[14] = (uint64_t)src1[3] * (uint64_t)src2[2];
   mul[15] = (uint64_t)src1[3] * (uint64_t)src2[3];

   result[0] = mul[0];
   tmp1 = (mul[0] >> 32) + (mul[1] & 0xFFFFFFFF) + (mul[4] & 0xFFFFFFFF);
   result[1] = tmp1;
   tmp2 = (tmp1 >> 32) + (mul[1] >> 32) + (mul[4] >> 32) + (mul[2] & 0xFFFFFFFF) + (mul[5] & 0xFFFFFFFF) + (mul[8] & 0xFFFFFFFF);
   result[2] = tmp2;
   tmp1 = (tmp2 >> 32) + (mul[2] >> 32) + (mul[5] >> 32) + (mul[8] >> 32) + (mul[3] & 0xFFFFFFFF) + (mul[6] & 0xFFFFFFFF) + (mul[9] & 0xFFFFFFFF) + (mul[12] & 0xFFFFFFFF);
   result[3] = tmp1;
   tmp2 = (tmp1 >> 32) + (mul[3] >> 32) + (mul[6] >> 32) + (mul[9] >> 32) + (mul[12] >> 32) + (mul[7] & 0xFFFFFFFF) + (mul[10] & 0xFFFFFFFF) + (mul[13] & 0xFFFFFFFF);
   result[4] = tmp2;
   tmp1 = (tmp2 >> 32) + (mul[7] >> 32) + (mul[10] >> 32) + (mul[13] >> 32) + (mul[11] & 0xFFFFFFFF) + (mul[14] & 0xFFFFFFFF);
   result[5] = tmp1;
   tmp2 = (tmp1 >> 32) + (mul[11] >> 32) + (mul[14] >> 32) + (mul[15] & 0xFFFFFFFF);
   result[6] = tmp2;
   tmp1 = (tmp2 >> 32) + (mul[15] >> 32);
   result[7] = tmp1;

   result_out[0] = ((uint64_t)result[0]) | (((uint64_t)result[1]) << 32);
   result_out[1] = ((uint64_t)result[2]) | (((uint64_t)result[3]) << 32);
   result_out[2] = ((uint64_t)result[4]) | (((uint64_t)result[5]) << 32);
   result_out[3] = ((uint64_t)result[6]) | (((uint64_t)result[7]) << 32);
}


static void shl128(uint64_t value[2], int amount)
{
   if (amount < 64) {
      value[1] = (value[1] << amount) | (value[0] >> (64 - amount));
      value[0] <<= amount;
   }
   else {
      value[1] = value[0] << (amount - 64);
      value[0] = 0;
   }
}


static void shr128(uint64_t value[2])
{
   value[0] = (value[1] << 63) | (value[0] >> 1);
   value[1] >>= 1;
}


static void inc128(uint64_t value[2])
{
   if (value[0] == 0xFFFFFFFFFFFFFFFFULL) {
      value[0] = 0;
      value[1]++;
   }
   else {
      value[0]++;
   }
}


static void float128_mul(uint64_t result[2], uint64_t a[2], uint64_t b[2])
{
   int e1, e2, e;
   uint64_t ma[2], mb[2], mul[4];

   ma[0] = a[0];
   ma[1] = (a[1] & ((1ULL << 48)-1)) | (1ULL << 48);
   mb[0] = b[0];
   mb[1] = (b[1] & ((1ULL << 48)-1)) | (1ULL << 48);

   e1 = ((a[1] >> 48) & 0x7FFF) - 16383;
   e2 = ((b[1] >> 48) & 0x7FFF) - 16383;

   e = e1 + e2;
   shl128(ma, 9);
   shl128(mb, 8);
   mul128(mul, ma, mb);
   
   if (mul[3] & (1ULL << 50)) {
      e++;
      mul[0] |= mul[2] & 1;
      shr128(&mul[2]);
   }

   if (mul[2] & 1) {
      if (mul[0] | mul[1]) {
         inc128(&mul[2]);
      }
      else if (mul[2] & 2) {
         inc128(&mul[2]);
      }
      if (mul[3] & (1ULL << 50)) {
         e++;
         shr128(&mul[2]);
      }
   }

   shr128(&mul[2]);
   result[0] = mul[2];
   result[1] = ((uint64_t)(e+16383) << 48) | (mul[3] & ((1ULL << 48)-1));
}


static int clz64(uint64_t value)
{
   #define DUP2(x) x, x
   #define DUP4(x) DUP2(x), DUP2(x)
   #define DUP8(x) DUP4(x), DUP4(x)
   #define DUP16(x) DUP8(x), DUP8(x)
   #define DUP32(x) DUP16(x), DUP16(x)
   #define DUP64(x) DUP32(x), DUP32(x)
   #define DUP128(x) DUP64(x), DUP64(x)
   static uint8_t table[256] = {
      8, 7, DUP2(6), DUP4(5), DUP8(4), DUP16(3), DUP32(2), DUP64(1), DUP128(0)
   };
   #undef DUP2
   #undef DUP4
   #undef DUP8
   #undef DUP16
   #undef DUP32
   #undef DUP64
   #undef DUP128

   if (value & 0xFFFFFFFF00000000ULL) {
      if (value & 0xFFFF000000000000ULL) {
         if (value & 0xFF00000000000000ULL) {
            return table[value >> 56];
         }
         else {
            return table[value >> 48] + 8;
         }
      }
      else {
         if (value & 0x0000FF0000000000ULL) {
            return table[value >> 40] + 16;
         }
         else {
            return table[value >> 32] + 24;
         }
      }
   }
   else {
      if (value & 0x00000000FFFF0000ULL) {
         if (value & 0x00000000FF000000ULL) {
            return table[value >> 24] + 32;
         }
         else {
            return table[value >> 16] + 40;
         }
      }
      else {
         if (value & 0x000000000000FF00ULL) {
            return table[value >> 8] + 48;
         }
         else {
            return table[value] + 56;
         }
      }
   }
}


static void float128_from_long(uint64_t f[2], uint64_t value)
{
   int lz;

   if (value == 0) {
      f[0] = 0;
      f[1] = 0;
      return;
   }

   f[0] = value;
   f[1] = 0;
   lz = clz64(value);
   shl128(f, 112 - 64 + lz + 1);
   f[1] = (f[1] & ((1ULL << 48)-1)) | (((uint64_t)(64 - lz - 1 + 16383)) << 48);
}


static void float128_from_double(uint64_t f[2], double value)
{
   union {
      double f;
      uint64_t i;
   } u;
   int e;

   u.f = value;
   e = (int)((u.i >> 52) & 0x7FF) - 1023;
   if (e == -1023) {
      u.i <<= 1;
      while ((u.i & (1ULL << 52)) == 0) {
         e--;
         u.i <<= 1;
      }
   }
   f[0] = u.i & ((1ULL<<52)-1);
   f[1] = 0;
   shl128(f, 60);
   f[1] |= ((uint64_t)(e+16383) << 48);
}


static double float128_to_double(uint64_t f[2])
{
   union {
      double f;
      uint64_t i;
   } u;
   int e;
   uint64_t tmp[2];

   if ((f[0] | f[1]) == 0) {
      return 0.0;
   }

   e = ((f[1] >> 48) & 0x7FFF) - 16383;

   tmp[0] = f[0];
   tmp[1] = (f[1] & ((1ULL << 48)-1)) | (1ULL << 48);
   shl128(tmp, 5);

   if (tmp[1] & 1) {
      if (tmp[0]) {
         tmp[1]++;
      }
      else if (tmp[1] & 2) {
         tmp[1]++;
      }
      if (tmp[1] & (1ULL << 54)) {
         e++;
         tmp[1] >>= 1;
      }
   }

   if (e <= -1023) {
      e--;
      tmp[1] = (tmp[1] | (1ULL<<53)) >> (-(1023 + e));
      e = -1023;
   }

   u.i = ((uint64_t)(e+1023) << 52) | ((tmp[1] >> 1) & ((1ULL << 52)-1));
   return u.f;
}


// buffer must be at least 20 bytes long
static char *float_to_string(char *buf, float input_value)
{
   union {
      double f;
      uint64_t i;
   } u;
   double value;
   int s, e, de, pos, num_digits, dot_pos;
   char *end;
   uint64_t digits, m;

   u.f = input_value;
   s = u.i >> 63;
   e = (int)((u.i >> 52) & 0x7FF) - 1023;
   u.i &= (1ULL << 63)-1;
   value = u.f;

   if (e == -1023) {
      strcopy(buf, s? "-0" : "0");
      return buf;
   }

   if (e == 1024) {
      m = u.i & ((1ULL<<52)-1);
      if (m) {
         strcopy(buf, "nan");
      }
      else {
         strcopy(buf, s? "-inf" : "inf");
      }
      return buf;
   }

   if (u.i < 0x41E0000000000000ULL && value == (int64_t)value && (int64_t)value < 1000000000) {
      digits = (int64_t)value;
      buf += 12;
      *(--buf) = 0;
      while (digits) {
         *(--buf) = '0' + (digits % 10);
         digits /= 10;
      }
      if (s) {
         *(--buf) = '-';
      }
      return buf;
   }

   u.i = float_log10_2;
   de = floor(e * u.f);
   if (de > 38) {
      de = 38;
   }
   if (de < -45) {
      de = -45;
   }

   u.i = float_powers_of_ten[53-de];
   value *= u.f;

   digits = value;
   num_digits = 1;
   if (digits >= 10) {
      de++;
      num_digits++;
   }
   if (digits >= 100) {
      de++;
      num_digits++;
   }

   u.f = value;
   e = (int)((u.i >> 52) & 0x7FF) - 1023;
   m = u.i & ((1ULL<<52)-1);

   pos = 52 - e;

   while (m) {
      m &= (1ULL<<pos)-1;
      m *= 10;
      if (num_digits >= 9) {
         if ((m >> pos) >= 5) {
            digits++;
         }
         break;
      }
      digits = digits*10 + (m >> pos);
      num_digits++;
   }

   dot_pos = num_digits-de-1;
   if (dot_pos == 0) dot_pos--;
   if (de >= num_digits || de <= -5) {
      dot_pos = num_digits-1;
   }

   end = buf + 15;
   buf = end; 
   *buf = 0;
   while (digits) {
      if (dot_pos-- == 0) {
         *(--buf) = '.';
      }
      *(--buf) = '0' + (digits % 10);
      digits /= 10;
   }
   if (dot_pos >= 0) {
      while (--dot_pos >= 0) {
         *(--buf) = '0';
      }
      *(--buf) = '.';
      *(--buf) = '0';
   }
   if (s) {
      *(--buf) = '-';
   }

   while (*(end-1) == '0') {
      *(--end) = 0;
   }

   if (de >= num_digits) {
      *end++ = 'e';
      *end++ = '+';
      *end++ = '0' + (de / 10);
      *end++ = '0' + (de % 10);
      *end++ = 0;
   }
   else if (de <= -5) {
      de = -de;
      *end++ = 'e';
      *end++ = '-';
      *end++ = '0' + (de / 10);
      *end++ = '0' + (de % 10);
      *end++ = 0;
   }

   return buf;
}


// buffer must be at least 29 bytes long
static char *double_to_string(char *buf, double input_value)
{
   union {
      double f;
      uint64_t i;
   } u;
   uint64_t value[2], power[2];
   int s, e, de, pos, num_digits, dot_pos;
   char *end;
   uint64_t digits, m;

   u.f = input_value;
   s = u.i >> 63;
   e = (int)((u.i >> 52) & 0x7FF) - 1023;
   u.i &= (1ULL << 63)-1;
   input_value = u.f;

   if (u.i == 0) {
      strcopy(buf, s? "-0" : "0");
      return buf;
   }

   if (e == 1024) {
      m = u.i & ((1ULL<<52)-1);
      if (m) {
         strcopy(buf, "nan");
      }
      else {
         strcopy(buf, s? "-inf" : "inf");
      }
      return buf;
   }

   if (u.i < 0x43E0000000000000ULL && input_value == (int64_t)input_value && (int64_t)input_value < 100000000000000000LL) {
      digits = (int64_t)input_value;
      buf += 19;
      *(--buf) = 0;
      while (digits) {
         *(--buf) = '0' + (digits % 10);
         digits /= 10;
      }
      if (s) {
         *(--buf) = '-';
      }
      return buf;
   }
   
   float128_from_double(value, input_value);
   e = (int)((value[1] >> 48) & 0x7FFF) - 16383;

   u.i = float_log10_2;
   de = floor(e * u.f);
   if (de > 308) {
      de = 308;
   }
   if (de < -324) {
      de = -324;
   }

   power[0] = double_powers_of_ten[(340-de)*2+0];
   power[1] = double_powers_of_ten[(340-de)*2+1];
   float128_mul(value, value, power);

   e = (int)((value[1] >> 48) & 0x7FFF) - 16383;

   digits = ((value[1] & ((1ULL << 48)-1)) | (1ULL << 48)) >> (48 - e);

   num_digits = 1;
   if (digits >= 10) {
      de++;
      num_digits++;
   }
   if (digits >= 100) {
      de++;
      num_digits++;
   }

   shl128(value, 12);
   m = value[1];

   pos = 60 - e;

   while (m) {
      m &= (1ULL<<pos)-1;
      m *= 10;
      if (num_digits >= 17) {
         if ((m >> pos) >= 5) {
            digits++;
         }
         break;
      }
      digits = digits*10 + (m >> pos);
      num_digits++;
   }

   dot_pos = num_digits-de-1;
   if (dot_pos == 0) dot_pos--;
   if (de >= num_digits || de <= -5) {
      dot_pos = num_digits-1;
   }

   end = buf + 23;
   buf = end; 
   *buf = 0;
   while (digits) {
      if (dot_pos-- == 0) {
         *(--buf) = '.';
      }
      *(--buf) = '0' + (digits % 10);
      digits /= 10;
   }
   if (dot_pos >= 0) {
      while (--dot_pos >= 0) {
         *(--buf) = '0';
      }
      *(--buf) = '.';
      *(--buf) = '0';
   }
   if (s) {
      *(--buf) = '-';
   }

   while (*(end-1) == '0') {
      *(--end) = 0;
   }

   if (de >= num_digits) {
      *end++ = 'e';
      *end++ = '+';
      if (de / 100) {
         *end++ = '0' + (de / 100);
      }
      *end++ = '0' + ((de / 10) % 10);
      *end++ = '0' + (de % 10);
      *end++ = 0;
   }
   else if (de <= -5) {
      de = -de;
      *end++ = 'e';
      *end++ = '-';
      if (de / 100) {
         *end++ = '0' + (de / 100);
      }
      *end++ = '0' + ((de / 10) % 10);
      *end++ = '0' + (de % 10);
      *end++ = 0;
   }

   return buf;
}


static float string_to_float(char *s)
{
   union {
      float f;
      uint32_t i;
   } u;
   union {
      double f;
      uint64_t i;
   } u2;
   uint64_t digits = 0;
   double value;
   int num_digits = 0;
   int sign=0, e=0, e2, esign;
   int round=0, rest=0;

   if (*s == '+') {
      s++;
   }
   else if (*s == '-') {
      sign = 1;
      s++;
   }

   if (*s != '.') {
      while (*s == '0') {
         s++;
      }
      if (*s != '.') {
         for (;;) {
            if (*s >= '0' && *s <= '9') {
               if (num_digits < 9) {
                  digits = digits*10 + (*s - '0');
                  num_digits++;
               }
               else {
                  if (num_digits == 9) {
                     if (*s == '5') {
                        round = 1;
                     }
                     else if (*s > '5') {
                        round = 2;
                     }
                     num_digits++;
                  }
                  else {
                     if (*s != '0') {
                        rest = 1;
                     }
                  }
                  e++;
               }
               s++;
            }
            else break;
         }
      }
   }
   if (*s == '.') {
      s++;
      if (digits == 0) {
         while (*s == '0') {
            s++;
            e--;
         }
      }
      for (;;) {
         if (*s >= '0' && *s <= '9') {
            if (num_digits < 9) {
               digits = digits*10 + (*s - '0');
               e--;
               num_digits++;
            }
            else {
               if (num_digits == 9) {
                  if (*s == '5') {
                     round = 1;
                  }
                  else if (*s > '5') {
                     round = 2;
                  }
                  num_digits++;
               }
               else {
                  if (*s != '0') {
                     rest = 1;
                  }
               }
            }
            s++;
         }
         else break;
      }
   }
   if (round) {
      if (round == 1 && rest == 0) {
         if (digits & 1) {
            digits++;
         }
      }
      else if (round != 0) {
         digits++;
      }
      if (digits >= 1000000000) {
         digits /= 10;
         e++;
      }
   }
   if (*s == 'e' || *s == 'E') {
      s++;
      e2 = 0;
      esign = 0;
      if (*s == '+') {
         s++;
      }
      else if (*s == '-') {
         esign = 1;
         s++;
      }
      while (*s == '0') {
         s++;
      }
      for (;;) {
         if (*s >= '0' && *s <= '9') {
            e2 = e2*10 + (*s - '0');
            if (!esign && e+e2 > 38) {
               u.i = (0xFF << 23) | (sign << 31); // +/-inf
               return u.f;
            }
            else if (esign && e-e2 < -53) {
               u.i = sign << 31; // +/-0
               return u.f;
            }
            s++;
         }
         else break;
      }
      if (esign) {
         e -= e2;
      }
      else {
         e += e2;
      }
   }
   if (*s != 0) {
      u.i = (0xFF << 23) | (1 << 22); // NaN
      return u.f;
   }
   if (e > 38) {
      u.i = (0xFF << 23) | (sign << 31); // +/-inf
      return u.f;
   }
   if (e < -53) {
      u.i = sign << 31; // +/-0
      return u.f;
   }

   u2.i = float_powers_of_ten[53+e];
   value = (double)digits * u2.f;

   u.f = value;
   u.i |= sign << 31;
   return u.f;
}


static double string_to_double(const char *s)
{
   union {
      double f;
      uint64_t i;
   } u;
   uint64_t digits = 0;
   uint64_t value[2], power[2];
   int num_digits = 0;
   int sign=0, e=0, e2, esign;
   int round=0, rest=0;

   if (*s == '+') {
      s++;
   }
   else if (*s == '-') {
      sign = 1;
      s++;
   }

   if (*s != '.') {
      while (*s == '0') {
         s++;
      }
      if (*s != '.') {
         for (;;) {
            if (*s >= '0' && *s <= '9') {
               if (num_digits < 17) {
                  digits = digits*10 + (*s - '0');
                  num_digits++;
               }
               else {
                  if (num_digits == 17) {
                     if (*s == '5') {
                        round = 1;
                     }
                     else if (*s > '5') {
                        round = 2;
                     }
                     num_digits++;
                  }
                  else {
                     if (*s != '0') {
                        rest = 1;
                     }
                  }
                  e++;
               }
               s++;
            }
            else break;
         }
      }
   }
   if (*s == '.') {
      s++;
      if (digits == 0) {
         while (*s == '0') {
            s++;
            e--;
         }
      }
      for (;;) {
         if (*s >= '0' && *s <= '9') {
            if (num_digits < 17) {
               digits = digits*10 + (*s - '0');
               e--;
               num_digits++;
            }
            else {
               if (num_digits == 17) {
                  if (*s == '5') {
                     round = 1;
                  }
                  else if (*s > '5') {
                     round = 2;
                  }
                  num_digits++;
               }
               else {
                  if (*s != '0') {
                     rest = 1;
                  }
               }
            }
            s++;
         }
         else break;
      }
   }
   if (round) {
      if (round == 1 && rest == 0) {
         if (digits & 1) {
            digits++;
         }
      }
      else if (round != 0) {
         digits++;
      }
      if (digits >= 100000000000000000ULL) {
         digits /= 10;
         e++;
      }
   }
   if (*s == 'e' || *s == 'E') {
      s++;
      e2 = 0;
      esign = 0;
      if (*s == '+') {
         s++;
      }
      else if (*s == '-') {
         esign = 1;
         s++;
      }
      while (*s == '0') {
         s++;
      }
      for (;;) {
         if (*s >= '0' && *s <= '9') {
            e2 = e2*10 + (*s - '0');
            if (!esign && e+e2 > 308) {
               u.i = (0x7FFULL << 52) | (((uint64_t)sign) << 63); // +/-inf
               return u.f;
            }
            else if (esign && e-e2 < -340) {
               u.i = ((uint64_t)sign) << 31; // +/-0
               return u.f;
            }
            s++;
         }
         else break;
      }
      if (esign) {
         e -= e2;
      }
      else {
         e += e2;
      }
   }
   if (*s != 0) {
      u.i = (0x7FFULL << 52) | (1ULL << 51); // NaN
      return u.f;
   }
   if (e > 308) {
      u.i = (0x7FFULL << 52) | (((uint64_t)sign) << 63); // +/-inf
      return u.f;
   }
   if (e < -340) {
      u.i = ((uint64_t)sign) << 63; // +/-0
      return u.f;
   }
   if (digits == 0) {
      e = 0;
   }

   float128_from_long(value, digits);
   power[0] = double_powers_of_ten[(340+e)*2+0];
   power[1] = double_powers_of_ten[(340+e)*2+1];
   float128_mul(value, value, power);

   u.f = float128_to_double(value);
   u.i |= ((uint64_t)sign) << 63;
   return u.f;
}


static float ftrunc(float value)
{
   union {
      float f;
      uint32_t i;
   } u;
   int e;

   u.f = value;
   e = ((u.i >> 23) & 0xFF) - 127;

   if (e < 0) {
      u.i &= 0x80000000;
   }
   else if (e < 23) {
      u.i &= ~((1 << (23-e))-1);
   }

   return u.f;
}


static float ftrunc_up(float value)
{
   union {
      float f;
      uint32_t i;
   } u;
   uint32_t m;
   int e;

   u.f = value;
   e = ((u.i >> 23) & 0xFF) - 127;

   if (e < 0) {
      if (u.i & 0x7FFFFFFF) {
         return u.i & 0x80000000? -1.0f : 1.0f;
      }
      return u.i & 0x80000000? -0.0f : 0.0f;
   }
   else if (e < 23) {
      if (u.i & ((1 << (23-e))-1)) {
         m = (u.i & ((1 << 23)-1)) | (1 << 23);
         m &= ~((1 << (23-e))-1);
         m += 1 << (23-e);
         if (m & (1 << 24)) {
            m >>= 1;
            e++;
         }
         u.i = (u.i & 0x80000000) | ((e+127) << 23) | (m & ((1 << 23)-1));
      }
   }

   return u.f;
}


static double dtrunc(double value)
{
   union {
      double f;
      uint64_t i;
   } u;
   int e;

   u.f = value;
   e = ((u.i >> 52) & 0x7FF) - 1023;

   if (e < 0) {
      u.i &= 0x8000000000000000ULL;
   }
   else if (e < 52) {
      u.i &= ~((1ULL << (52-e))-1);
   }

   return u.f;
}


static double dtrunc_up(double value)
{
   union {
      double f;
      uint64_t i;
   } u;
   uint64_t m;
   int e;

   u.f = value;
   e = ((u.i >> 52) & 0x7FF) - 1023;

   if (e < 0) {
      if (u.i & 0x7FFFFFFFFFFFFFFFULL) {
         return u.i & 0x8000000000000000ULL? -1.0 : 1.0;
      }
      return u.i & 0x8000000000000000ULL? -0.0 : 0.0;
   }
   else if (e < 52) {
      if (u.i & ((1ULL << (52-e))-1)) {
         m = (u.i & ((1ULL << 52)-1)) | (1ULL << 52);
         m &= ~((1ULL << (52-e))-1);
         m += 1ULL << (52-e);
         if (m & (1ULL << 53)) {
            m >>= 1;
            e++;
         }
         u.i = (u.i & 0x8000000000000000ULL) | (((uint64_t)(e+1023)) << 52) | (m & ((1ULL << 52)-1));
      }
   }

   return u.f;
}


float floorf(float value)
{
   return value >= 0.0f? ftrunc(value) : ftrunc_up(value);
}


float ceilf(float value)
{
   return value >= 0.0f? ftrunc_up(value) : ftrunc(value);
}


float roundf(float value)
{
   return ftrunc(value >= 0.0f? value + 0.5f : value - 0.5f);
}


double floor(double value)
{
   return value >= 0.0f? dtrunc(value) : dtrunc_up(value);
}


double ceil(double value)
{
   return value >= 0.0f? dtrunc_up(value) : dtrunc(value);
}


double round(double value)
{
   return dtrunc(value >= 0.0f? value + 0.5f : value - 0.5f);
}


// https://en.wikipedia.org/wiki/Exponentiation_by_squaring#With_constant_auxiliary_memory

static double exp_sqr(double x, int n)
{
   double y = 1.0;

   if (n == 0) {
      return 1.0;
   }
   while (n > 1) {
      if (n & 1) {
         y *= x;
         x *= x;
      }
      else {
         x *= x;
      }
      n >>= 1;
   }
   return x * y;
}


// https://en.wikipedia.org/wiki/Exponential_function#Computation

static double exp_taylor(double x)
{
   double x2, x3, x4, x8;
   
   x2 = x*x;
   x3 = x2*x;
   x4 = x2*x2;
   x8 = x4*x4;

   return (
      1.0 + x +
      x2 * 0.5 +
      x3 * 0.16666666666666667 +
      x4 * 0.041666666666666667 +
      x4*x * 0.0083333333333333333 +
      x4*x2 * 0.0013888888888888889 +
      x4*x3 * 0.00019841269841269841 +
      x8 * 0.000024801587301587302 +
      x8*x * 0.0000027557319223985891 +
      x8*x2 * 0.00000027557319223985891 +
      x8*x3 * 0.000000025052108385441719
   );
}


float expf(float x)
{
   return exp(x);
}


double exp(double value)
{
   union {
      double f;
      uint64_t i;
   } u;
   double n, frac, result, taylor;
   int neg = 0;

   if (value < 0.0) {
      value = -value;
      neg = 1;
   }

   n = dtrunc(value);
   if (n > 709.0) {
      u.i = 0x7FFULL << 52; // inf
      return u.f;
   }
   frac = value - n;

   result = exp_sqr(2.7182818284590452, n);
   taylor = exp_taylor(frac * 0.25);
   taylor *= taylor;
   taylor *= taylor;
   result *= taylor;

   if (neg) {
      return 1.0 / result;
   }
   return result;
}


float log2f(float x)
{
   return log2(x);
}


// https://en.wikipedia.org/wiki/Binary_logarithm#Iterative_approximation

double log2(double value)
{
   union {
      double f;
      uint64_t i;
   } u;
   double m, tmp, result;
   int e, i, cnt;

   u.f = value;
   if (u.i >> 63) {
      u.i = (0x7FFULL << 52) | (1ULL << 51); // nan
      return u.f;
   }
   if (u.i == 0) {
      u.i = (0x7FFULL << 52) | (1ULL << 63); // -inf
      return u.f;
   }

   e = ((u.i >> 52) & 0x7FF) - 1023;
   u.i = (u.i & ((1ULL<<52)-1)) | (1023ULL << 52);
   m = u.f;

   result = e;
   e = 0;

   for (i=0; i<64; i++) {
      if (m == 1.0) break;
      cnt = 0;
      do {
         m *= m;
         cnt++;
      }
      while (m < 2.0);
      e -= cnt;
      if (e <= -1023) break;

      u.i = (uint64_t)(e + 1023) << 52;
      tmp = result + u.f;
      if (tmp == result) break;
      result = tmp;
      m *= 0.5;
   }

   return result;
}


float log10f(float x)
{
   return log10(x);
}


double log10(double value)
{
   return log2(value) * 0.30102999566398120;
}


float logf(float x)
{
   return log(x);
}


double log(double value)
{
   return log2(value) * 0.69314718055994531;
}


float powf(float x, float y)
{
   return pow(x, y);
}


double pow(double x, double y)
{
   if (x == 0.0) {
      return x;
   }
   if (y == 0.0) {
      return 1.0;
   }
   return exp(log(x) * y);
}


float sqrtf(float x)
{
   return sqrt(x);
}


double sqrt(double x)
{
   return pow(x, 0.5);
}


float cbrtf(float x)
{
   return cbrt(x);
}


double cbrt(double x)
{
   if (x < 0) {
      return -pow(-x, 0.33333333333333333);
   }
   return pow(x, 0.33333333333333333);
}


// https://en.wikipedia.org/wiki/Sine_and_cosine#Series_definitions

static double sin_taylor(double x)
{
   double x2, x3, x4, x5, x7, x8, x9;

   x2 = x*x;
   x3 = x2*x;
   x4 = x2*x2;
   x5 = x3*x2;
   x7 = x4*x3;
   x8 = x4*x4;
   x9 = x5*x4;

   return (
      x -
      x3 * 0.16666666666666667 +
      x5 * 0.0083333333333333333 -
      x7 * 0.00019841269841269841 +
      x9 * 0.0000027557319223985891 -
      x8*x3 * 0.000000025052108385441719 +
      x8*x5 * 0.00000000016059043836821615 -
      x8*x7 * 0.00000000000076471637318198165 +
      x9*x8 * 0.0000000000000028114572543455208 -
      x8*x8*x3 * 0.0000000000000000082206352466243297
   );
}


float sinf(float x)
{
   return sin(x);
}


double sin(double x)
{
   double tmp;
   int neg = 0, quadrant;
   
   if (x < 0.0) {
      x = -x;
      neg = 1;
   }

   tmp = dtrunc(x * 0.63661977236758134); // 1.0/(pi/2)
   x = x - tmp * 1.5707963267948966;
   tmp *= 0.25;
   quadrant = (tmp - dtrunc(tmp)) * 4.0;

   if (quadrant == 1 || quadrant == 3) {
      x = 1.5707963267948966 - x;
   }
   x = sin_taylor(x);
   if (quadrant == 2 || quadrant == 3) {
      x = -x;
   }
   if (neg) {
      x = -x;
   }
   return x;
}


float cosf(float x)
{
   return cos(x);
}


double cos(double x)
{
   return sin(x + 1.5707963267948966);
}


float tanf(float x)
{
   return tan(x);
}


double tan(double x)
{
   return sin(x) / cos(x);
}


// https://en.wikipedia.org/wiki/Inverse_trigonometric_functions#Infinite_series

static double asin_leibniz(double x)
{
   double x2, x3, x4, x5, x7, x8, x9, x11, x13, x15, x16, x17, x24;

   x2 = x*x;
   x3 = x2*x;
   x4 = x2*x2;
   x5 = x3*x2;
   x7 = x4*x3;
   x8 = x4*x4;
   x9 = x5*x4;
   x11 = x8*x3;
   x13 = x8*x5;
   x15 = x8*x7;
   x16 = x8*x8;
   x17 = x9*x8;
   x24 = x16*x8;

   return (
      x +
      x3 * 0.16666666666666667 +
      x5 * 0.075 +
      x7 * 0.044642857142857144 +
      x9 * 0.030381944444444444 +
      x11 * 0.022372159090909091 +
      x13 * 0.017352764423076923 +
      x15 * 0.01396484375 +
      x17 * 0.011551800896139706 +
      x16*x3 * 0.0097616095291940789 +
      x16*x5 * 0.0083903358096168155 +
      x16*x7 * 0.0073125258735988451 +
      x16*x9 * 0.0064472103118896484 +
      x16*x11 * 0.0057400376708419235 +
      x16*x13 * 0.0051533096823199042 +
      x16*x15 * 0.0046601434869150962 +
      x24*x9 * 0.0042409070936793631 +
      x24*x11 * 0.0038809645588376692 +
      x24*x13 * 0.0035692053938259345 +
      x24*x15 * 0.0032970595034734847
   );
}


float asinf(float x)
{
   return asin(x);
}


double asin(double x)
{
   union {
      double f;
      uint64_t i;
   } u;
   int neg = 0, invert = 0;

   if (x < 0.0) {
      x = -x;
      neg = 1;
   }
   if (x > 1.0) {
      u.i = (0x7FFULL << 52) | (1ULL << 51); // nan
      return u.f;
   }
   if (x > 0.5) {
      x = sqrt((1.0 - x) * 0.5);
      invert = 1;
   }
   x = asin_leibniz(x);
   if (invert) {
		x = 1.5707963267948966 - x * 2.0;
   }
   if (neg) {
      x = -x;
   }
   return x;
}


float acosf(float x)
{
   return acos(x);
}


double acos(double x)
{
   return 1.5707963267948966 - asin(x);
}


// https://en.wikipedia.org/wiki/Inverse_trigonometric_functions#Infinite_series

static double atan_leibniz(double x)
{
   double x2, x3, x4, x5, x7, x8, x9;

   x2 = x*x;
   x3 = x2*x;
   x4 = x2*x2;
   x5 = x3*x2;
   x7 = x4*x3;
   x8 = x4*x4;
   x9 = x5*x4;

   return (
      x -
      x3 * 0.33333333333333333 +
      x5 * 0.2 -
      x7 * 0.14285714285714286 +
      x9 * 0.11111111111111111 -
      x8*x3 * 0.090909090909090909 +
      x8*x5 * 0.076923076923076923 -
      x8*x7 * 0.066666666666666667 +
      x9*x8 * 0.058823529411764706 -
      x8*x8*x3 * 0.052631578947368421 +
      x8*x8*x5 * 0.047619047619047619 -
      x8*x8*x7 * 0.043478260869565217
   );
}


float atanf(float x)
{
   return atan(x);
}


// https://en.wikipedia.org/wiki/Inverse_trigonometric_functions#Arctangent_addition_formula

double atan(double x)
{
   double add;
   int neg = 0, invert = 0, adjust1 = 0, adjust2 = 0;
   
   if (x < 0.0) {
      x = -x;
      neg = 1;
   }
   if (x > 1.0) {
      x = 1.0 / x;
      invert = 1;
   }
   if (x > 0.5) {
      add = -0.54630248984379051; // tan(-0.5)
      x = (x + add) / (1.0 - x * add);
      adjust1 = 1;
   }
   if (x > 0.25) {
      add = -0.25534192122103627; // tan(-0.25)
      x = (x + add) / (1.0 - x * add);
      adjust2 = 1;
   }
   x = atan_leibniz(x);
   if (adjust2) {
      x += 0.25;
   }
   if (adjust1) {
      x += 0.5;
   }
   if (invert) {
      x = 1.5707963267948966 - x;
   }
   if (neg) {
      x = -x;
   }
   return x;
}


float atan2f(float y, float x)
{
   return atan2(y, x);
}

double atan2(double y, double x)
{
   union {
      double f;
      uint64_t i;
   } u;
   double angle;
   int ys, xs;

   u.f = y;
   ys = u.i >> 63;
   u.f = x;
   xs = u.i >> 63;

   if (x == 0.0 && y == 0.0) {
      if (xs) {
         return ys? -3.1415926535897932 : 3.1415926535897932;
      }
      else {
         return ys? -0.0 : 0.0;
      }
   }

   if (ys) {
      y = -y;
   }
   angle = atan(y / x);
   if (xs) {
      angle = 3.1415926535897932 + angle;
   }
   if (ys) {
      angle = -angle;
   }
   return angle;
}


float fabsf(float x)
{
   union {
      float f;
      uint32_t i;
   } u;
   
   u.f = x;
   u.i &= 0x7FFFFFFF;
   return u.f;
}


double fabs(double x)
{
   union {
      double f;
      uint64_t i;
   } u;
   
   u.f = x;
   u.i &= 0x7FFFFFFFFFFFFFFFULL;
   return u.f;
}


float fminf(float x, float y)
{
   return x < y? x : y;
}


double fmin(double x, double y)
{
   return x < y? x : y;
}


float fmaxf(float x, float y)
{
   return x > y? x : y;
}


double fmax(double x, double y)
{
   return x > y? x : y;
}