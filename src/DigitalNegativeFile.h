#ifndef DIGITAL_NEGATIVE_FILE_H
#define DIGITAL_NEGATIVE_FILE_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define FILE_EOF EOF
#define MAX_STRING_SIZE 4096

#ifndef FILE_READER_BUFFER_SIZE
#define FILE_READER_BUFFER_SIZE 1024
#endif

typedef struct {
  File *file;
  FileByteOrder byteOrder;
  char buffer[FILE_READER_BUFFER_SIZE];
} FileReader;

typedef struct {
  FILE *descriptor;
  int size;
  bool reachedEndOfFile;
} File;

typedef enum {
  FILE_MODE_READ,
  FILE_MODE_WRITE,
  FILE_MODE_READ_WRITE
} FileMode;

typedef enum {
  FILE_BYTE_ORDER_BIG_ENDIAN,
  FILE_BYTE_ORDER_LITTLE_ENDIAN
} FileByteOrder;

FileReader *FileReader_Open(const char *fileName);
void FileReader_Close(FileReader *fileReader);
char *FileReader_ReadNBytes(FileReader *reader, const int n);
int FileReader_ReadIntegerOfNBytes(FileReader *reader, const int n);

File *File_Open(const char *fileName, FileMode fileMode);
void File_Close(File *file);
void File_ReadLine(File *file, const int characterLimit, char *output);
char File_ReadCharacter(File *file);
char File_PeekCharacter(File *file);
void File_Rewind(File *file, int numCharactersToRewind);
long File_GetReadWritePosition(File *file);
bool File_ReachedEndOfFile(File *file);
void File_ReadEntireContentsToString(File *file, char *output);
int File_GetSize(File *file);

#ifdef DIGITAL_NEGATIVE_FILE_IMPLEMENTATION

FileReader *FileReader_Open(const char *fileName) {
  FileReader *reader = malloc(sizeof *reader);
  reader->file = File_Open("fileName", FILE_MODE_READ);
  reader->byteOrder = FILE_BYTE_ORDER_LITTLE_ENDIAN;

  return reader;
}

void FileReader_Close(FileReader *reader) {
  File_Close(reader->file);
  free(reader);
}

char *FileReader_ReadNBytes(FileReader *reader, const int n) {
  return "";
}
int FileReader_ReadIntegerOfNBytes(FileReader *reader, const int n) {
  return 0;
}

File *File_Open(const char *fileName, FileMode fileMode) {
  File *file = malloc(sizeof *file);
  
  const int MAX_FOPEN_MODE_LENGTH = 3;
  char mode[MAX_FOPEN_MODE_LENGTH + 1]; /* +1 for null terminator */
  switch (fileMode) {
    case FILE_MODE_READ:
      strcpy(mode, "r");
      break;
      
    case FILE_MODE_WRITE:
      strcpy(mode, "w");
      break;
      
    case FILE_MODE_READ_WRITE:
      strcpy(mode, "r+");
      break;
      
    default:
      printf("File error - unsupported file mode passed to qf_openFile.\n");
      exit(-1);
      break;
  }
  
  FILE *fp = fopen(fileName, mode);
  if (!fp) {
    printf("File error - %s could not be opened...\n", fileName);
    exit(-1);
  }
  
  fseek(fp, 0L, SEEK_END);
  const int size = ftell(fp);
  rewind(fp);
  
  file->descriptor = fp;
  file->size = size;
  file->reachedEndOfFile = false;
  
  return file;
}

void File_Close(File *file) {
  fclose(file->descriptor);
  free(file);
}

void File_ReadLine(File *file, const int characterLimit, char *output) {
  if (fgets(output, characterLimit, file->descriptor) == NULL) {
    file->reachedEndOfFile = true;
  }
}

char File_ReadCharacter(File *file) {
  char c;
  if ( (c = fgetc(file->descriptor)) == EOF) {
    file->reachedEndOfFile = true;
  }

  return c;
}

/*
 * Peek at the next character without advancing position in file
 */
char File_PeekCharacter(File *file) {
  char c;
  if ( (c = fgetc(file->descriptor)) != FILE_EOF) {
    fseek(file->descriptor, -1, SEEK_CUR);
  }
  return c;
}

void File_Rewind(File *file, int numCharactersToRewind) {
  fseek(file->descriptor, -numCharactersToRewind, SEEK_CUR);
}

long File_GetReadWritePosition(File *file) {
  return ftell(file->descriptor);
}

bool File_ReachedEndOfFile(File *file) {
  return file->reachedEndOfFile;
}

void File_ReadEntireContentsToString(File *file, char *output) {

  if (file->size > MAX_STRING_SIZE) {
    printf("File error - cannot read file contents to string, exceeds defined maximum string size %d.\n", MAX_STRING_SIZE);
    exit(-1);
  }
  const int bytesPerElement = 1;
  const int numElements = file->size;
  
  rewind(file->descriptor);                                       /* make sure we start at beginning */
  fread(output, bytesPerElement, numElements, file->descriptor);
  
  output[numElements] = '\0';                                     /* add a null terminator */
}

int File_GetSize(File *file) {
  return file->size;
}

#endif

#endif