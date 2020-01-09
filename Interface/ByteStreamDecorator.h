/*
 * ByteStreamDecorator.h
 *
 */

#ifndef BYTESTREAMDECORATOR_H_
#define BYTESTREAMDECORATOR_H_

#include "ByteStream.h"

/**
 * Dies ist eine simple Basis-Klasse für die Dekoration-Klasse für ByteStream-Objekte.
 *
 * Die Default-Implementationen der virtuellen Methoden sind einfache Forwarder, die möglichst
 * performant die Aufrufe weiterreichen. Reimplementationen virtueller Methoden ermöglichen dann
 * die Dekoration des backendIo-Objektes.
 *
 * Es müssen alle Routinen einer Datenrichtung überladen werden, sonst wird ggf. nicht korrekt dekoriert.
 */
class ByteStreamDecorator: public ByteStream
{
public:
  ByteStreamDecorator(ByteStream& lowerStream);
//  virtual
//  ~ByteStreamDecorator();

  // ByteStream Interface
  /**
   * Schreibt ein Byte in den Ausgangs-Datenstrom.
   * Die Default-Implementation hier verschluckt einfach das übergebene Byte.
   *
   * \return 1, wenn das Byte platziert werden konnte, sonst 0.
   */
  virtual unsigned write(unsigned char b);

  /**
   * Schreibt einen Byte-Puffer in den Ausgangs-Datenstrom.
   * Die Default-Implementation ruft write() für jedes Byte des Blocks auf.
   *
   * \return die Anzahl an tatsächlich platzierten Bytes.
   */
  virtual unsigned writeBlock(const unsigned char *b, unsigned int numBytes);

  /**
   * Liest ein Byte aus dem Eingangs-Datenstrom, wenn vorhanden.
   * Die Default-Implementation hier liefert nie Daten.
   *
   * \return 1, wenn ein Byte gelesen werden konnte, sonst 0.
   */
  virtual unsigned read(unsigned char &b);

  /**
   * Liest Daten in einen Byte-Puffer aus dem Eingangs-Datenstrom.
   * Die Default-Implementation ruft read() auf, bis kein Byte gelesen werden konnte oder der Block voll ist.
   *
   * \return die Anzahl an tatsächlich gelesenen Bytes.
   */
  virtual unsigned readBlock(unsigned char *b, unsigned int numBytes);

protected:
  ByteStream& backendIo;
};

#endif /* BYTESTREAMDECORATOR_H_ */
