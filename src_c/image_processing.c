/****************************************************************************
Routine:			image_processing.c

Author/Copyright:	Hans-Gerd Maas

Address:			Institute of Geodesy and Photogrammetry
                                        ETH - Hoenggerberg
                                        CH - 8093 Zurich

Creation Date:		1988

Description:		different image processing routines ...

Routines contained: filter_3:	3*3 filter, reads matrix from filter.par
                                        lowpass_3:	3*3 local average with
9 pointers, fast lowpass_n:	n*n local average, fast computation time
                                                                independent from
n histogram:	computes histogram enhance:	enhances gray value spectrum to
0..255, some extreme gray values are cut off mark_img:	reads image and pixel
coordinate set, marks the image in a certain (monocomp) color and writes image
****************************************************************************/
#include "ptv.h"
#include <string.h>

void filter_3(unsigned char *img, unsigned char *img_lp) {
  register unsigned char *ptr, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5, *ptr6, *ptr7,
      *ptr8, *ptr9;
  int end;
  float m[9], sum, val;
  short buf;
  FILE *fp;
  register int i;

  /* read filter elements from parameter file */
  fp = fopen_rp("filter.par"); // replaced fopen_r, ad holten 12-2012
  sum = 0;
  if (fp) {
    for (i = 0, sum = 0; i < 9; i++) {
      fscanf(fp, "%f", &m[i]);
      sum += m[i];
    }
    fclose(fp);
  }
  if (sum == 0)
    exit(1);

  end = imgsize - 513;

  ptr = img_lp + 513;
  ptr1 = img;
  ptr2 = img + 1;
  ptr3 = img + 2;
  ptr4 = img + imx;
  ptr5 = img + imx + 1;
  ptr6 = img + imx + 2;
  ptr7 = img + 2 * imx;
  ptr8 = img + 2 * imx + 1;
  ptr9 = img + 2 * imx + 2;

  for (i = 513; i < end; i++) {
    val = m[0] * *ptr1++ + m[1] * *ptr2++ + m[2] * *ptr3++ + m[3] * *ptr4++ +
          m[4] * *ptr5++ + m[5] * *ptr6++ + m[6] * *ptr7++ + m[7] * *ptr8++ +
          m[8] * *ptr9++;
    buf = (short)(val / sum);
    if (buf > 255)
      buf = 255;
    if (buf < 8)
      buf = 8;
    *ptr++ = (unsigned char)buf;
  }
}

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void enhance(unsigned char *img) {
  register unsigned char *ptr;
  unsigned char *end, gmin = 255, gmax = 0, offs;
  float diff, gain;
  int i, sum, histo[256];

  void histogram(unsigned char *img, int *hist);

  end = img + imgsize;

  histogram(img, histo);
  for (i = 0, sum = 0; (i < 255) && (sum < imx); sum += histo[i], i++)
    ;
  gmin = i;
  for (i = 255, sum = 0; (i > 0) && (sum < 512); sum += histo[i], i--)
    ;
  gmax = i;
  offs = gmin;
  diff = (float)(gmax - gmin);
  gain = (float)255 / diff;

  for (ptr = img; ptr < end; ptr++) {
    if (*ptr < gmin)
      *ptr = gmin;
    else if (*ptr > gmax)
      *ptr = gmax;
    *ptr = (unsigned char)((*ptr - offs) * gain);
    if (*ptr < 8)
      *ptr = 8; /* due monocomp colors */
  }
}
#endif

void histogram(unsigned char *img, int *hist) {
  int i;
  unsigned char *end;
  register unsigned char *ptr;

  for (i = 0; i < 256; i++)
    hist[i] = 0;

  end = img + imgsize;
  for (ptr = img; ptr < end; ptr++)
    hist[*ptr]++;
}

void lowpass_3(unsigned char *img, unsigned char *img_lp) {
  register unsigned char *ptr, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5, *ptr6, *ptr7,
      *ptr8, *ptr9;
  short buf;
  register int i;

  ptr = img_lp + 513;
  ptr1 = img;
  ptr2 = img + 1;
  ptr3 = img + 2;
  ptr4 = img + imx;
  ptr5 = img + imx + 1;
  ptr6 = img + imx + 2;
  ptr7 = img + 2 * imx;
  ptr8 = img + 2 * imx + 1;
  ptr9 = img + 2 * imx + 2;

  for (i = 0; i < imgsize; i++) {
    buf = *ptr5++ + *ptr1++ + *ptr2++ + *ptr3++ + *ptr4++ + *ptr6++ + *ptr7++ +
          *ptr8++ + *ptr9++;
    *ptr++ = buf / 9;
  }
}

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void lowpass_n(int n, unsigned char *img, unsigned char *img_lp) {
  register unsigned char *ptrl, *ptrr, *ptrz;
  short *buf1, *buf2, buf, *end;
  register short *ptr, *ptr1, *ptr2, *ptr3;
  int k, n2, nq;
  register int i;

  n2 = 2 * n + 1;
  nq = n2 * n2;

  buf1 = (short *)calloc(imgsize, sizeof(short));
  if (!buf1) {
    puts("calloc for buf1 --> error");
    exit(1);
  }
  buf2 = (short *)calloc(imx, sizeof(short));

  /* --------------  average over lines  --------------- */
  end = buf1 + imgsize;
  buf = 0;
  for (ptrr = img; ptrr < img + n2; ptrr++)
    buf += *ptrr;
  *(buf1 + n) = buf;

  for (ptrl = img, ptr = buf1 + n + 1; ptr < end; ptrl++, ptr++, ptrr++) {
    buf += (*ptrr - *ptrl);
    *ptr = buf;
  }

  /* -------------  average over columns	-------------- */
  end = buf2 + imx;
  for (ptr1 = buf1, ptrz = img_lp + imx * n, ptr3 = buf2; ptr3 < end;
       ptr1++, ptrz++, ptr3++) {
    for (k = 0, ptr2 = ptr1; k < n2; k++, ptr2 += imx)
      *ptr3 += *ptr2;
    *ptrz = *ptr3 / nq;
  }
  for (i = n + 1, ptr1 = buf1, ptrz = img_lp + imx * (n + 1),
      ptr2 = buf1 + imx * n2;
       i < imy - n; i++) {
    for (ptr3 = buf2; ptr3 < end; ptr3++, ptr1++, ptrz++, ptr2++) {
      *ptr3 += (*ptr2 - *ptr1);
      *ptrz = *ptr3 / nq;
    }
  }
  free(buf1);
  free(buf2); // added, ad holten, 12-2012
}
#endif

void unsharp_mask(int n, unsigned char *img0, unsigned char *img_lp) {
  register unsigned char *imgum, *ptrl, *ptrr, *ptrz;
  int *buf1, *buf2, buf, *end;
  register int *ptr, *ptr1, *ptr2, *ptr3;
  int ii, n2, nq, m;
  register int i;

  n2 = 2 * n + 1;
  nq = n2 * n2;

  imgum = (unsigned char *)calloc(imgsize, 1);
  if (!imgum) {
    puts("calloc for imgum --> error");
    exit(1);
  }

  buf1 = (int *)calloc(imgsize, sizeof(int));
  if (!buf1) {
    puts("calloc for buf1 --> error");
    exit(1);
  }
  buf2 = (int *)calloc(imx, sizeof(int));

  // for (ptrl=imgum, ptrr=img0; ptrl<(imgum+imgsize); ptrl++, ptrr++)
  //  *ptrl = *ptrr;
  copy_images(img0, imgum, imgsize); // ad holten, 12-2012

  /* cut off high gray values (not in general use !)
  for (ptrz=imgum; ptrz<(imgum+imgsize); ptrz++) if (*ptrz > 160) *ptrz = 160;
*/

  /* --------------  average over lines  --------------- */

  for (i = 0; i < imy; i++) {
    ii = i * imx;
    /* first element */
    buf = *(imgum + ii);
    *(buf1 + ii) = buf * n2;

    /* elements 1 ... n */
    for (ptr = buf1 + ii + 1, ptrr = imgum + ii + 2, ptrl = ptrr - 1, m = 3;
         ptr < buf1 + ii + n + 1; ptr++, ptrl += 2, ptrr += 2, m += 2) {
      buf += (*ptrl + *ptrr);
      *ptr = buf * n2 / m;
    }

    /* elements n+1 ... imx-n */
    for (ptrl = imgum + ii, ptr = buf1 + ii + n + 1, ptrr = imgum + ii + n2;
         ptrr < imgum + ii + imx; ptrl++, ptr++, ptrr++) {
      buf += (*ptrr - *ptrl);
      *ptr = buf;
    }

    /* elements imx-n ... imx */
    for (ptrl = imgum + ii + imx - n2, ptrr = ptrl + 1,
        ptr = buf1 + ii + imx - n, m = n2 - 2;
         ptr < buf1 + ii + imx; ptrl += 2, ptrr += 2, ptr++, m -= 2) {
      buf -= (*ptrl + *ptrr);
      *ptr = buf * n2 / m;
    }
  }
  free(imgum);

  /* -------------  average over columns	-------------- */

  end = buf2 + imx;

  /* first line */
  for (ptr1 = buf1, ptr2 = buf2, ptrz = img_lp; ptr2 < end;
       ptr1++, ptr2++, ptrz++) {
    *ptr2 = *ptr1;
    *ptrz = *ptr2 / n2;
  }

  /* lines 1 ... n */
  for (i = 1; i < n + 1; i++) {
    ptr1 = buf1 + (2 * i - 1) * imx;
    ptr2 = ptr1 + imx;
    ptrz = img_lp + i * imx;
    for (ptr3 = buf2; ptr3 < end; ptr1++, ptr2++, ptr3++, ptrz++) {
      *ptr3 += (*ptr1 + *ptr2);
      *ptrz = n2 * (*ptr3) / nq / (2 * i + 1);
    }
  }

  /* lines n+1 ... imy-n-1 */
  for (i = n + 1, ptr1 = buf1, ptrz = img_lp + imx * (n + 1),
      ptr2 = buf1 + imx * n2;
       i < imy - n; i++) {
    for (ptr3 = buf2; ptr3 < end; ptr3++, ptr1++, ptrz++, ptr2++) {
      *ptr3 += (*ptr2 - *ptr1);
      *ptrz = *ptr3 / nq;
    }
  }

  /* lines imy-n ... imy */
  for (i = n; i > 0; i--) {
    ptr1 = buf1 + (imy - 2 * i - 1) * imx;
    ptr2 = ptr1 + imx;
    ptrz = img_lp + (imy - i) * imx;
    for (ptr3 = buf2; ptr3 < end; ptr1++, ptr2++, ptr3++, ptrz++) {
      *ptr3 -= (*ptr1 + *ptr2);
      *ptrz = n2 * (*ptr3) / nq / (2 * i + 1);
    }
  }
  free(buf1);
  free(buf2); // added, ad holten, 12-2012
}

void zoom(unsigned char *img, unsigned char *zoomimg, int xm, int ym, int zf)
/* zoom at col xm, line ym, with factor zf */
{
  int i0, j0, sx, sy, i1, i2, j1, j2;
  register int i, j, k, l;

  sx = imx / zf;
  sy = imy / zf;
  i0 = ym - sy / 2;
  j0 = xm - sx / 2;

  /* lines = i, cols = j */
  for (i = 0; i < sy; i++)
    for (j = 0; j < sx; j++) {
      i1 = i0 + i;
      j1 = j0 + j;
      i2 = zf * i;
      j2 = zf * j;
      for (k = 0; k < zf; k++)
        for (l = 0; l < zf; l++) {
          *(zoomimg + imx * (i2 + k) + j2 + l) = *(img + imx * i1 + j1);
        }
    }
}

void zoom_new(unsigned char *img, unsigned char *zoomimg, int xm, int ym,
              int zf, int zimx, int zimy)
// zoom at col xm, line ym, factor zf
// zoom image size: zimx, zimy
{
  int xa, ya;
  register int i;
  unsigned char *end;
  register unsigned char *ptri, *ptrz;

  xa = xm - zimx / (2 * zf);
  ya = ym - zimy / (2 * zf);
  ptri = img + ya * imx + xa;
  end = zoomimg + zimx * zimy;

  for (ptrz = zoomimg, i = 0; ptrz < end; ptrz++) {
    *ptrz = *ptri;
    i++;
    if ((i % zimx) == 0)
      ptri = img + (ya + (i / (zimx * zf))) * imx + xa;
    if ((i % zf) == 0)
      ptri++;
  }
}

void split(unsigned char *img, int field) {
  register int i, j;
  register unsigned char *ptr;
  unsigned char *end;

  switch (field) {
  case 0: /* frames */
    return;
    break;

  case 1: /* odd lines */
    for (i = 0; i < imy / 2; i++)
      for (j = 0; j < imx; j++)
        *(img + imx * i + j) = *(img + 2 * imx * i + j + imx);
    break;

  case 2: /* even lines */
    for (i = 0; i < imy / 2; i++)
      for (j = 0; j < imx; j++)
        *(img + imx * i + j) = *(img + 2 * imx * i + j);
    break;
  }
  end = img + imgsize;
  for (ptr = img + imgsize / 2; ptr < end; ptr++)
    *ptr = 2;
}

void copy_images(unsigned char *img1, unsigned char *img2, size_t imgsize) {
  // ad holten, 12-2012  replaced with memcpy()
  // register unsigned char *ptr1, *ptr2;
  // unsigned char           *end;

  // for (end=img1+imgsize, ptr1=img1, ptr2=img2; ptr1<end; ptr1++, ptr2++)
  // *ptr2 = *ptr1;
  memcpy(img2, img1, imgsize);
}

/*------------------------------------------------------------------------
        Subtract mask, Matthias Oswald, Juli 08
  ------------------------------------------------------------------------*/
void subtract_mask(unsigned char *img, unsigned char *img_mask,
                   unsigned char *img_new) {
  register unsigned char *ptr1, *ptr2, *ptr3;
  int i;

  for (i = 0, ptr1 = img, ptr2 = img_mask, ptr3 = img_new; i < imgsize;
       ptr1++, ptr2++, ptr3++, i++) {
    if (*ptr2 == 0)
      *ptr3 = 0;
    else
      *ptr3 = *ptr1;
  }
}