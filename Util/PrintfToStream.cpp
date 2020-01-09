/*
 * PrintfToStream.cpp
 *
 *  Created on: 29.11.2011
 *      Author: johannes
 *
 *  This is modified an open source embedded printf() for use in this class.
 */

//*******************************************************************************
//  BUGS
//  If '%' is included in a format string, in the form \% with the intent
//  of including the percent sign in the output string, this function will
//  mis-handle the data entirely!!
//  Essentially, it will just discard the character following the percent sign.
//  This bug is not easy to fix in the existing code;
//  for now, I'll just try to remember to use %% instead of \% ...
//*******************************************************************************

//lint -esym(752, debug_output)
//lint -esym(766, stdio.h)

#include <stdarg.h>

#include "PrintfToStream.h"

PrintfToStream::PrintfToStream(ByteStream& stream)
: ByteStreamDecorator(stream)
{}

//PrintfToStream::~PrintfToStream()
//{
//  // Auto-generated destructor stub
//}

////****************************************************************************
//static uint my_strlen(char *str)
//{
//  if (str == 0)
//    return 0;
//  uint slen = 0;
//  while (*str != 0)
//  {
//    slen++;
//    str++;
//  }
//  return slen;
//}

//****************************************************************************
#define PAD_RIGHT   1
#define PAD_ZERO    2
#define PAD_PLUS    4
#define PAD_8BIT    8
#define PAD_16BIT   16
#define PAD_32BIT   32
#define PAD_64BIT   64
#define PAD_LONG    128

int PrintfToStream::prints(const char *string, unsigned width, unsigned pad)
{
  register int pc = 0, padchar = ' ';

  if (width > 0)
  {
    register unsigned len = 0;
    register const char *ptr;
    for (ptr = string; *ptr; ++ptr)
    {
      ++len;
    }

    if (len >= width)
    {
      width = 0;
    }
    else
    {
      width -= len;
    }

    if (pad & PAD_ZERO)
    {
      padchar = '0';
    }
  }
  if (!(pad & PAD_RIGHT))
  {
    for (; width > 0; --width)
    {
      put_char(padchar);
      ++pc;
    }
  }
  for (; *string; ++string)
  {
    put_char(*string);
    ++pc;
  }
  for (; width > 0; --width)
  {
    put_char(padchar);
    ++pc;
  }

  return pc;
}

//****************************************************************************
/* the following should be enough for int on 64 and 32 bit arch */
#define PRINT_BUF_LEN (sizeof(intptr_t) == 8 ? 22 : 12)

/***
 * print an integer
 * @param i integer to print
 * @param base base to use for print
 * @param sign integer is signed
 * @param width minimum print width
 * @param pad pad flags
 * @param letbase
 * @return
 */
int PrintfToStream::printi(intptr_t i, int base, int sign, int width, int pad, int letterBase)
{
  char print_buf[PRINT_BUF_LEN];
  register char *s;
  register int t, neg = 0, pc = 0;
  register uintptr_t u = i;

  if (i == 0)
  {
    print_buf[0] = '0';
    print_buf[1] = '\0';
    return prints(print_buf, width, pad);
  }

  if (0 == (pad & PAD_LONG))
  {
    i = (int)i;
    u = (unsigned int)u;
  }

  if (sign && base == 10 && i < 0)
  {
    neg = 1;
    u = (uintptr_t) -i;
  }

  //  make sure print_buf is NULL-term
  s = print_buf + PRINT_BUF_LEN - 1;
  *s = '\0';

  while (u)
  {
    t = u % base; //lint !e573  Warning 573: Signed-unsigned mix with divide
    if (t >= 10)
      t += letterBase - '0' - 10;
    *--s = t + '0';
    u /= base; //lint !e573  Warning 573: Signed-unsigned mix with divide
  }

  if (neg)
  {
    if (width && (pad & PAD_ZERO))
    {
      put_char('-');
      ++pc;
      --width;
    }
    else
    {
      *--s = '-';
    }
  }
  else
  {
    if (pad & PAD_PLUS)
    {
      *--s = '+';
    }
  }

  return pc + prints(s, width, pad);
}

//****************************************************************************
//  This version returns the length of the output string.
//  It is more useful when implementing a walking-string function.
//****************************************************************************
unsigned PrintfToStream::dbl2stri(double dbl, unsigned width, unsigned dec_digits, int pad)
{
  int pc = 0;
  int sign = 1;

  //*******************************************
  //  extract negative info
  //*******************************************
  if (dbl < 0.0)
  {
    dbl *= -1.0;
    sign = -1;
  }

  //**************************************************************************
  //  construct fractional multiplier for specified number of digits.
  //**************************************************************************
  uint mult = 1;
  uint idx;
  for (idx = 0; idx < dec_digits; idx++)
  {
    mult *= 10;
  }

  uint wholeNum = (uint) dbl;
  uint decimalNum = (uint) (((dbl - wholeNum) * mult) + 0.5); // handle rounding by adding 0.5 LSB

  if (decimalNum >= mult)
  {
    decimalNum -= mult;
    wholeNum++;
  }

  //*******************************************
  //  convert integer portion
  //*******************************************
  int intWidth;
  intWidth = width - dec_digits;
  if (dec_digits > 0)
  {
    if (pad & PAD_RIGHT)
    {
      intWidth = 0;
    }
    else
    {
      intWidth -= 1;
      if (intWidth < 0)
      {
        intWidth = 0;
      }
    }
  }
  pc = printi(wholeNum * sign, 10, 1, intWidth, pad, 'a');
  if (dec_digits > 0)
  {
    put_char('.');
    pc++;

    if (pad & PAD_RIGHT)
    {
      dec_digits = width - pc;
      pad = PAD_RIGHT;
    }
    else
    {
      pad = PAD_ZERO;
    }

    //*******************************************
    //  convert fractional portion
    //*******************************************
    pc += printi(decimalNum, 10, 0, dec_digits, pad, 'a');
  }

  return pc;
}

//****************************************************************************
int PrintfToStream::vprintf(const char *format, va_list args)
{
  register int width, pad;
  register int pc = 0;
  char scr[2];
  unsigned dec_width = 0;
  int post_decimal, isLong;
  for (; *format != 0; ++format)
  {
    if (*format == '%')
    {
      ++format;
      width = pad = isLong = 0;
      if (*format == '\0')
        break;
      if (*format == '%')
      {
        put_char(*format);
        ++pc;
        continue;
      }
      if (*format == '-')
      {
        ++format;
        pad |= PAD_RIGHT;
      }
      if (*format == '+')
      {
        ++format;
        pad |= PAD_PLUS;
      }
      while (*format == '0')
      {
        ++format;
        pad |= PAD_ZERO;
      }
      post_decimal = 0;
      if (*format == '.' || (*format >= '0' && *format <= '9'))
      {
        while (1)
        {
          if (*format == '.')
          {
            post_decimal = 1;
            dec_width = 0;
            format++;
          }
          else if ((*format >= '0' && *format <= '9'))
          {
            if (post_decimal)
            {
              dec_width *= 10;
              dec_width += *format - '0';
            }
            else
            {
              width *= 10;
              width += *format - '0';
            }
            format++;
          }
          else
          {
            break;
          }
        }
      }
      if ((*format == 't') || (*format == 'l'))
      {
        ++format;
#ifndef PRINTF_NO_LONG
        isLong = 1;
#endif
      }
      switch (*format)
      {
      case 's':
        {
          // char *s = *((char **) varg++);   //lint !e740
          /*register*/ char *s = va_arg(args, char*); //lint !e740 !e826  convert to double pointer
          pc += prints(s ? s : "(null)", width, pad);
        }
        break;
      case 'd':
        if (!isLong)
          pc += printi(va_arg(args, int), 10, 1, width, pad, 'a');
        else
          pc += printi(va_arg(args, long), 10, 1, width, pad | PAD_LONG, 'a');
        break;
      case 'x':
        if (!isLong)
          pc += printi(va_arg(args, int), 16, 0, width, pad, 'a');
        else
          pc += printi(va_arg(args, long), 16, 0, width, pad | PAD_LONG, 'a');
        break;
      case 'X':
        if (!isLong)
          pc += printi(va_arg(args, int), 16, 0, width, pad, 'A');
        else
          pc += printi(va_arg(args, long), 16, 0, width, pad | PAD_LONG, 'A');
        break;
      case 'u':
        if (!isLong)
          pc += printi(va_arg(args, unsigned int), 10, 0, width, pad, 'a');
        else
          pc += printi(va_arg(args, unsigned long), 10, 0, width, pad | PAD_LONG, 'a');
        break;
      case 'c':
        /* char are converted to int then pushed on the stack */
        scr[0] = va_arg(args, int);
        scr[1] = '\0';
        pc += prints(scr, width, pad);
        break;

      case 'f':
        {
          double dbl = va_arg(args, double); //lint !e740 !e826  convert to double pointer
          pc += dbl2stri(dbl, width, dec_width, pad);
        }
        break;

      default:
        put_char('%');
        put_char(*format);
        break;
      }
    }
    else
    {
      put_char(*format);
      ++pc;
    }
  } //  for each char in format string
  return pc;
}

int PrintfToStream::printf(const char *format, ...)
{
  int ret;
  va_list args;

  va_start( args, format );
  ret = vprintf(format, args);
  va_end(args);
  return ret;
}

