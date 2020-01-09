/*
 * ByteStreamDecorator.cpp
 *
 */

#include "ByteStreamDecorator.h"

ByteStreamDecorator::ByteStreamDecorator(ByteStream& lower)
: backendIo(lower)
{}

//ByteStreamDecorator::~ByteStreamDecorator()
//{
//  // Auto-generated destructor stub
//}

unsigned ByteStreamDecorator::read(unsigned char &b)
{
  return backendIo.read(b);
}

unsigned ByteStreamDecorator::readBlock(unsigned char *b, unsigned numBytes)
{
  return backendIo.readBlock(b, numBytes);
}

unsigned ByteStreamDecorator::write(unsigned char b)
{
  return backendIo.write(b);
}

unsigned ByteStreamDecorator::writeBlock(const unsigned char *b, unsigned numBytes)
{
  return backendIo.writeBlock(b, numBytes);
}

