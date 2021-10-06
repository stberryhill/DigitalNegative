#define DIGITAL_NEGATIVE_IMPLEMENTATION
#ifndef DIGITAL_NEGATIVE_H
#define DIGITAL_NEGATIVE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define TIFF_MAGIC_NUMBER 42

typedef struct {
} DigitalNegative;

void DigitalNegative_Decode(const char *filePath, DigitalNegative *output);
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
#define log(printfArgs) { printf(printfArgs); printf("\n"); }

bool stringEndsWith(const char *string, const char *suffix) {
  const int stringLength = strlen(string);
  const int suffixLength = strlen(suffix);

  if (suffixLength > stringLength) return false;

  int i;
  int suffixI = 0;
  for (i = stringLength - suffixLength; i < stringLength; i++) {
    if (string[i] != suffix[suffixI]) return false;
    suffixI++;
  }

  return true;
}

void readTiffHeader(FileReader *reader, TiffHeader *output) {
  char bytes[256];
  FileReader_ReadNBytes(reader, 2, bytes);
  bool isLittleEndian = strcmp(bytes, BYTES01_LITTLE_ENDIAN) == 0;
  if (!isLittleEndian) {
    reader->byteOrder = FILE_BYTE_ORDER_BIG_ENDIAN;
    crashAndLogIf(strcmp(bytes, BYTES01_BIG_ENDIAN) != 0, "File is neither big endian nor little endian.");
  }

  printf("File is %s endian.\n", isLittleEndian ? "little" : "big");

  int magicNumber;
  FileReader_ReadIntegerOfNBytes(reader, 2, &magicNumber);
  crashAndLogIf(magicNumber != TIFF_MAGIC_NUMBER, "Missing magic number identifying file as Tiff.");

  printf("Found magic number %d.\n", TIFF_MAGIC_NUMBER);

  int firstIFDOffset;
  FileReader_ReadIntegerOfNBytes(reader, 4, &firstIFDOffset);
  bool firstIFDOffsetIsOnWordBoundary = firstIFDOffset % 4 == 0;
  crashAndLogIf(!firstIFDOffsetIsOnWordBoundary, "First IFD offset must be on word boundary.");

  printf("First IFD offset is %d.\n", firstIFDOffset);
}

void readIfd(FileReader *reader, Ifd *output) {
  // TODO: implement
}

void DigitalNegative_Decode(const char *filePath, DigitalNegative *output) {
  const int characterCount = strlen(filePath);
  char *uppercasePath = malloc(characterCount + 1);
  int i;
  for (i = 0; i < characterCount; i++) {
    uppercasePath[i] = toupper(filePath[i]);
  }

  crashAndLogIf(!(stringEndsWith(uppercasePath, ".DNG") || stringEndsWith(uppercasePath, ".TIF")), "Can't open digital negative file - wrong file extension. (Must be .DNG or .TIF)");
  
  FileReader *reader = FileReader_Open(filePath);

  printf("Reading TIFF Header...\n");
  TiffHeader header;
  readTiffHeader(reader, &header);
  printf("...DONE.\n\n");

  printf("Reading IFDs...\n");
  Ifd ifd;
  readIfd(reader, &ifd);
  printf("...DONE.\n\n");

  FileReader_Close(reader);
}

void DigitalNegative_Free(DigitalNegative *digitalNegative) {
  free (digitalNegative);
}

#endif

#endif