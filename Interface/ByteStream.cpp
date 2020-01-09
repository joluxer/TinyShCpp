#include "ByteStream.h"

ByteStream::ByteStream()
{

}

ByteStream::~ByteStream()
{

}

unsigned ByteStream::write(unsigned char b)
{
  return 1;
}

unsigned ByteStream::writeBlock(const unsigned char *b, unsigned int numBytes)
{
  unsigned int i = 0;
  unsigned n = 1;

  // writeBlock data as long as more data and room is available
  while ((numBytes > 0) && (n > 0))
  {
    n = write(b[i]);
    i += n;
    numBytes -= n;
  }

  return i;
}

unsigned ByteStream::writeBlock(const char *string)
{
  unsigned int i = 0;
  unsigned n;

  // writeBlock data until the end of the string (NULL terminated)
  while (string[i])
  {
    i++;
  }

  n = writeBlock(string, i);

  return n;
}

unsigned ByteStream::read(unsigned char &b)
{
  return 0;
}

unsigned ByteStream::readBlock(unsigned char *b, unsigned int numBytes)
{
  unsigned n = 1;
  unsigned int i = 0;

  // writeBlock data as long as more data and room is available
  while (n && (numBytes-- > 0))
  {
    n = read(b[i]);
    i += n;
  }

  return i;
}

int ByteStream::get_char()
{
  unsigned char c;

  if (read(c))
  {
    return c;
  }
  return -1;
}

int ByteStream::put_char(int c)
{
  unsigned char b = c;
  if (write(b))
  {
    return c;
  }
  return -1;
}
