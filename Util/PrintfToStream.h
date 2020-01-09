/*
 * PrintfToStream.h
 *
 *  Created on: 29.11.2011
 *      Author: johannes
 */

#ifndef PRINTFTOSTREAMSTREAM_H_
#define PRINTFTOSTREAMSTREAM_H_

#include "ByteStreamDecorator.h"
#include <stdarg.h>
#include <stdint.h>

/***
 * Diese Klasse stellt einen Decorator bereit, der dem übergebenen ByteStream Printf-Fähigkeiten
 * hinzufügt.
 *
 * Die Klasse unterstützt folgende Formate:
 * %s, %d, %x, %X, %u, %c, %f
 */

class PrintfToStream: public ByteStreamDecorator
{
public:
  PrintfToStream(ByteStream& stream);
//  virtual ~PrintfToStream();

  /***
   * Schreibt anhand des gegebenen Format-Strings Zeichen in den Ausgangs-Datenstrom, die Argumente-Liste
   * wird anhand des Formatstrings in den Ausgangs-Datenstrom formatiert eingefügt.
   */
  int vprintf(const char * fmt, va_list args);

  /***
   * Schreibt anhand des gegebenen Format-Strings Zeichen in den Ausgangs-Datenstrom, die
   * variablen Argumente werden anhand des Formatstrings in den Ausgangs-Datenstrom formatiert eingefügt.
   *
   * Es wird die vprintf()-Methode gerufen.
   */
  int printf(const char * fmt, ...);

protected:
  typedef unsigned int uint;

  int prints(const char *string, unsigned width, unsigned pad);
  int printi(intptr_t i, int base, int sign, int width, int pad, int letterBase);
  unsigned dbl2stri(double dbl, unsigned width, unsigned dec_digits, int pad);

};

#endif /* PRINTFTOSTREAMSTREAM_H_ */
