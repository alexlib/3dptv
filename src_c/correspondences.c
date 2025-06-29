/****************************************************************************

Routine:		  correspondences.c

Author/Copyright: Hans-Gerd Maas

Address:		  Institute of Geodesy and Photogrammetry
                                  ETH - Hoenggerberg
                                  CH - 8093 Zurich

Creation Date:	  1988/89

Description:	  establishment of correspondences for 2/3/4 cameras

****************************************************************************/
/*
Copyright (c) 1990-2011 ETH Zurich

See the file license.txt for copying permission.
*/

#include "ptv.h"  // NOLINT

/* #define MAXCAND 100 */

/****************************************************************************/
/*--------------- 4 camera model: consistent quadruplets -------------------*/
/****************************************************************************/

void correspondences_4(Tcl_Interp *interp, const char **argv) {
  int i, j, k, l, m, n, o, i1, i2, i3;
  int count, match0 = 0, match4 = 0, match3 = 0, match2 = 0, match1 = 0;
  int p1, p2, p3, p4, p31, p41, p42, pt1;
  int tim[4][NMAX], intx, inty;
  double xa12, ya12, xb12, yb12, X, Y, Z, corr;
  candidate cand[MAXCAND];
  n_tupel *con0;
  correspond *list[4][4];
  Zoompar zoompar[4];
  char buf[256];
  /* ----------------------------------------------------------------------- */

  /* allocate memory for lists of correspondences */
  for (i1 = 0; i1 < n_img - 1; i1++)
    for (i2 = i1 + 1; i2 < n_img; i2++)
      list[i1][i2] = (correspond *)malloc(num[i1] * sizeof(correspond));  // NOLINT

  con0 = (n_tupel *)malloc(4 * NMAX * sizeof(n_tupel));  // NOLINT

  /* ----------------------------------------------------------------------- */

  printf("in corres zmin0: %f, zmax0: %f\n", Zmin_lay[0], Zmax_lay[0]);

  /*  initialize ...  */
  snprintf(buf, sizeof(buf), "Establishing correspondences");
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 2");
  Tcl_Eval(interp, ".text insert 2 $tbuf");

  match = 0;
  match0 = 0;
  match2 = 0;

  for (i1 = 0; i1 < n_img - 1; i1++)
    for (i2 = i1 + 1; i2 < n_img; i2++)
      for (i = 0; i < num[i1]; i++) {
        list[i1][i2][i].p1 = 0;
        list[i1][i2][i].n = 0;
      }

  for (i = 0; i < NMAX; i++) {
    con0[i].corr = 0;
    for (j = 0; j < 4; j++)
      con0[i].p[j] = -1;
    for (j = 0; j < 4; j++)
      tim[j][i] = 0;
  }

  /* ------- if only one cam and 2D ------- */  // by Beat Luthi June 2007
  if (n_img == 1) {
    if (res_name[0] == 0)
      snprintf(res_name, sizeof(res_name), "rt_is");
    fp1 = fopen(res_name, "w");
    fprintf(fp1, "%4d\n", num[0]);
    for (i = 0; i < num[0]; i++) {
      o = epi_mm_2D(geo[0][i].x, geo[0][i].y, Ex[0], I[0], G[0], mmp, &X, &Y,
                    &Z);
      pix[0][geo[0][i].pnr].tnr = i;
      fprintf(fp1, "%4d", i + 1);
      fprintf(fp1, " %9.3f %9.3f %9.3f", X, Y, Z);  /* Write 3D coordinates */
      fprintf(fp1, " %4d", geo[0][i].pnr);
      fprintf(fp1, " %4d", -1);
      fprintf(fp1, " %4d", -1);
      fprintf(fp1, " %4d\n", -1);
    }
    fclose(fp1);
    match1 = num[0];
  }
  /* -------------end of only one cam and 2D ------------ */

  /* matching  1 -> 2,3,4  +  2 -> 3,4  +  3 -> 4 */

  for (i1 = 0; i1 < n_img - 1; i1++)
    for (i2 = i1 + 1; i2 < n_img; i2++) {
      snprintf(buf, sizeof(buf), "Establishing correspondences  %d - %d", i1, i2);
      puts(buf);

      /* establish correspondences from num[i1] points of img[i1] to img[i2] */
      for (i = 0; i < num[i1]; i++)
        if (geo[i1][i].x != -999) {
          o = epi_mm(geo[i1][i].x, geo[i1][i].y, Ex[i1], I[i1], G[i1], Ex[i2],
                     I[i2], G[i2], mmp, &xa12, &ya12, &xb12, &yb12);

          /* origin point in the list */
          p1 = i;
          list[i1][i2][p1].p1 = p1;
          pt1 = geo[i1][p1].pnr;

          /* search for a conjugate point in geo[i2] */
          find_candidate_plus(geo[i2], pix[i2], num[i2], xa12, ya12, xb12, yb12,
                              eps0, pix[i1][pt1].n, pix[i1][pt1].nx,
                              pix[i1][pt1].ny, pix[i1][pt1].sumg, cand, &count,
                              i2, argv);

          /* write all corresponding candidates to the preliminary list */
          /* of correspondences */
          if (count > MAXCAND)
            count = MAXCAND;
          for (j = 0; j < count && j < MAXCAND; j++) {
            list[i1][i2][p1].p2[j] = cand[j].pnr;
            list[i1][i2][p1].corr[j] = cand[j].corr;
            list[i1][i2][p1].dist[j] = cand[j].tol;
          }
          list[i1][i2][p1].n = count;
        }
    }
  // commented out, ad holten, 12-2012
  // /* repair memory fault (!?) */
  // for (j=0; j<4; j++) for (i=0; i<NMAX; i++) tim[j][i] = 0;

  // but tested, to be sure that everything is alright. WILL BE REMOVED LATER
  for (j = 0; j < 4; j++)
    for (i = 0; i < NMAX; i++)
      if (tim[j][i] != 0) {
        printf(">>>>>>>>>> correspondences_4: ERROR on line 144, CHECK CODE "
               "<<<<<<<<<<<<<<<< \n");
        i = NMAX;
        j = 4;
      }

  /* ------------------------------------------------------------------ */
  /* ------------------------------------------------------------------ */

  /* search consistent quadruplets in the list */
  if (n_img == 4) {
    puts("Search consistent quadruplets");
    for (i = 0, match0 = 0; i < num[0]; i++) {
      p1 = list[0][1][i].p1;
      for (j = 0; j < list[0][1][i].n; j++)
        for (k = 0; k < list[0][2][i].n; k++)
          for (l = 0; l < list[0][3][i].n; l++) {
            p2 = list[0][1][i].p2[j];
            p3 = list[0][2][i].p2[k];
            p4 = list[0][3][i].p2[l];
            for (m = 0; m < list[1][2][p2].n; m++)
              for (n = 0; n < list[1][3][p2].n; n++) {
                p31 = list[1][2][p2].p2[m];
                p41 = list[1][3][p2].p2[n];
                if (p3 == p31 && p4 == p41) {
                  for (o = 0; o < list[2][3][p3].n; o++) {
                    p42 = list[2][3][p3].p2[o];
                    if (p4 == p42) {
                      corr = (list[0][1][i].corr[j] + list[0][2][i].corr[k] +
                              list[0][3][i].corr[l] + list[1][2][p2].corr[m] +
                              list[1][3][p2].corr[n] + list[2][3][p3].corr[o]) /
                             (list[0][1][i].dist[j] + list[0][2][i].dist[k] +
                              list[0][3][i].dist[l] + list[1][2][p2].dist[m] +
                              list[1][3][p2].dist[n] + list[2][3][p3].dist[o]);
                      if (corr > corrmin) {
                        /* accept as preliminary match */
                        con0[match0].p[0] = p1;
                        con0[match0].p[1] = p2;
                        con0[match0].p[2] = p3;
                        con0[match0].p[3] = p4;
                        con0[match0++].corr = corr;
                        if (match0 == 4 * NMAX) { /* security */
                          printf("Overflow in correspondences:");
                          printf(" > %d matches\n", match0);
                          i = num[0];
                        }
                      }
                    }
                  }
                }
              }
          }
    }
    /* -------------------------------------------------------------------- */

    /* sort quadruplets for match quality (.corr) */

    quicksort_con(con0, match0);

    /* -------------------------------------------------------------------- */

    /* take quadruplets from the top to the bottom of the sorted list
       only if none of the points has already been used */
    // simplified 'if (p1 > -1) if (++tim[0][p1] > 1)' to
    //            'if (p1 > -1 && ++tim[0][p1] > 1)'   and so on, ad holten
    //            12-2012
    for (i = 0, match = 0; i < match0; i++) {
      p1 = con0[i].p[0];
      if (p1 > -1 && ++tim[0][p1] > 1)
        continue;
      p2 = con0[i].p[1];
      if (p2 > -1 && ++tim[1][p2] > 1)
        continue;
      p3 = con0[i].p[2];
      if (p3 > -1 && ++tim[2][p3] > 1)
        continue;
      p4 = con0[i].p[3];
      if (p4 > -1 && ++tim[3][p4] > 1)
        continue;
      con[match++] = con0[i];
    }

    match4 = match;
    snprintf(buf, sizeof(buf), "%d consistent quadruplets, ", match4);
    puts(buf);
  }

  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */

  /* search consistent triplets :  123, 124, 134, 234 */
  if ((n_img == 4 && allCam_flag == 0) || n_img == 3) {
    puts("Search consistent triplets");
    match0 = 0;
    for (i1 = 0; i1 < n_img - 2; i1++)
      for (i2 = i1 + 1; i2 < n_img - 1; i2++)
        for (i3 = i2 + 1; i3 < n_img; i3++)
          for (i = 0; i < num[i1]; i++) {
            p1 = list[i1][i2][i].p1;
            if (p1 > NMAX || tim[i1][p1] > 0)
              continue;

            for (j = 0; j < list[i1][i2][i].n; j++)
              for (k = 0; k < list[i1][i3][i].n; k++) {
                p2 = list[i1][i2][i].p2[j];
                if (p2 > NMAX || tim[i2][p2] > 0)
                  continue;
                p3 = list[i1][i3][i].p2[k];
                if (p3 > NMAX || tim[i3][p3] > 0)
                  continue;

                for (m = 0; m < list[i2][i3][p2].n; m++) {
                  p31 = list[i2][i3][p2].p2[m];
                  if (p3 == p31) {
                    corr = (list[i1][i2][i].corr[j] + list[i1][i3][i].corr[k] +
                            list[i2][i3][p2].corr[m]) /
                           (list[i1][i2][i].dist[j] + list[i1][i3][i].dist[k] +
                            list[i2][i3][p2].dist[m]);
                    if (corr > corrmin) {
                      for (n = 0; n < n_img; n++)
                        con0[match0].p[n] = -2;
                      con0[match0].p[i1] = p1;
                      con0[match0].p[i2] = p2;
                      con0[match0].p[i3] = p3;
                      con0[match0++].corr = corr;
                    }
                  }
                }
              }
          }

    /* -----------------------------------------------------------------------
    */

    /* sort triplets for match quality (.corr) */
    quicksort_con(con0, match0);

    /* -----------------------------------------------------------------------
     */
    // pragmatic version:
    // take triplets from the top to the bottom of the sorted list
    // only if none of the points has already been used
    for (i = 0; i < match0; i++) {
      p1 = con0[i].p[0];
      if (p1 > -1 && ++tim[0][p1] > 1)
        continue;
      p2 = con0[i].p[1];
      if (p2 > -1 && ++tim[1][p2] > 1)
        continue;
      p3 = con0[i].p[2];
      if (p3 > -1 && ++tim[2][p3] > 1)
        continue;
      p4 = con0[i].p[3];
      if (p4 > -1 && n_img > 3 && ++tim[3][p4] > 1)
        continue;

      con[match++] = con0[i];
    }

    match3 = match - match4;
    snprintf(buf, sizeof(buf), "%d consistent quadruplets, %d triplets ", match4, match3);
    puts(buf);

    ///* repair artifact (?) */            // commented out ad holten,
    // /12-2012
    // if (n_img == 3)
    // for (i=0; i<match; i++) con[i].p[3] = -1;
    if (n_img == 3) {  // added for a test (WILL BE REMOVED LATER)
      for (i = 0; i < match; i++)
        if (con[i].p[3] != -1) {
          printf(">>>>>>>>>>>>>> correspondences_4: ERROR on line 144, CHECK "
                 "CODE <<<<<<<<<<<<<<<<< \n");
          con[i].p[3] = -1;
        }
    }
  }

  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */

  // search consistent pairs :  12, 13, 14, 23, 24, 34

  // only if an object model is available or if only 2 images are used
  // if (1<2 && n_img>1 && allCam_flag==0) {        //  removed 1<2, ad holten,
  // 12-2012
  if (n_img > 1 && allCam_flag == 0) {
    puts("Search pairs");

    match0 = 0;
    for (i1 = 0; i1 < n_img - 1; i1++) {
      // if ( n_img == 2 || (num[0] < 64 && num[1] < 64 && num[2] < 64 && num[3]
      // < 64))
      if (n_img > 1) {
        for (i2 = i1 + 1; i2 < n_img; i2++)
          for (i = 0; i < num[i1]; i++) {
            p1 = list[i1][i2][i].p1;
            if (p1 > NMAX || tim[i1][p1] > 0)
              continue;

            /* take only unambigous pairs */
            if (list[i1][i2][i].n != 1)
              continue;

            p2 = list[i1][i2][i].p2[0];
            if (p2 > NMAX || tim[i2][p2] > 0)
              continue;

            corr = list[i1][i2][i].corr[0] / list[i1][i2][i].dist[0];

            if (corr > corrmin) {
              con0[match0].p[i1] = p1;
              con0[match0].p[i2] = p2;
              con0[match0++].corr = corr;
            }
          }
      }
    }
    /* -----------------------------------------------------------------------
     */

    /* sort pairs for match quality (.corr) */
    quicksort_con(con0, match0);

    /* -----------------------------------------------------------------------
     */

    // take pairs from the top to the bottom of the sorted list
    // only if none of the points has already been used
    for (i = 0; i < match0; i++) {
      p1 = con0[i].p[0];
      if (p1 > -1 && ++tim[0][p1] > 1)
        continue;
      p2 = con0[i].p[1];
      if (p2 > -1 && ++tim[1][p2] > 1)
        continue;
      p3 = con0[i].p[2];
      if (p3 > -1 && n_img > 2 && ++tim[2][p3] > 1)
        continue;
      p4 = con0[i].p[3];
      if (p4 > -1 && n_img > 3 && ++tim[3][p4] > 1)
        continue;

      con[match++] = con0[i];
    }
  }  // end pairs

  match2 = match - match4 - match3;
  if (n_img == 1) {
    snprintf(buf, sizeof(buf), "determined %d points from 2D", match1);
    puts(buf);
  } else {
    snprintf(buf, sizeof(buf),
            "%d consistent quadruplets(red), %d triplets(green) and %d "
            "unambigous pairs",
            match4, match3, match2);
    puts(buf);
  }
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 3");
  Tcl_Eval(interp, ".text insert 3 $tbuf");

  /* ----------------------------------------------------------------------- */

  /* give each used pix the correspondence number */
  for (i = 0; i < match; i++)
    for (j = 0; j < n_img; j++)
      if (con[i].p[j] > -1) {  // Bug, detected in Nov 2011 by Koni&Beat
        p1 = geo[j][con[i].p[j]].pnr;
        if (p1 > -1 && p1 < num[j])
          pix[j][p1].tnr = i;
      }

  /* draw crosses on canvas */
  if (display) {
    clear_drawnobjectslist(n_img);

    for (i = 0; i < n_img; i++)
      get_tclzoomparms(interp, &zoompar[i], i);

    // for (i=0; i<4; i++) {
    // zfac[i] = zoom_f[i];
    //  if (zoom_f[i]<0) zoom_f[i] = 1;
    //}
    for (i = 0; i < match4; i++) { /* red crosses for quadruplets */
      for (j = 0; j < n_img; j++) {
        p1 = geo[j][con[i].p[j]].pnr;
        if (p1 > -1 && isinview(pix[j][p1].x, pix[j][p1].y, j)) {
          img_to_view_coordinates(&intx, &inty, pix[j][p1].x, pix[j][p1].y, j);
          drawcross(interp, intx, inty, cr_sz, j, "red");
          draw_pnr_autohide(interp, intx + 5, inty + 0, i, j, "white", 2,
                            zoompar[j].fac);
        }
      }
    }
    for (i = match4; i < match4 + match3;
         i++) { /* green crosses for triplets */
      for (j = 0; j < n_img; j++) {
        p1 = geo[j][con[i].p[j]].pnr;
        if (p1 > -1 && con[i].p[j] > -1 &&
            isinview(pix[j][p1].x, pix[j][p1].y, j)) {
          img_to_view_coordinates(&intx, &inty, pix[j][p1].x, pix[j][p1].y, j);
          drawcross(interp, intx, inty, cr_sz, j, "green");
          draw_pnr_autohide(interp, intx + 5, inty + 0, i, j, "white", 2,
                            zoompar[j].fac);
        }
      }
    }
    for (i = match4 + match3; i < match4 + match3 + match2;
         i++) { /* yellow crosses for pairs */
      for (j = 0; j < n_img; j++) {
        p1 = geo[j][con[i].p[j]].pnr;
        if (p1 > -1 && con[i].p[j] > -1 &&
            isinview(pix[j][p1].x, pix[j][p1].y, j)) {
          img_to_view_coordinates(&intx, &inty, pix[j][p1].x, pix[j][p1].y, j);
          drawcross(interp, intx, inty, cr_sz, j, "yellow");
          draw_pnr_autohide(interp, intx + 5, inty + 0, i, j, "white", 2,
                            zoompar[j].fac);
        }
      }
    }

    for (j = 0; j < n_img; j++) {
      for (i = 0; i < num[j]; i++) { /* blue crosses for unused detections */
        p1 = pix[j][i].tnr;
        if (p1 == -1 && isinview(pix[j][i].x, pix[j][i].y, j)) {
          img_to_view_coordinates(&intx, &inty, pix[j][i].x, pix[j][i].y, j);
          drawcross(interp, intx, inty, cr_sz, j, "blue");
        }
      }
    }
    // for (i=0; i<4; i++)
    //	 zoom_f[i] = zfac[i];
  }
  /* ----------------------------------------------------------------------- */
  /* free memory for lists of correspondences */
  for (i1 = 0; i1 < n_img - 1; i1++)
    for (i2 = i1 + 1; i2 < n_img; i2++)
      free(list[i1][i2]);

  free(con0);

  snprintf(buf, sizeof(buf), "Correspondences done");
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 2");
  Tcl_Eval(interp, ".text insert 2 $tbuf");
}
