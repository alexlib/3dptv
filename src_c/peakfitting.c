/****************************************************************************

Routine:			peakfitting.c

Author/Copyright:	Hans-Gerd Maas

Address:			Institute of Geodesy and Photogrammetry
                                        ETH - Hoenggerberg
                                        CH - 8093 Zurich

Creation Date:		Feb. 1990

Description:

****************************************************************************/
/*
Copyright (c) 1990-2011 ETH Zurich

See the file license.txt for copying permission.
*/

#include "ptv.h"

void check_touch(peak *tpeak, int p1, int p2);

int peak_fit_new(Tcl_Interp *interp, unsigned char *img, char par_file[],
                 int xmin, int xmax, int ymin, int ymax, target pix[], int nr)
/*
  unsigned char *img            image data
  char     par_file[]           name of parameter file
  int      xmin,xmax,ymin,ymax  search area
  target   pix[]                pixel coord array, global
  int      nr                   image number for display
*/
/*	newest peak fitting technique for particle coordinate determination  */
/*	labeling with discontinuity,
        reunification with distance and profile criteria  */
{
  int n_peaks = 0;  /* # of peaks detected */
  int n_wait;       /* size of waitlist for connectivity */
  int x8[8], y8[8]; /* neighbours for connectivity */
  int p2;           /* considered point number */
  int sumg_min, gvthres[4], thres, disco, nxmin, nxmax, nymin, nymax, nnmin,
      nnmax, rel_disc, dummy;
  /* parameters for target acceptance */
  int pnr, sumg, xn, yn; /* collecting variables for center of gravity */
  int n_target = 0;      /* # of targets detected */
  int intx1, inty1;      /* pixels for profile test and crosses */
  int unify;             /* flag for unification of targets */
  int unified = 0;       /* # of unified targets */
  int non_unified = 0;   /* # of tested, but not unified targets */
  register int i, j, k, l, m, n; /* loop variables */
  unsigned char gv, gvref;       /* current and reference greyvalue */
  unsigned char gv1, gv2;        /* greyvalues for profile test */
  void check_touch();            /* marks touch events */
  double x1, x2, y1, y2, s12;    /* values for profile test */
  double x, y;                   /* preliminary coordinates */
  short *label_img;              /* target number labeling */
  peak *peaks, *ptr_peak;        /* detected peaks */
  targpix waitlist[2048];        /* pix to be tested for connectivity */
  FILE *fpp;                     /* parameter file pointer */

  /* read image name, threshold and shape limits from parameter file */

  /*------------------------------------------------------------------------*/

  fpp = fopen_rp(par_file); // replaced fopen_r(), ad holten 12-2012
  if (!fpp)
    return 0;
  fscanf(fpp, "%d", &gvthres[0]); /* threshold for binarization 1.image */
  fscanf(fpp, "%d", &gvthres[1]); /* threshold for binarization 2.image */
  fscanf(fpp, "%d", &gvthres[2]); /* threshold for binarization 3.image */
  fscanf(fpp, "%d", &gvthres[3]); /* threshold for binarization 4.image */
  fscanf(fpp, "%d", &disco);      /* max discontinuity */
  fscanf(fpp, "%d  %d", &nnmin, &nnmax); /* min. and max. number of */
  fscanf(fpp, "%d  %d", &nxmin, &nxmax); /* pixels per target,	   */
  fscanf(fpp, "%d  %d", &nymin, &nymax); /* abs, in x, in y		  */
  fscanf(fpp, "%d", &sumg_min);          /* min. sumg */
  fscanf(fpp, "%d", &cr_sz);             /* size of crosses */
  fscanf(fpp, "%d", &dummy);
  fscanf(fpp, "%d", &dummy);
  fscanf(fpp, "%d", &rel_disc);
  fclose(fpp);

  /* give thres value refering to image number */
  thres = gvthres[nr];

  /*------------------------------------------------------------------------*/
  /* allocate memory */
  /*------------------------------------------------------------------------*/

  label_img = (short *)calloc(imgsize, sizeof(short));
  peaks = (peak *)malloc(4 * NMAX * sizeof(peak));
  ptr_peak = peaks;

  /*------------------------------------------------------------------------*/
  /* 1.: connectivity analysis with peak search and discontinuity criterion */
  /*------------------------------------------------------------------------*/

  puts("Searching local maxima, connectivity analysis, peak factor 2 set");

  // for (i=ymin; i<ymax-1; i++)
  //	for (j=xmin; j<xmax; j++) //Beat L�thi Jan 09 I changed to (i=ymin;
  //i<ymax-1; i++), new:-1

  // replaced by the next code to avoid passing image bounds, ad holten 12-2012
  if (ymin < 1)
    ymin = 1;
  if (ymax > imy - 1)
    ymax = imy - 1;

  for (i = ymin; i < ymax; i++)
    for (j = xmin; j < xmax; j++) {
      n = i * imx + j;

      /* compare with threshold */
      gv = *(img + n);
      if (gv <= 2 * thres)
        continue;

      /* skip already labeled pixel */
      if (*(label_img + n) != 0)
        continue;

      /* check, wether pixel is a local maximum */
      if (gv >= *(img + n - 1) && gv >= *(img + n + 1) &&
          gv >= *(img + n - imx) && gv >= *(img + n + imx) &&
          gv >= *(img + n - imx - 1) && gv >= *(img + n + imx - 1) &&
          gv >= *(img + n - imx + 1) && gv >= *(img + n + imx + 1)) {
        /* label peak in label_img, initialize peak */
        n_peaks++;
        *(label_img + n) = n_peaks;
        ptr_peak->pos = n;
        ptr_peak->status = 1;
        ptr_peak->xmin = j;
        ptr_peak->xmax = j;
        ptr_peak->ymin = i;
        ptr_peak->ymax = i;
        ptr_peak->unr = 0;
        ptr_peak->n = 0;
        ptr_peak->sumg = 0;
        ptr_peak->x = 0;
        ptr_peak->y = 0;
        ptr_peak->n_touch = 0;
        for (k = 0; k < 4; k++)
          ptr_peak->touch[k] = 0;
        ptr_peak++;

        waitlist[0].x = j;
        waitlist[0].y = i;
        n_wait = 1;

        while (n_wait > 0) {
          gvref = *(img + imx * (waitlist[0].y) + (waitlist[0].x));

          x8[0] = waitlist[0].x - 1;
          y8[0] = waitlist[0].y;
          x8[1] = waitlist[0].x + 1;
          y8[1] = waitlist[0].y;
          x8[2] = waitlist[0].x;
          y8[2] = waitlist[0].y - 1;
          x8[3] = waitlist[0].x;
          y8[3] = waitlist[0].y + 1;
          /*
          x8[4] = waitlist[0].x - 1;	  y8[4] = waitlist[0].y - 1;
          x8[5] = waitlist[0].x + 1;	  y8[5] = waitlist[0].y - 1;
          x8[6] = waitlist[0].x - 1;	  y8[6] = waitlist[0].y + 1;
          x8[7] = waitlist[0].x + 1;	  y8[7] = waitlist[0].y + 1;
          */

          /*
          for (k=0; k<8; k++)
          */
          for (k = 0; k < 4; k++) {
            yn = y8[k];
            xn = x8[k];
            // if (xn<0 || xn>imx || yn<0 || yn>imy) continue;		// Bug repaired,
            // ad holten, 12-2012
            if (xn < 1 || xn > imx - 2 || yn < 1 || yn > imy - 2)
              continue;

            n = imx * yn + xn;
            if (*(label_img + n) != 0)
              continue;
            gv = *(img + n);

            /* conditions for threshold, discontinuity and peak fitting */
            if ((gv > thres) &&
                (gv <= gvref + disco)
                //		removed the image borders test from Beat, ad
                //holten, 2012
                //		&& (xn>=xmin)&&(xn<xmax) &&
                //(yn>=ymin)&&(yn<ymax-1)//Beat L�thi Jan 09 I changed to
                //(i=ymin; i<ymax-1; i++), new:-1
                && (gvref + disco >= *(img + imx * (yn - 1) + xn)) &&
                (gvref + disco >= *(img + imx * (yn + 1) + xn)) &&
                (gvref + disco >= *(img + imx * yn + (xn - 1))) &&
                (gvref + disco >= *(img + imx * yn + (xn + 1)))
                /*
                && (gvref + disco >= *(img + imx*(yn-1) + xn-1))
                && (gvref + disco >= *(img + imx*(yn-1) + xn+1))
                && (gvref + disco >= *(img + imx*(yn+1) + (xn-1)))
                && (gvref + disco >= *(img + imx*(yn+1) + (xn+1)))	*/
            ) {
              *(label_img + imx * yn + xn) = n_peaks;
              waitlist[n_wait].x = xn;
              waitlist[n_wait].y = yn;
              n_wait++;
            }
          }

          n_wait--;
          for (m = 0; m < n_wait; m++)
            waitlist[m] = waitlist[m + 1];
          waitlist[n_wait].x = 0;
          waitlist[n_wait].y = 0;
        } /*  end of while-loop	*/
      }
    }

  /*------------------------------------------------------------------------*/
  /* 2.:	process label image */
  /*		(collect data for center of gravity, shape and brightness
   * parameters) */
  /*		get touch events */
  /*------------------------------------------------------------------------*/

  puts("Collecting peak data");

  for (i = ymin; i < ymax; i++)
    for (j = xmin; j < xmax; j++) {
      n = i * imx + j;

      if (*(label_img + n) > 0) {
        /* process pixel */
        pnr = *(label_img + n);
        gv = *(img + n);
        ptr_peak = peaks + pnr - 1;
        ptr_peak->n++;
        ptr_peak->sumg += gv;
        ptr_peak->x += (j * gv);
        ptr_peak->y += (i * gv);
        if (j < ptr_peak->xmin)
          ptr_peak->xmin = j;
        if (j > ptr_peak->xmax)
          ptr_peak->xmax = j;
        if (i < ptr_peak->ymin)
          ptr_peak->ymin = i;
        if (i > ptr_peak->ymax)
          ptr_peak->ymax = i;

        /* get touch events */
        // bugs repaired, ad holten 12-2012 i = Y-axis, j = X-axis !!!
        // if (i>0 && j>1) 	check_touch (ptr_peak, pnr,
        // *(label_img+n-imx-1)); if (i>0)			check_touch
        // (ptr_peak, pnr, *(label_img+n-imx)); if (i>0 && j<imy-1) check_touch
        // (ptr_peak, pnr, *(label_img+n-imx+1));

        // if (j>0)			check_touch (ptr_peak, pnr,
        // *(label_img+n-1));
        // if (j<imy-1)		check_touch (ptr_peak, pnr, *(label_img+n+1));

        // if (i<imx-1 && j>0) check_touch (ptr_peak, pnr,
        // *(label_img+n+imx-1)); if (i<imx-1)		check_touch (ptr_peak,
        // pnr, *(label_img+n+imx)); if (i<imx-1 && j<imy-1) check_touch
        // (ptr_peak, pnr, *(label_img+n+imx+1));

        if (i > 0 && j > 0)
          check_touch(ptr_peak, pnr, *(label_img + n - imx - 1));
        if (i > 0)
          check_touch(ptr_peak, pnr, *(label_img + n - imx));
        if (i > 0 && j < imx - 1)
          check_touch(ptr_peak, pnr, *(label_img + n - imx + 1));

        if (j > 0)
          check_touch(ptr_peak, pnr, *(label_img + n - 1));
        if (j < imx - 1)
          check_touch(ptr_peak, pnr, *(label_img + n + 1));

        if (i < imy - 1 && j > 0)
          check_touch(ptr_peak, pnr, *(label_img + n + imx - 1));
        if (i < imy - 1)
          check_touch(ptr_peak, pnr, *(label_img + n + imx));
        if (i < imy - 1 && j < imx - 1)
          check_touch(ptr_peak, pnr, *(label_img + n + imx + 1));
      }
    }

  /*------------------------------------------------------------------------*/
  /* 3.:	  reunification test: profile and distance */
  /*------------------------------------------------------------------------*/

  puts("Unifying targets");

  for (i = 0; i < n_peaks; i++) {
    if (peaks[i].n_touch == 0)
      continue; /* no touching targets */
    if (peaks[i].unr != 0)
      continue; /* target already unified */

    /* profile criterion */
    /* point 1 */
    x1 = peaks[i].x / peaks[i].sumg;
    y1 = peaks[i].y / peaks[i].sumg;
    gv1 = *(img + peaks[i].pos);

    /* consider all touching points */
    for (j = 0; j < peaks[i].n_touch; j++) {
      p2 = peaks[i].touch[j] - 1;

      if (p2 >= n_peaks)
        continue; /* workaround memory overwrite problem */
      if (p2 < 0)
        continue; /*  workaround memory overwrite problem */
      if (peaks[p2].unr != 0)
        continue; /* target already unified */

      /* point 2 */
      x2 = peaks[p2].x / peaks[p2].sumg;
      y2 = peaks[p2].y / peaks[p2].sumg;

      gv2 = *(img + peaks[p2].pos);

      s12 = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

      /* consider profile dot for dot */
      /* if any points is by more than disco below profile, do not unify */
      if (s12 < 2.0)
        unify = 1;
      else {
        for (unify = 1, l = 1; l < s12; l++) {
          intx1 = (int)(x1 + l * (x2 - x1) / s12);
          inty1 = (int)(y1 + l * (y2 - y1) / s12);
          if (rel_disc == 0) { // Beat March 2011
            gv = *(img + inty1 * imx + intx1) + disco;
            if (gv < (gv1 + l * (gv2 - gv1) / s12) || gv < gv1 || gv < gv2)
              unify = 0;
            if (unify == 0)
              break;
          } else { // Beat March 2011
            gv = *(img + inty1 * imx + intx1);
            if (gv < (gv1 + l * (gv2 - gv1) / s12) * (double)disco * 0.01)
              unify = 0;
            if (unify == 0)
              break;
          }
        }
      }
      if (unify == 0) {
        non_unified++;
        continue;
      }

      /* otherwise unify targets */
      unified++;
      peaks[i].unr = p2;
      peaks[p2].x += peaks[i].x;
      peaks[p2].y += peaks[i].y;
      peaks[p2].sumg += peaks[i].sumg;
      peaks[p2].n += peaks[i].n;
      if (peaks[i].xmin < peaks[p2].xmin)
        peaks[p2].xmin = peaks[i].xmin;
      if (peaks[i].ymin < peaks[p2].ymin)
        peaks[p2].ymin = peaks[i].ymin;
      if (peaks[i].xmax > peaks[p2].xmax)
        peaks[p2].xmax = peaks[i].xmax;
      if (peaks[i].ymax > peaks[p2].ymax)
        peaks[p2].ymax = peaks[i].ymax;
    }
  }

  /*------------------------------------------------------------------------*/
  /* 4.:	  process targets */
  /*------------------------------------------------------------------------*/

  puts("Calculate centers of gravity");

  for (i = 0; i < n_peaks; i++) {
    /* check whether target touches image borders */
    if (peaks[i].xmin == xmin && (xmax - xmin) > 32)
      continue;
    if (peaks[i].ymin == ymin && (xmax - xmin) > 32)
      continue;
    if (peaks[i].xmax == xmax - 1 && (xmax - xmin) > 32)
      continue;
    if (peaks[i].ymax == ymax - 1 && (xmax - xmin) > 32)
      continue;

    if (peaks[i].unr == 0 && peaks[i].sumg > sumg_min &&
        (peaks[i].xmax - peaks[i].xmin + 1) >= nxmin &&
        (peaks[i].ymax - peaks[i].ymin + 1) >= nymin &&
        (peaks[i].xmax - peaks[i].xmin) < nxmax &&
        (peaks[i].ymax - peaks[i].ymin) < nymax && peaks[i].n >= nnmin &&
        peaks[i].n <= nnmax) {
      sumg = peaks[i].sumg;

      /* target coordinates */
      pix[n_target].x = 0.5 + peaks[i].x / sumg;
      pix[n_target].y = 0.5 + peaks[i].y / sumg;

      /* draw cross */
      if (display)
        drawcross(interp, (int)pix[n_target].x, (int)pix[n_target].y, cr_sz, nr,
                  "blue");

      /* target shape parameters */
      pix[n_target].sumg = sumg;
      pix[n_target].n = peaks[i].n;
      pix[n_target].nx = peaks[i].xmax - peaks[i].xmin + 1;
      pix[n_target].ny = peaks[i].ymax - peaks[i].ymin + 1;
      pix[n_target].tnr = -1;
      pix[n_target].pnr = n_target;
      n_target++;
    }
  }

  /* get number of touch events */
  if (examine == 10) {
    for (x = 0, i = 0; i < n_target; i++)
      x += pix[i].n;
    x /= n_target;
    printf("Average number of pix per target: %6.3f\n", x);

    for (sumg = 0, i = 0; i < n_peaks; i++)
      sumg += peaks[i].n_touch;
    printf("Number of touch events: %d\n", sumg / 2);

    y = 2 * n_target * n_target * x / imgsize;
    printf("expected number of touch events: %6.0f\n", y);
  }
  free(label_img);
  free(peaks);
  return (n_target);
}

void check_touch(peak *tpeak, int p1, int p2)
/* check wether p1, p2 are already marked as touching and mark them otherwise */
{
  int m, done;

  if (p2 == 0)
    return; /* p2 not labeled */
  if (p2 == p1)
    return; /* p2 belongs to p1 */

  /* check wether p1, p2 are already marked as touching */
  for (done = 0, m = 0; m < tpeak->n_touch; m++)
    if (tpeak->touch[m] == p2)
      done = 1;

  /* mark touch event */
  if (done == 0) {
    tpeak->touch[tpeak->n_touch] = p2;
    tpeak->n_touch++;
    /* don't allow for more than 4 touchs */
    if (tpeak->n_touch > 3)
      tpeak->n_touch = 3;
  }
}
