/*
 *
 * types.c
 *
 */

#include <stdio.h>

#if defined(__AROS__)
#include "aros-inc.h"
#include <aros/macros.h>
#endif

#include "main.h"
#include "types.h"

#define ID_MPEG_1B3 0x000001B3
#define ID_MPEG_1BA 0x000001BA

#define ID_FORM  0x464F524D
#define ID_LIST  0x4C495354
#define ID_PROP  0x50524F50

#define ID_RIFF  0x52494646
#define ID_AVI   0x41564920

#define ID_FLI   0x000011AF
#define ID_FLC   0x000012AF

#define ID_QT_MOOV 0x6D6F6F76
#define ID_QT_MDAT 0x6D646174
#define ID_QT_SKIP 0x736B6970
#define ID_QT_FREE 0x66726565
#define ID_QT_FTYP 0x66747970
#define ID_QT_WIDE 0x77696465

#define ID_NSF1 0x4E45534d
#define ID_NSF2 0x1A000000

#define FOURCC(a,b,c,d) ((a << 24) | (b << 16) | (c << 8) | (d))

#define ID_RM FOURCC('.', 'R', 'M', 'F')

static unsigned long get_be32(const unsigned char *p)
{
  return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) |
         ((unsigned long)p[2] << 8)  |  (unsigned long)p[3];
}

int get_type(char *filename)
{
  unsigned long data[4], d0 = 0, d2 = 0;
  unsigned char buf[16];
  int type = UNKNOWN;
  FILE *file;

  file = fopen(filename, "rb");

  if (file != NULL)
  {
    if (fread(buf, 1, 16, file) != 16) {
      fclose(file);
      return UNKNOWN;
    }

    /* Skip an ID3v2 tag so tagged MP3 files are still recognised. */
    if ((buf[0] == 'I') && (buf[1] == 'D') && (buf[2] == '3')) {
      unsigned long tag_size =
        ((unsigned long)(buf[6] & 0x7f) << 21) |
        ((unsigned long)(buf[7] & 0x7f) << 14) |
        ((unsigned long)(buf[8] & 0x7f) << 7)  |
         (unsigned long)(buf[9] & 0x7f);

      if (fseek(file, (long)(10 + tag_size), SEEK_SET) == 0)
        fread(buf, 1, 16, file);
    }

    fclose(file);

    /* The header is big-endian on disk regardless of host word size. */
    data[0] = get_be32(buf + 0);
    data[1] = get_be32(buf + 4);
    data[2] = get_be32(buf + 8);
    data[3] = get_be32(buf + 12);

    debug_printf("HEADER: %08lx, %08lx, %08lx, %08lx\n", data[0], data[1], data[2], data[3]);

    switch(data[0])
    {
      case ID_RM:
        type = RM_SYSTEM;
        break;

      case ID_FORM:
      case ID_LIST:
      case ID_PROP:
        type = IFF_ANIM;
        break;

      case ID_RIFF:
        if (data[2] == ID_AVI)
          type = AVI_ANIM;
        break;

      default:
        d0 = (data[0] >> 16) & 0xFFFF;
        d2 = (data[1] >> 16) & 0xFFFF;

        if ((d2 == ID_FLI) || (d2 == ID_FLC))
          type = FLIC_ANIM;

        {
          /* QuickTime/MP4 atoms can start at offset 4, 8 or 12. */
          int k;
          for (k = 1; k <= 3; k++) {
            unsigned long w = data[k];
            if ((w == ID_QT_MOOV) || (w == ID_QT_MDAT) ||
                (w == ID_QT_SKIP) || (w == ID_QT_FREE) ||
                (w == ID_QT_FTYP) || (w == ID_QT_WIDE)) {
              type = QT_ANIM;
              break;
            }
          }
        }

        break;
    }

    if ((data[0] == ID_NSF1) && ((data[1] & 0xff000000) == ID_NSF2)) {
      type = NSF_AUDIO;
    }

    if ((type == UNKNOWN) && ((data[0] >> 16) == 0x0b77)) {
      type = AC3_AUDIO;
    }

    /* Check if MPEG audio file */
    if (type == UNKNOWN) {
      if((data[0]&0xffe00000) == 0xffe00000) {
        if((data[0]>>17)&3) {
          if(((data[0]>>12)&0xf) != 0xf) {
            if(((data[0]>>10)&0x3) != 0x3) {
              type = MPEG_AUDIO;
            }
          }
        }
      }
    }

    /* Maybe it's an MPEG ? */
    if (type == UNKNOWN) {
      unsigned int startCode;
      file = fopen(filename, "rb");
      fread(&startCode, 1, 4, file);

#if defined(__AROS__)
	startCode = AROS_BE2LONG(startCode);
#endif

	    for (;;) {
        unsigned char byte;

        /* MPEG system/video stream */
        if ((startCode & 0xffffff00) == 0x00000100) {
          if ((startCode&0xff) == 0xb3) {
            type = MPEG_VIDEO;
            break;
          } else if ((startCode&0xff) == 0xba) {
            type = MPEG_SYSTEM;
            break;
          } else {
            type = UNKNOWN;
            break;
          }
        }

        if (fread(&byte, 1, 1, file) != 1) {
          break;
        }

        startCode = (startCode<<8)|byte;
      }

      fclose(file);
    }
  }
  else
  {
    amp_printf("File \"%s\" not found!\n", filename);
    return FAIL;
  }

  return type;
}
