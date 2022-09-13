/*
 * License: Zlib
 * Copyright (c) Sean Tyler Berryhill (sean.tyler.berryhill@gmail.com)
 */

#define DIGITAL_NEGATIVE_IMPLEMENTATION
#ifndef DIGITAL_NEGATIVE_H
#define DIGITAL_NEGATIVE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

typedef struct {
  uint16_t photometricInterpretation;
} DigitalNegative;

void DigitalNegative_Decode(const char *filePath, DigitalNegative *output);
void DigitalNegative_Free(DigitalNegative *digitalNegative);

#ifdef DIGITAL_NEGATIVE_IMPLEMENTATION
#define DIGITAL_NEGATIVE_FILE_IMPLEMENTATION
#include "DigitalNegativeFile.h"
#include <string.h>

#define BYTES01_LITTLE_ENDIAN "II"
#define BYTES01_BIG_ENDIAN "MM"
#define TIFF_MAGIC_NUMBER 42
#define TIFF_WORD_SIZE 2

#define endsOnTiffWordBoundary(x) x % TIFF_WORD_SIZE == 0

typedef struct {
  bool isLittleEndian;
  int firstIfdOffset;
} TiffHeader;

typedef enum {
  FIELD_TYPE_BYTE,       /* 8-bit unsigned integer.*/
  FIELD_TYPE_ASCII,      /* 8-bit byte that contains a 7-bit ASCII code; the last byte must be NUL (binary zero). */
  FIELD_TYPE_SHORT,      /* 16-bit (2-byte) unsigned integer. */
  FIELD_TYPE_LONG,       /* 32-bit (4-byte) unsigned integer. */
  FIELD_TYPE_RATIONAL,   /* Two LONGs (4 bytes each):  the first represents the numerator of a fraction; the second, the denominator. */
  FIELD_TYPE_SBYTE,      /* An 8-bit signed (twos-complement) integer. */
  FIELD_TYPE_UNDEFINED,  /* An 8-bit byte that may contain anything, depending on the definition of the field. */
  FIELD_TYPE_SSHORT,     /* A 16-bit (2-byte) signed (twos-complement) integer. */
  FIELD_TYPE_SLONG,      /* A 32-bit (4-byte) signed (twos-complement) integer. */
  FIELD_TYPE_SRATIONAL,  /* Two SLONG’s:  the first represents the numerator of a fraction, the second the denominator. */
  FIELD_TYPE_FLOAT,      /* Single precision (4-byte) IEEE format. */
  FIELD_TYPE_DOUBLE,     /* Double precision (8-byte) IEEE format. */
  FIELD_TYPE_COUNT
} FieldType;

static const int fieldTypeSizeLookupArray[FIELD_TYPE_COUNT] = {1, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8};

typedef union {
  char bytes[8];
  char byteValue;
  char asciiValue;
  short shortValue;
  int longValue;
  long rationalValue;
  short sshortValue;
  int slongValue;
  long srationalValue;
  float floatValue;
  double doubleValue;
} FieldValue;

/* An IFD entry is synonymous with a field */
typedef struct {
  uint32_t tag;
  FieldType fieldType;
  int valueCount; /* Fields have a value count, they are 1D arrays. But most fields contain only a single value. */
  int valueOffset;
  FieldValue *values; /* TODO: Design map on paper. This will be in the map. values only loaded when requested */
} IfdEntry;

typedef struct {
  uint32_t offset;
  uint32_t numberOfEntries;
  IfdEntry *entries;
  uint32_t nextIfdOffset;
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
  output->isLittleEndian = strcmp(bytes, BYTES01_LITTLE_ENDIAN) == 0;
  if (!output->isLittleEndian) {
    reader->byteOrder = FILE_BYTE_ORDER_BIG_ENDIAN;
    crashAndLogIf(strcmp(bytes, BYTES01_BIG_ENDIAN) != 0, "File is neither big endian nor little endian.");
  }

  printf("File is %s endian.\n", output->isLittleEndian ? "little" : "big");

  uint32_t magicNumber;
  FileReader_ReadUnsignedIntegerOfNBytes(reader, 2, &magicNumber);
  crashAndLogIf(magicNumber != TIFF_MAGIC_NUMBER, "Missing magic number identifying file as Tiff.");

  printf("Found magic number %u.\n", TIFF_MAGIC_NUMBER);

  FileReader_ReadUnsignedIntegerOfNBytes(reader, 4, &output->firstIfdOffset);
  bool firstIFDOffsetIsOnWordBoundary = endsOnTiffWordBoundary(output->firstIfdOffset);
  crashAndLogIf(!firstIFDOffsetIsOnWordBoundary, "First IFD offset must be on word boundary.");

  printf("First IFD offset is %u.\n", output->firstIfdOffset);
}

void readIfdEntry(FileReader *reader, IfdEntry *output) {
  FileReader_ReadUnsignedIntegerOfNBytes(reader, 2, &output->tag);

  printf("Entry has tag %u.\n", output->tag);

  /* Warning: It is possible that other TIFF field types will be added in the future.
   * Readers should skip over fields containing an unexpected field type. */
  uint32_t fieldTypeCode;
  FileReader_ReadUnsignedIntegerOfNBytes(reader, 2, &fieldTypeCode);
  crashAndLogIf(fieldTypeCode < 0, "Field type code must be > 0.");

  printf("Field type code = %u.\n", fieldTypeCode);

  const bool isUnknownFieldType = fieldTypeCode > FIELD_TYPE_COUNT || fieldTypeCode < 1;
  if (isUnknownFieldType) {
    const int bytesLeftInEntry = 8; // Each ifd entry is 12 bytes
    FileReader_SetReadPosition(reader, FileReader_GetReadPosition(reader) + (long) bytesLeftInEntry); // Skip unknown field type.
  } else {
    output->fieldType = fieldTypeCode - 1;
    const int fieldTypeSize = fieldTypeSizeLookupArray[output->fieldType];

    FileReader_ReadUnsignedIntegerOfNBytes(reader, 4, &output->valueCount);
    crashAndLogIf(output->valueCount <= 0, "Value count of IFD entry must be > 0.");

    printf("Value count = %u.\n", output->valueCount);

    /* To save time and space the Value Offset contains the Value instead of pointing to
    * the Value if and only if the Value fits into 4 bytes. If the Value is shorter than 4
    * bytes, it is left-justified within the 4-byte Value Offset, i.e., stored in the lower-
    * numbered bytes. Whether the Value fits within 4 bytes is determined by the TypeValue count
    * and Count of the field */
    FileReader_ReadUnsignedIntegerOfNBytes(reader, 4, &output->valueOffset);
    printf("Value offset = %u.\n", output->valueOffset);

    bool valueIsStoredInValueOffset = (output->valueCount * fieldTypeSize) <= 4;
    
    if (valueIsStoredInValueOffset) {

    } else {
      bool valueOffsetIsOnWordBoundary = endsOnTiffWordBoundary(output->valueOffset);
      crashAndLogIf(!valueOffsetIsOnWordBoundary, "Value offset must be on word boundary.");
    }
  }
}

void readIfd(FileReader *reader, const int ifdOffset, Ifd *output) {
  output->offset = ifdOffset;
  FileReader_SetReadPosition(reader, ifdOffset);

  FileReader_ReadUnsignedIntegerOfNBytes(reader, 2, &output->numberOfEntries);
  crashAndLogIf(output->numberOfEntries <= 0, "IFD has invalid number of entries (must be > 0).");

  printf("Number of entires = %u.\n", output->numberOfEntries);

  output->entries = malloc(output->numberOfEntries * (sizeof *output->entries));

  int i;
  for (i = 0; i < output->numberOfEntries; i++) {
    printf("Reading IFD Entry...\n");
    readIfdEntry(reader, &output->entries[i]); // Must be in ascending order by tag.
    printf("...DONE\n\n");
  }

  FileReader_ReadUnsignedIntegerOfNBytes(reader, 4, &output->nextIfdOffset);

  printf("Next IFD offset = %u.\n", output->nextIfdOffset);
}

void DigitalNegative_Decode(const char *filePath, DigitalNegative *output) {
  const int characterCount = strlen(filePath);
  char *uppercasePath = malloc(characterCount + 1);
  int i;
  for (i = 0; i < characterCount; i++) {
    uppercasePath[i] = toupper(filePath[i]);
  }

  crashAndLogIf(!(stringEndsWith(uppercasePath, ".DNG") || stringEndsWith(uppercasePath, ".TIF")), "Can't open digital negative file - wrong file extension. (Must be .DNG or .TIF)");
  free(uppercasePath);
  
  FileReader *reader = FileReader_Open(filePath);

  printf("Reading TIFF Header...\n");
  TiffHeader header;
  readTiffHeader(reader, &header);
  printf("...DONE.\n\n");

  printf("Reading IFDs...\n");
  bool moreIfds = true;
  uint32_t offset = header.firstIfdOffset;
  while (moreIfds) {
    Ifd ifd;
    readIfd(reader, offset, &ifd);

    offset = ifd.nextIfdOffset;
    moreIfds = offset != 0;
  }

  printf("...DONE.\n\n");

  FileReader_Close(reader);
}

void DigitalNegative_Free(DigitalNegative *digitalNegative) {
  free(digitalNegative);
}

#endif

#endif