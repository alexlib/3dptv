#include "ptv.h"

void qs_coord2d_pnr(coord_2d *crd, int left, int right);
void qs_target_y(target *pix, int left, int right);
void qs_con(n_tupel *con, int left, int right);

/* tools / makros */

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
int round(double x)
/* round input double x to integer */
{
  if (x >= 0)
    return ((int)(x + .5));
  else
    return ((int)(x - .5));
}
#endif

void write_ori(Exterior Ex, Interior I, Glass G, char filename[64])
/* write exterior and interior orientation */
{
  FILE *fp;
  int i;

  fp = fopen(filename, "w");
  fprintf(fp, "%11.4f %11.4f %11.4f\n    %10.7f  %10.7f  %10.7f\n\n", Ex.x0,
          Ex.y0, Ex.z0, Ex.omega, Ex.phi, Ex.kappa);
  for (i = 0; i < 3; i++)
    fprintf(fp, "    %10.7f %10.7f %10.7f\n", Ex.dm[i][0], Ex.dm[i][1],
            Ex.dm[i][2]);
  fprintf(fp, "\n    %8.4f %8.4f\n    %8.4f\n", I.xh, I.yh, I.cc);
  fprintf(fp, "\n    %20.15f %20.15f  %20.15f\n", G.vec_x, G.vec_y, G.vec_z);
  fclose(fp);
}

int read_ori(Exterior *Ex, Interior *I, Glass *G, char filename[64])
/* read exterior and interior orientation */
{
  FILE *fp;
  int i;

  fp = fopen_rp(filename); // replaced fopen_r(), ad holten, 12-2012
  if (!fp)
    return 0;

  fscanf(fp, "%lf %lf %lf %lf %lf %lf", &(Ex->x0), &(Ex->y0), &(Ex->z0),
         &(Ex->omega), &(Ex->phi), &(Ex->kappa));
  for (i = 0; i < 3; i++)
    fscanf(fp, " %lf %lf %lf", &(Ex->dm[i][0]), &(Ex->dm[i][1]),
           &(Ex->dm[i][2]));
  fscanf(fp, "%lf %lf %lf", &(I->xh), &(I->yh), &(I->cc));
  fscanf(fp, "%lf %lf %lf", &(G->vec_x), &(G->vec_y), &(G->vec_z));
  fclose(fp);
  return 1;
}

FILE *fopen_rp(char *filename)
/*	tries to open 'filename' for reading;
        prints a message, if the file can't be opened. */
{
  FILE *fp = fopen(filename, "r");
  if (!fp)
    printf("Could not open this file:\n    %s\n", filename);

  return (fp);
}

FILE *fopen_r(char filename[256])
/*	tries to open a file;
        gives a message, if it cannot open it
        and waits until it has been created   */
{
  FILE *fpr;
  int count;

  fpr = fopen(filename, "r");
  if (!fpr) {
    printf("could not open %s, please create this file\n", filename);

    /* wait until file can be opened */
    while (!fpr)
      fpr = fopen(filename, "r");

    /* wait until file really created */
    for (count = 0; count < 100000; count++)
      ;
  }
  return (fpr);
}

int read_image(Tcl_Interp *interp, char path[128], unsigned char *img) {
  int i, j;
  Tk_PhotoHandle img_handle;
  Tk_PhotoImageBlock img_block;

  if (tiff_flag) {
    char val[256];
    snprintf(val, sizeof(val), "temp read %s", path);
    Tcl_Eval(interp, val);

    img_handle = Tk_FindPhoto(interp, "temp");
    Tk_PhotoGetImage(img_handle, &img_block);
    for (j = 0; j < imgsize; j++) {
      i = 4 * j;
      *(img + j) = *(img_block.pixelPtr + i);
    }
  } else {
    fp1 = fopen_rp(path); // replaced fopen_r, ad holten 12-2012
    if (!fp1)
      return 0;
    fread(img, 1, imgsize, fp1);
    fclose(fp1);
    img_handle = Tk_FindPhoto(interp, "temp");
    Tk_PhotoGetImage(img_handle, &img_block);
    tclimg2cimg(interp, img, &img_block);
  }
  return 1;
}

int write_tiff(const char path[256], unsigned char *data, int nx, int ny) {
  TIFF *tif;
  unsigned char *data1;                    /* pixel data */
  unsigned char *buf, *buf1, *begin, *end; /* scanline buffer */
  int y;

  /* open tiff file */
  tif = TIFFOpen(path, "w");

  if (tif == NULL) {
    fprintf(stderr, "Error opening TIFF file: %s\n", path);
    return (-1);
  }

  /* set tiff fields */
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, nx);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, ny);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  /* memory for scanline buffer */
  buf = (unsigned char *)malloc(TIFFScanlineSize(tif));
  if (buf == NULL) {
    fprintf(stderr, "Save TIFF roi: Cannot allocate memory ");
    fprintf(stderr, "for scanline buffer.\n");
    return (-1);
  }

  /* read the required scanlines and copy the required portion */
  data1 = data;
  begin = buf;
  end = begin + nx;

  for (y = 0; y < ny; y++) {
    for (buf1 = begin; buf1 < end; buf1++)
      *buf1 = *data1++;
    TIFFWriteScanline(tif, buf, y, 0);
  }

  /* release scanline buffer */
  free(buf);

  /* flush data and close file */
  TIFFFlushData(tif);
  TIFFClose(tif);
  return (1);
}

/*************************************************************************/

void compose_name_plus_nr(char basename[256], char str[256], int nr,
                          char filename[256]) {

  // replaced code, ad holten 12-2012
  //	char	nr_ch[256];
  //	//	if (nr < 10)		sprintf (nr_ch, "00%1d", nr);
  //	//	else if (nr < 100)	  sprintf (nr_ch, "0%2d",  nr);
  //	if		(nr < 10)	sprintf (nr_ch, "%1d", nr);
  //	else if (nr < 100)	sprintf (nr_ch, "%2d",	nr);
  //	else				sprintf (nr_ch, "%3d",	nr);
  //	sprintf (filename, "%s%s%s", basename, str, nr_ch);

  sprintf(filename, "%s%s%d", basename, str, nr);
}

void compose_name_plus_nr_str(char basename[256], char str[256], int nr,
                              char filename[256]) {
  // replaced code, ad holten 12-2012
  //	char	nr_ch[256];
  //	if (nr < 10)		sprintf (nr_ch, "%1d", nr);
  //	else if (nr < 100)	  sprintf (nr_ch, "%2d",  nr);
  //	else	 sprintf (nr_ch, "%3d",  nr);
  //	sprintf (filename, "%s%s%s", basename, nr_ch, str);

  sprintf(filename, "%s%d%s", basename, nr, str);
}

/* find nearest neighbours */

int kill_in_list(Tcl_Interp *interp, int nr, int num, int ms_x, int ms_y) {
  int i, imin = 9999, intx, inty;
  double x, y, d, dmin = 9999;

  // if (zoom_f[nr] > 1)	// commented out, ad holten 04-2014
  //{
  //	sprintf (buf, "cannot delete point from zoomed image");
  //	Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  //	Tcl_Eval(interp, ".text delete 3");
  //	Tcl_Eval(interp, ".text insert 3 $tbuf");
  //	return (0);
  //}

  for (i = 0; i < num; i++) {
    x = (double)ms_x - pix[nr][i].x;
    y = (double)ms_y - pix[nr][i].y;
    d = sqrt(x * x + y * y);
    if (d < dmin) {
      dmin = d;
      imin = i;
    }
  }
  if (dmin > 10)
    return (-1); /*  limit: 10 pixel  */

  // intx = (int) pix[nr][imin].x;			// changed, ad holten
  // inty = (int) pix[nr][imin].y;
  img_to_view_coordinates(&intx, &inty, pix[nr][imin].x, pix[nr][imin].y, nr);
  drawcross(interp, intx, inty, cr_sz + 1, nr, "magenta");
  for (i = imin; i < num; i++)
    pix[nr][i] = pix[nr][i + 1];

  return (imin);
}

int nearest_neighbour_geo(coord_2d crd[], int num, double x, double y,
                          double eps) {
  register int j;
  int j0, dj, pnr = -999;
  double d, dmin = 1e20, xmin, xmax, ymin, ymax;

  xmin = x - eps;
  xmax = x + eps;
  ymin = y - eps;
  ymax = y + eps;

  /* binarized search for start point of candidate search */
  for (j0 = num / 2, dj = num / 4; dj > 1; dj /= 2) {
    if (crd[j0].x < xmin)
      j0 += dj;
    else
      j0 -= dj;
  }
  j0 -= 12;
  if (j0 < 0)
    j0 = 0; /* due to trunc */

  for (j = j0; j < num; j++) /* candidate search */
  {
    if (crd[j].x > xmax)
      break; /* finish search */

    if (crd[j].y > ymin && crd[j].y < ymax) {
      d = sqrt((x - crd[j].x) * (x - crd[j].x) +
               (y - crd[j].y) * (y - crd[j].y));
      if (d < dmin) {
        dmin = d;
        pnr = j;
      }
    }
  }
  return (pnr);
}

int nearest_neighbour_pix(target pix[], int num, double x, double y,
                          double eps) {
  register int j;
  int pnr = -999;
  double d, dmin = 1e20, xmin, xmax, ymin, ymax;

  xmin = x - eps;
  xmax = x + eps;
  ymin = y - eps;
  ymax = y + eps;

  for (j = 0; j < num; j++) /* candidate search */
  {
    if (pix[j].y > ymin && pix[j].y < ymax && pix[j].x > xmin &&
        pix[j].x < xmax) {
      d = sqrt((x - pix[j].x) * (x - pix[j].x) +
               (y - pix[j].y) * (y - pix[j].y));
      if (d < dmin) {
        dmin = d;
        pnr = j;
      }
    }
  }
  return (pnr);
}

/***********************************************************************/
/* sorting routines */
/***********************************************************************/

/* bubble sorts */

void bubble_y(coord_2d *item, int count) {
  int i, j;
  coord_2d temp;

  for (i = 1; i < count; ++i)
    for (j = count - 1; j >= i; --j) {
      if (item[j - 1].y > item[j].y) {
        temp = *(item + j - 1);
        *(item + j - 1) = *(item + j);
        *(item + j) = temp;
      }
    }
}

void bubble_conlist(correspond *item, int count) {
  int i, j;
  correspond temp;

  for (i = 1; i < count; ++i)
    for (j = count - 1; j >= i; --j) {
      if (item[j - 1].corr > item[j].corr) {
        temp = *(item + j - 1);
        *(item + j - 1) = *(item + j);
        *(item + j) = temp;
      }
    }
}

void bubble_foundpix1(foundpix *item) {
  int i, j;
  foundpix temp;

  for (i = 1; i < n_img; ++i)
    for (j = n_img - 1; j >= i; --j) {
      if (item[j - 1].freq < item[j].freq) {
        temp = *(item + j - 1);
        *(item + j - 1) = *(item + j);
        *(item + j) = temp;
      }
    }
}

/***********************************************************************/
/* quicksort algorithms for several issues */
/***********************************************************************/

/* quicksort of 2d coordinates in x-order */

void quicksort_coord2d_x(coord_2d *crd, int num) {
  qs_coord2d_x(crd, 0, num - 1);
}

void qs_coord2d_x(coord_2d *crd, int left, int right) {
  register int i, j;
  double xm;
  coord_2d temp;

  i = left;
  j = right;
  xm = crd[(left + right) / 2].x;

  do {
    while (crd[i].x < xm && i < right)
      i++;
    while (xm < crd[j].x && j > left)
      j--;

    if (i <= j) {
      temp = crd[i];
      crd[i] = crd[j];
      crd[j] = temp;
      i++;
      j--;
    }
  } while (i <= j);

  if (left < j)
    qs_coord2d_x(crd, left, j);
  if (i < right)
    qs_coord2d_x(crd, i, right);
}

/***********************************************************************/

/* quicksort of 2d coordinates in pnr-order */

void quicksort_coord2d_pnr(coord_2d *crd, int num) {
  qs_coord2d_pnr(crd, 0, num - 1);
}

void qs_coord2d_pnr(coord_2d *crd, int left, int right) {
  register int i, j;
  double pnrm;
  coord_2d temp;

  i = left;
  j = right;
  pnrm = crd[(left + right) / 2].pnr;

  do {
    while (crd[i].pnr < pnrm && i < right)
      i++;
    while (pnrm < crd[j].pnr && j > left)
      j--;

    if (i <= j) {
      temp = crd[i];
      crd[i] = crd[j];
      crd[j] = temp;
      i++;
      j--;
    }
  } while (i <= j);

  if (left < j)
    qs_coord2d_pnr(crd, left, j);
  if (i < right)
    qs_coord2d_pnr(crd, i, right);
}

/***********************************************************************/

/* quicksort of targets in y-order */

void quicksort_target_y(target *pix, int num) { qs_target_y(pix, 0, num - 1); }

void qs_target_y(target *pix, int left, int right) {
  register int i, j;
  double ym;
  target temp;

  i = left;
  j = right;
  ym = pix[(left + right) / 2].y;

  do {
    while (pix[i].y < ym && i < right)
      i++;
    while (ym < pix[j].y && j > left)
      j--;

    if (i <= j) {
      temp = pix[i];
      pix[i] = pix[j];
      pix[j] = temp;
      i++;
      j--;
    }
  } while (i <= j);

  if (left < j)
    qs_target_y(pix, left, j);
  if (i < right)
    qs_target_y(pix, i, right);
}

/***********************************************************************/

/* quicksort for list of correspondences in order of match quality */
/* 4 camera version */

void quicksort_con(n_tupel *con, int num) { qs_con(con, 0, num - 1); }

void qs_con(n_tupel *con, int left, int right) {
  register int i, j;
  double xm;
  n_tupel temp;

  i = left;
  j = right;
  xm = con[(left + right) / 2].corr;

  do {
    while (con[i].corr > xm && i < right)
      i++;
    while (xm > con[j].corr && j > left)
      j--;

    if (i <= j) {
      temp = con[i];
      con[i] = con[j];
      con[j] = temp;
      i++;
      j--;
    }
  } while (i <= j);

  if (left < j)
    qs_con(con, left, j);
  if (i < right)
    qs_con(con, i, right);
}

/***********************************************************************/

void tclimg2cimg(Tcl_Interp *interp, unsigned char *c_img,
                 Tk_PhotoImageBlock *tcl_img) {
  int i, j;
  i = 0;
  for (j = 0; j < imgsize; j++) {
    i = j * 4;
    *(tcl_img->pixelPtr + i) = *(c_img + j);
    *(tcl_img->pixelPtr + i + 1) = *(c_img + j);
    *(tcl_img->pixelPtr + i + 2) = *(c_img + j);
    /* value for alpha canal 42, answer to all questions*/
    *(tcl_img->pixelPtr + i + 3) = 42;
  }
}

void cimg2tclimg(Tcl_Interp *interp, unsigned char *c_img,
                 Tk_PhotoImageBlock *tcl_img, int alpha) {
  int i, j;
  for (j = 0; j < imgsize; j++) {
    i = j * 4;
    *(tcl_img->pixelPtr + i) = *(c_img + j);
    *(tcl_img->pixelPtr + i + 1) = *(c_img + j);
    *(tcl_img->pixelPtr + i + 2) = *(c_img + j);
    *(tcl_img->pixelPtr + i + 3) = alpha;
  }
}
