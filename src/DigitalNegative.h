#define DIGITAL_NEGATIVE_IMPLEMENTATION
#ifndef DIGITAL_NEGATIVE_H
#define DIGITAL_NEGATIVE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
} DigitalNegative;

DigitalNegative DigtalNegative_Decode(const char *filePath);
void DigitalNegative_Free(DigitalNegative *digitalNegative);

#ifdef DIGITAL_NEGATIVE_IMPLEMENTATION
#define DIGITAL_NEGATIVE_FILE_IMPLEMENTATION
#include "DigitalNegativeFile.h"
#include <string.h>

#define BYTES01_LITTLE_ENDIAN "II"
#define BYTES01_BIG_ENDIAN "MM"
const static char BYTES23_TIFF[2] = {4, 2};

typedef struct {
  bool isLittleEndian;
  int firstIfdOffset;
} TiffHeader;

typedef struct {
  int offset;
} Ifd;

#define crashAndLogIf(condition, message) {if (condition) { printf("%s\n", message); exit(EXIT_FAILURE); }}

bool stringEndsWith(const char *string, const char *suffix) {
  const int stringLength = strlen(string);
  const int suffixLength = strlen(suffix);

  if (suffixLength > stringLength) return false;

  int i;
  for (i = stringLength - suffixLength; i < stringLength; i++) {
    if (string[i] != suffix[i]) return false;
  }

  return true;
}

void readTiffHeader(FileReader *reader, TiffHeader *output) {
  char *bytes;
  bytes = FileReader_ReadNBytes(reader, 2);
  bool isLittleEndian = strcmp(bytes, BYTES01_LITTLE_ENDIAN) == 0;
  if (!isLittleEndian) {
    reader->byteOrder = FILE_BYTE_ORDER_BIG_ENDIAN;
    crashAndLogIf(strcmp(bytes, BYTES01_BIG_ENDIAN) != 0, "File is neither big endian nor little endian.");
  }

  bytes = FileReader_ReadNBytes(reader, 2);
  bool isTiffCompatible = bytes[0] == BYTES23_TIFF[0] && bytes[1] == BYTES23_TIFF[1];
  crashAndLogIf(!isTiffCompatible, "Missing magic number identifying file as Tiff (42).");

  int firstIFDOffset = FileReader_ReadIntegerOfNBytes(reader, 4);
  bool firstIFDOffsetIsOnWordBoundary = firstIFDOffset % 4 == 0;
  crashAndLogIf(!firstIFDOffsetIsOnWordBoundary, "First IFD offset must be on word boundary.");
}

DigitalNegative DigtalNegative_Decode(const char *filePath) {
  crashAndLogIf(!(stringEndsWith(filePath, ".DNG") || stringEndsWith(filePath, ".TIF")), "Can't open digital negative file - wrong file extension. (Must be .DNG or .TIF)");
  
  FileReader *reader = FileReader_Open(filePath);

  TiffHeader header;
  readTiffHeader(reader, &header);

  Ifd ifd;
  readIfd(reader, &ifd);

  FileReader_Close(reader);
}

void DigitalNegative_Free(DigitalNegative *digitalNegative) {
  free (digitalNegative);
}

#endif

#endif