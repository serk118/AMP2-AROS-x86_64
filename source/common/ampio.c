/*
 *
 * ampio.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <exec/memory.h>
#include <proto/exec.h>

#include "ampio.h"
#include "../main/main.h"
#include "../main/prefs.h"

#include <aros/macros.h>

#define IO_BUFFER_SIZE (64*1024)

static unsigned char *ampio_buffer = NULL;
static int ampio_file_position = 0; /* This one must always be correct */
static int ampio_position = 0;
static int ampio_buffersize = 0;
static int ampio_eof = 0;
static int ampio_filesize = 0;
static FILE *ampio_fp = NULL;

int ampio_fread(unsigned char *buffer, int length)
{
  int i;

  for (i=0; i<length; i++) {
    if (ampio_position >= ampio_buffersize) {
      if (ampio_eof == 1) {
        return i;
      }
      ampio_file_position += ampio_position;
      ampio_buffersize = fread(ampio_buffer, 1, IO_BUFFER_SIZE, ampio_fp);
      if (ampio_buffersize != IO_BUFFER_SIZE) {
        ampio_eof = 1;
      }
      ampio_position = 0;
    }
    buffer[i] = ampio_buffer[ampio_position++];
  }

  return length;
}

int ampio_fseek(int length, int whence)
{
  int i;

  /* Be a little intelligent */
  if (whence == SEEK_SET) {
    int cur_pos = ampio_ftell();
    if (length == cur_pos) {
      return 1;
    }
    /* Be a bit more intelligent */
    if ((length > cur_pos) && (length < (cur_pos + IO_BUFFER_SIZE))) {
      length -= cur_pos;
      whence = SEEK_CUR;
    }
  }

  if (whence == SEEK_SET) {
    fseek(ampio_fp, length, SEEK_SET);
    ampio_file_position = length;
    ampio_buffersize = fread(ampio_buffer, 1, IO_BUFFER_SIZE, ampio_fp);
    if (ampio_buffersize != IO_BUFFER_SIZE) {
      ampio_eof = 1;
    } else {
      ampio_eof = 0;
    }
    ampio_position = 0;
    return 1;
  }

  if ((whence != SEEK_CUR) || (length < 0)){
    return 0; /* Not supported yet */
  }

  for (i=0; i<length; i++) {
    if (ampio_position >= ampio_buffersize) {
      if (ampio_eof == 1) {
        return 0;
      }
      ampio_file_position += ampio_position;
      ampio_buffersize = fread(ampio_buffer, 1, IO_BUFFER_SIZE, ampio_fp);
      if (ampio_buffersize != IO_BUFFER_SIZE) {
        ampio_eof = 1;
      }
      ampio_position = 0;
    }
    ampio_position++;
  }

  return 1;
}

int ampio_fskip(int length)
{
  return ampio_fseek(length, SEEK_CUR);
}

int ampio_length(void)
{
  return ampio_filesize;
}

int ampio_ftell(void)
{
  return ampio_file_position + ampio_position;
}

int ampio_feof(void)
{
  if ((ampio_eof == 1) && (ampio_position >= ampio_buffersize)) {
    return 1;
  }

  return 0;
}

int ampio_fclose(void)
{
  if (ampio_fp != NULL) {
    fclose(ampio_fp);
    ampio_fp = NULL;
  }
  if (ampio_buffer != NULL) {
    free(ampio_buffer);
    ampio_buffer = NULL;
  }

  return 0;
}

int ampio_fopen(char *filename)
{
  unsigned long free_mem;
  struct stat statbuf;

  /* Initialize some variables */
  ampio_file_position = 0;
  ampio_position = 0;
  ampio_buffersize = 0;
  ampio_eof = 0;
  ampio_filesize = 0;

  if (stat(filename, &statbuf) != 0) {
    amp_printf("could not open file '%s'\n", filename);
    return 0;
  }

  ampio_filesize = statbuf.st_size;
  ampio_fp = fopen(filename, "rb");

  if (ampio_fp == NULL) {
    amp_printf("could not open file '%s'\n", filename);
    return 0;
  }
  strcat(prefs.window_title, filename);

  if (prefs.readall == PREFS_ON) {
    /* Always leave 8MB free for other tasks */
    #define SAFE (1024*1024*8)

    free_mem = AvailMem(MEMF_FAST|MEMF_LARGEST) - SAFE;
  
    if (free_mem >= ampio_filesize) {
      ampio_buffer = malloc(ampio_filesize);
      if (ampio_buffer != NULL) {
        ampio_buffersize = fread(ampio_buffer, 1, ampio_filesize, ampio_fp);
        ampio_eof = 1; /* Nothing more to read */
      }
    } else {
      amp_printf("READALL failed, not enough free memory\n");
    }
  }

  if (ampio_buffer == NULL) {
    ampio_buffer = malloc(IO_BUFFER_SIZE);
    if (ampio_buffer == NULL) {
      return 0;
    }
  }

  return 1;
}

/* Slow but usefull */

unsigned char get_byte()
{
  unsigned char ret;

  ampio_fread((unsigned char *)&ret, 1);

  return ret;
}

unsigned short get_be16()
{
  unsigned char b[2];

  ampio_fread(b, 2);

  return (unsigned short)(((unsigned short)b[0] << 8) | b[1]);
}

unsigned long get_be32()
{
  unsigned char b[4];

  ampio_fread(b, 4);

  return ((unsigned long)b[0] << 24) | ((unsigned long)b[1] << 16) |
         ((unsigned long)b[2] << 8)  |  (unsigned long)b[3];
}

unsigned short get_le16()
{
  unsigned char b[2];

  ampio_fread(b, 2);

  return (unsigned short)(((unsigned short)b[1] << 8) | b[0]);
}

unsigned long get_le32()
{
  unsigned char b[4];

  ampio_fread(b, 4);

  return ((unsigned long)b[3] << 24) | ((unsigned long)b[2] << 16) |
         ((unsigned long)b[1] << 8)  |  (unsigned long)b[0];
}
