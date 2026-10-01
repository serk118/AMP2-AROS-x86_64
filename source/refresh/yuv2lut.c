/*
 *
 * yuv2lut.c
 *
 */

#if defined(__AROS__)
#include "aros-inc.h"
#endif

#include "refresh_internal.h"

void yuv2lut(rect_type *rect, unsigned char *src[])
{
#define PIXEL(x) (x)
  unsigned int yrow, uvrow, yadd, uvadd;
  unsigned int *dst, *py, *pu, *pv, y, u, v, val, tu, tv;
  unsigned char *y_table, *cb_table, *cr_table;
  int i, j, src_add, dst_add, cuv;

  cuv = rect->cuv ^ 0x80808080;

  yrow = rect->src_row_length;
  uvrow = rect->src_row_length >> 1;
  yadd = rect->src_offset;
  uvadd = rect->src_uv_offset;

  y_table = rect->y_table;
  cb_table = rect->cb_table;
  cr_table = rect->cr_table;

  src_add = rect->src_row_length - (rect->xloop >> 2);
  dst_add = rect->dst_row_length - (rect->xloop >> 2);

  dst = rect->dst;
  dst += rect->dst_offset;

  for (j=0; j<rect->yloop; j+=2) {
    if (rect->use_osd[j]) {
      py = ((unsigned int *)rect->osd[0]) + yadd;
      pu = ((unsigned int *)rect->osd[1]) + uvadd;
      pv = ((unsigned int *)rect->osd[2]) + uvadd;
    } else {
      py = ((unsigned int *)src[0]) + yadd;
      pu = ((unsigned int *)src[1]) + uvadd;
      pv = ((unsigned int *)src[2]) + uvadd;
    }

    if ((j & 2) == 0) {
      DITHER_YUV_LINE0(rect->xloop)
      pu -= rect->xloop >> 3;
      pv -= rect->xloop >> 3;
      py += src_add;
      dst += dst_add;

      DITHER_YUV_LINE1(rect->xloop)
      dst += dst_add;
    } else {
      DITHER_YUV_LINE2(rect->xloop)
      pu -= rect->xloop >> 3;
      pv -= rect->xloop >> 3;
      py += src_add;
      dst += dst_add;

      DITHER_YUV_LINE3(rect->xloop)
      dst += dst_add;
    }

    yadd += yrow;
    yadd += yrow;
    uvadd += uvrow;
  }
#undef PIXEL
}

void yuv2lut_gray(rect_type *rect, unsigned char *src[])
{
  unsigned int yrow, yadd;
  unsigned int *sptr, *dptr;
  int y;

  yrow = rect->src_row_length;
  yadd = rect->src_offset;

  dptr = rect->dst;
  dptr += rect->dst_offset;

  for (y = 0; y < rect->yloop; y++) {
    if (rect->use_osd[y]) {
      sptr = ((unsigned int *)rect->osd[0]) + yadd;
    } else {
      sptr = ((unsigned int *)src[0]) + yadd;
    }

    memcpy(dptr, sptr, rect->xloop);
    dptr += rect->dst_row_length;

    yadd += yrow;
  }
}
