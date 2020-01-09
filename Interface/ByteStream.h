#ifndef BYTESTREAM_H_
#define BYTESTREAM_H_

/**
 * Diese (konkrete) (Interface-)Klasse stellt ein Interface bereit, das die Details von zeichenweisem
 * Datenversand und -Empfang abdeckt.
 *
 * Instanziert man  diese Klasse, erhält man ein sogenanntes Null-Device, das nie Daten liefert
 * (immer leer ist) und alle gesendeten Daten verschluckt.
 *
 * Abgeleitete Klassen, die virtuelle Methoden überladen, können damit andere Funktionen
 * realiseren :-).
 *
 * @note die Nutzer dieses Interfaces gehen davon aus, daß der Stream nicht blockiert. Alle Implementationen
 * an diesem Interface sollen auch nicht blockierend sein. Blockierende Decorator dieses Interfaces müssen einen
 * Timeout-Mechanismus implementieren.
 */
class ByteStream
{

public:
  ByteStream();
  virtual ~ByteStream(void);

  /**
   * Schreibt ein Byte in den Ausgangs-Datenstrom.
   * Die Default-Implementation hier verschluckt einfach das übergebene Byte.
   *
   * \return 1, wenn das Byte platziert werden konnte, sonst 0.
   */
  virtual unsigned write(unsigned char b);
  unsigned write(char b);

  /**
   * Schreibt einen Byte-Puffer in den Ausgangs-Datenstrom.
   * Die Default-Implementation ruft write() für jedes Byte des Blocks auf.
   *
   * \return die Anzahl an tatsächlich platzierten Bytes.
   */
  virtual unsigned writeBlock(const unsigned char *b, unsigned numBytes);
  unsigned writeBlock(const char *b, unsigned numBytes);

  /**
   * Schreibt einen Null-terminierten String in den Ausgangsdatenstrom.
   *
   * @param string
   * @return Die Anzahl der tatsächlich geschriebenen Zeichen.
   */
  unsigned writeBlock(const char *string);

  /**
   * Liest ein Byte aus dem Eingangs-Datenstrom, wenn vorhanden.
   * Die Default-Implementation hier liefert nie Daten.
   *
   * \return 1, wenn ein Byte gelesen werden konnte, sonst 0.
   */
  virtual unsigned read(unsigned char &b);
  unsigned read(char& b);

  /**
   * Liest Daten in einen Byte-Puffer aus dem Eingangs-Datenstrom.
   * Die Default-Implementation ruft read() auf, bis kein Byte mehr gelesen werden konnte oder der Block voll ist.
   *
   * \return die Anzahl an tatsächlich gelesenen Bytes.
   */
  virtual unsigned readBlock(unsigned char *b, unsigned numBytes);
  unsigned readBlock(char* b, unsigned numBytes);

  /**
   * Gibt ein Zeichen aus dem Eingabestrom zurück, oder -1, wenn keine verfügbar sind
   * @return Zeichen, oder -1
   */
  int get_char();

  /**
   * Schreibt ein Zeichen in den Ausgabedatenstrom.
   * @param c ist ein Zeichen, daß in den Ausgabedatenstrom gelegt wird.
   * @return c, wenn erfolgreich, -1, wenn nicht.
   */
  int put_char(int c);

};

inline
unsigned ByteStream::write(char b)
{
  return write((unsigned char)b);
}

inline
unsigned ByteStream::read(char& b)
{
  return read((unsigned char&)b);
}

inline
unsigned ByteStream::writeBlock(const char* b, unsigned numBytes)
{
  return writeBlock((const unsigned char*)b, numBytes);
}

inline
unsigned ByteStream::readBlock(char* b, unsigned numBytes)
{
  return readBlock((unsigned char*)b, numBytes);
}

#endif // BYTESTREAM_H_
