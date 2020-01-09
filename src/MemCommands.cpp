/*
 * mem-commands.c
 *
 */

#include "MemCommands.h"


#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#ifdef NDEBUG
#undef DEBUG
#else
#ifndef DEBUG
#define DEBUG
#endif
#endif

#include "Util/PrintfToStream.h"

namespace Shell
{
static unsigned ptr = 0;

extern
unsigned char *memCmdsBasePtr;
unsigned char *memCmdsBasePtr = 0;

#ifdef DEBUG
static
void cmd_mapTest(TinySh& shell, int argc, const char **argv)
{
  static unsigned char* obj = 0;
  static unsigned objSize = 64;

  PrintfToStream fio(shell.io());

  if (0 == obj)
  {
    unsigned i;
    obj = (unsigned char*)malloc(objSize);
    for (i = 0; i < objSize; i++)
    {
      obj[i] = i;
    }
    memCmdsBasePtr = obj;
  }

  fio.printf("addr: 0x%08lx size: %u (0x%04x)\n", (intptr_t)obj, objSize, objSize);
}
#endif // DEBUG

static
void cmd_hexdump(TinySh& shell, int argc, const char **argv)
{
  static unsigned dumplen = 64;

  PrintfToStream fio(shell.io());

  if ((2 <= argc) && (3 >= argc))
  {
    ptr = TinySh::atoxi(argv[1]);

    if (3 == argc)
    {
      dumplen = TinySh::atoxi(argv[2]);
    }
  }

  unsigned char *buf = memCmdsBasePtr + ptr;
  unsigned i, j;
  for (i = 0; i < dumplen; i += 16)
  {
    fio.printf("%08lx: ", (intptr_t)(buf + i));
    for (j = 0; j < 16; j++)
      if (i + j < dumplen)
        fio.printf("%02x ", buf[i + j]);
      else
        fio.printf("   ");
    fio.printf(" ");
    for (j = 0; j < 16; j++)
      if (i + j < dumplen)
        fio.printf("%c", isprint(buf[i+j]) ? buf[i + j] : '.');
    fio.printf("\n");
  }

  ptr += dumplen;
}

static
void cmd_setBase(TinySh& shell, int argc, const char **argv)
{
  PrintfToStream fio(shell.io());

  if (2 == argc)
  {
    memCmdsBasePtr = (unsigned char*)TinySh::atoxi(argv[1]);
  }
  fio.printf("base addr: 0x%08lx\n", (intptr_t)memCmdsBasePtr);
}

static
void cmd_copy(TinySh& shell, int argc, const char **argv)
{
  if (4 == argc)
  {
    unsigned i;
    unsigned len = TinySh::atoxi(argv[3]);
    unsigned dest = TinySh::atoxi(argv[2]);
    ptr = TinySh::atoxi(argv[1]);

    for (i = 0; i < len; ++i)
    {
      memCmdsBasePtr[dest + i] = memCmdsBasePtr[ptr + i];
    }
  }
}

static
void cmd_comp(TinySh& shell, int argc, const char **argv)
{
  PrintfToStream fio(shell.io());

  if (4 == argc)
  {
    unsigned i, differences;
    intptr_t diff = (intptr_t)shell.get_arg();
    unsigned len = TinySh::atoxi(argv[3]);
    unsigned dest = TinySh::atoxi(argv[2]);
    ptr = TinySh::atoxi(argv[1]);

    differences = 0;
    for (i = 0; i < len; ++i)
    {
      if (memCmdsBasePtr[dest + i] != memCmdsBasePtr[ptr + i])
      {
        differences++;
        if (diff)
        {
          unsigned char c1 = memCmdsBasePtr[ptr + i];
          unsigned char c2 = memCmdsBasePtr[dest + i];
          fio.printf("found difference 0x%08lx: %02x %c | 0x%08lx: %02x %c\n", (intptr_t)memCmdsBasePtr[ptr + i], c1, isprint(c1)?c1:'.', (intptr_t)&memCmdsBasePtr[dest + i], c2, isprint(c2)?c2:'.');
        }
      }
    }
    fio.printf("total %u differences\n", differences);
  }
}

static
void cmd_readMem(TinySh& shell, int argc, const char **argv)
{
  PrintfToStream fio(shell.io());

  intptr_t opType = (intptr_t)shell.get_arg();
//  unsigned long mask = ~(-1 << (numBytes * 8));
  const char* format;
  unsigned count = 1;
  unsigned i;
  unsigned long value;
  intptr_t addr;

  if (2 == argc)
  {
    ptr = TinySh::atoxi(argv[1]);
  }
  else if (3 == argc)
  {
    ptr = TinySh::atoxi(argv[1]);
    count = TinySh::atoxi(argv[2]);
  }

//  // alignment
//  {
//    unsigned alignMask = (-1 << opType);
//    ptr &= alignMask;
//  }

  for (i = 0; i < count; ++i)
  {
    switch (opType)
    {
    default:
    case 0:
      format = "0x%08lx: 0x%02x\n";
      value = memCmdsBasePtr[ptr + i];
      addr = (intptr_t)(&memCmdsBasePtr[ptr + i]);
      break;

    case 1:
      format = "0x%08lx: 0x%04x\n";
      value = *((unsigned short*)(&memCmdsBasePtr[ptr]) + i);
      addr = (intptr_t)((unsigned short*)(&memCmdsBasePtr[ptr]) + i);
      break;

    case 2:
      format = "0x%08lx: 0x%08x\n";
      value = *((unsigned long*)(&memCmdsBasePtr[ptr]) + i);
      addr = (intptr_t)((unsigned long*)(&memCmdsBasePtr[ptr]) + i);
      break;

    }

    fio.printf(format, addr, value);
  }
}

static
void cmd_writeMem(TinySh& shell, int argc, const char **argv)
{
  if (2 < argc)
  {
    intptr_t opType = (intptr_t)shell.get_arg();
    unsigned numBytes = 1 << opType;
    unsigned long mask = ~(-1 << (numBytes * 8));
    unsigned count;
    unsigned i;
    unsigned long value;

    ptr = TinySh::atoxi(argv[1]);
    count = argc - 2;
    i = 0;

//    // alignment
//    {
//      unsigned alignMask = (-1 << opType);
//      ptr &= alignMask;
//    }

    while (0 < count)
    {
      value = (unsigned long)TinySh::atoxi(argv[i + 2]);
      value &= mask;
      switch (opType)
      {
      case 0:
        memCmdsBasePtr[ptr + i] = (unsigned char)value;
        break;

      case 1:
        *((unsigned short*)&memCmdsBasePtr[ptr] + i) = (unsigned short)value;
        break;

      case 2:
        *((unsigned long*)&memCmdsBasePtr[ptr] + i) = (unsigned long)value;
        break;

      }

      i++;
      count--;
    }
  }
}

static
void cmd_fillMem(TinySh& shell, int argc, const char **argv)
{
  if ((2 < argc) && (4 >= argc))
  {
    intptr_t opType = (intptr_t)shell.get_arg();
    unsigned numBytes = 1 << opType;
    unsigned long mask = ~(-1 << (numBytes * 8));
    unsigned count = 1;
    unsigned long value;
    unsigned i;

    ptr = TinySh::atoxi(argv[1]);
    value = TinySh::atoxi(argv[2]);
    value &= mask;
    if (4 == argc)
    {
      count = TinySh::atoxi(argv[3]);
    }

//    // alignment
//    {
//      unsigned alignMask = (-1 << opType);
//      ptr &= alignMask;
//    }

    i = 0;
    while (0 < count)
    {
      switch (opType)
      {
      case 0:
        memCmdsBasePtr[ptr + i] = (unsigned char)value;
        break;

      case 1:
        *((unsigned short*)&memCmdsBasePtr[ptr] + i) = (unsigned short)value;
        break;

      case 2:
        *((unsigned long*)&memCmdsBasePtr[ptr] + i) = (unsigned long)value;
        break;

      }

      i++;
      count--;
    }
  }
}

static
void cmd_memModify(TinySh& shell, int argc, const char **argv)
{
  if (4 == argc)
  {
    intptr_t opType = (intptr_t)shell.get_arg();
    unsigned numBytes = 1 << opType;
    unsigned long mask = ~(-1 << (numBytes * 8));
    unsigned long value;
    unsigned long operand = 0;
    char c;

    ptr = TinySh::atoxi(argv[1]);
    value = TinySh::atoxi(argv[3]);
    value &= mask;

//    // alignment
//    {
//      unsigned alignMask = (-1 << opType);
//      ptr &= alignMask;
//    }

    switch (opType)
    {
    default:
    case 0:
      operand = memCmdsBasePtr[ptr];
      break;

    case 1:
      operand = *(uint16_t*)&memCmdsBasePtr[ptr];
      break;

    case 2:
      operand = *(uint32_t*)&memCmdsBasePtr[ptr];
      break;

    }

    c = argv[2][0];
    switch (c)
    {
    case '=':
      operand = value;
      break;

    case '<':
      operand <<= value;
      break;

    case '>':
      operand >>= value;
      break;

    case '&':
      operand &= value;
      break;

    case '|':
      operand |= value;
      break;

    case '^':
      operand ^= value;
      break;

    case '+':
      operand += value;
      break;

    case '-':
      operand -= value;
      break;

    case '*':
      operand *= value;
      break;

    case '/':
      operand /= value;
      break;

    case '%':
      operand %= value;
      break;

    default:
      break;
    }

    switch (opType)
    {
    default:
    case 0:
      memCmdsBasePtr[ptr] = (uint8_t)operand;
      break;

    case 1:
      *((uint16_t*)&memCmdsBasePtr[ptr]) = (uint16_t)operand;
      break;

    case 2:
      *((uint32_t*)&memCmdsBasePtr[ptr]) = (uint32_t)operand;
      break;

    }
  }
}

#ifdef DEBUG
static const CommandDescription testCmd = { "testArea", "map a test memory area, set base address and return address and size", 0, &cmd_mapTest, 0, 0, 0 };
#endif
static const CommandDescription mod32Cmd = { "mod", "modify long", "addr <C assignment op> value", &cmd_memModify, (void *)2, 0 , 0 };
static const CommandDescription fill32Cmd = { "fill", "write long(s)", "addr value [count:1]", &cmd_fillMem, (void *)2, (CommandDescription*)&mod32Cmd, 0 };
static const CommandDescription wr32Cmd = { "write", "write long(s)", "addr value [value [...]]", &cmd_writeMem, (void *)2, (CommandDescription*)&fill32Cmd, 0 };
static const CommandDescription rd32Cmd = { "read", "read long(s)", "[addr [count:1]]", &cmd_readMem, (void *)2, (CommandDescription*)&wr32Cmd, 0 };
#ifdef DEBUG
static const CommandDescription dwordCmd = { "long", "work on int32", 0, 0, 0, (CommandDescription*)&testCmd, (CommandDescription*)&rd32Cmd };
#else
static const CommandDescription dwordCmd = { "long", "work on int32", 0, 0, 0, 0, (CommandDescription*)&rd32Cmd };
#endif
static const CommandDescription mod16Cmd = { "mod", "modify short", "addr <C assignment op> value", &cmd_memModify, (void *)1, 0 , 0 };
static const CommandDescription fill16Cmd = { "fill", "write short(s)", "addr value [count:1]", &cmd_fillMem, (void *)1, (CommandDescription*)&mod16Cmd, 0 };
static const CommandDescription wr16Cmd = { "write", "write short(s)", "addr value [value [...]]", &cmd_writeMem, (void *)1, (CommandDescription*)&fill16Cmd, 0 };
static const CommandDescription rd16Cmd = { "read", "read short(s)", "[addr [count:1]]", &cmd_readMem, (void *)1, (CommandDescription*)&wr16Cmd, 0 };
static const CommandDescription wordCmd = { "short", "work on int16", 0, 0, 0, (CommandDescription*)&dwordCmd, (CommandDescription*)&rd16Cmd };
static const CommandDescription mod8Cmd = { "mod", "modify byte", "addr <C assignment op> value", &cmd_memModify, (void *)0, 0 , 0 };
static const CommandDescription fill8Cmd = { "fill", "write byte(s)", "addr value [count:1]", &cmd_fillMem, (void *)0, (CommandDescription*)&mod8Cmd, 0 };
static const CommandDescription wr8Cmd = { "write", "write byte(s)", "addr value [value [...]]", &cmd_writeMem, (void *)0, (CommandDescription*)&fill8Cmd, 0 };
static const CommandDescription rd8Cmd = { "read", "read byte(s)", "[addr [count:1]]", &cmd_readMem, (void *)0, (CommandDescription*)&wr8Cmd, 0 };
static const CommandDescription byteCmd = { "byte", "work on bytes", 0, 0, 0, (CommandDescription*)&wordCmd, (CommandDescription*)&rd8Cmd };
static const CommandDescription diffCmd = { "diff", "display memory differences", "addr1 addr2 count", &cmd_comp, (void *)1, (CommandDescription*)&byteCmd, 0 };
static const CommandDescription compCmd = { "cmp", "compare memory bytes", "addr1 addr2 count", &cmd_comp, 0, (CommandDescription*)&diffCmd, 0 };
static const CommandDescription copyCmd = { "cp", "copy memory bytes", "src dest count", cmd_copy, 0, (CommandDescription*)&compCmd, 0 };
static const CommandDescription dumpCmd = { "hexdump", "dump memory bytes in hex (with base addr)", "[addr [num:64]]", &cmd_hexdump, 0, (CommandDescription*)&copyCmd, 0 };
const CommandDescription memCommands = { "base", "set or display base address for memory operations", "[addr]", &cmd_setBase, 0, (CommandDescription*)&dumpCmd, 0 };
CommandDescription memCmdGroup = { "mem", "manipulate memory relative to a base address", "sub_cmd", 0, 0, 0, (CommandDescription*)&memCommands };
}

