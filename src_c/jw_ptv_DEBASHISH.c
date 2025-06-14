/****************************************************************************
*****************************************************************************

Author/Copyright:      	Hans-Gerd Maas / Jochen Willneff

Address:	      	Institute of Geodesy and Photogrammetry
                        ETH - Hoenggerberg
                        CH - 8093 Zurich

Creation Date:	       	took a longer time ...

Description:	       	target detection, correspondences and
                        positioning with tclTk display
                        -> 4 camera version

Routines contained:    	many ...

****************************************************************************/
#include "src_c/ptv.h"

#define nmax 20000

/*  global declarations for ptv  */
/*-------------------------------------------------------------------------*/

int n_img;                           /* no of images */
int hp_flag = 0;                     /* flag for highpass */
int tiff_flag = 0;                   /* flag for tiff header */
int chfield;                         /* flag for field mode */
int nfix;                            /* no. of control points */
int num[4];                          /* no. of targets per image */
int numc[4];                         /* no. of targets in current image */
int nump[4];                         /* no. of targets in previous image */
int numn[4];                         /* no. of targets in next image */
int n_trac[4];                       /* no. of tracks */
int match = 0;                       /* no. of matches */
int match2 = 0;                      /* no. of matches in 2nd pass */
int nr[4][4];                        /* point numbers for man. ori */
int imx, imy, imgsize;               /* image size */
int zoom_x[4], zoom_y[4], zoom_f[4]; /* zoom parameters */
int pp1 = 0, pp2 = 0, pp3 = 0, pp4 = 0, pp5 = 0; /* for man. orientation */
int seq_first, seq_last;                         /* 1. and last img of seq */
int max_shake_points, max_shake_frames, step_shake;
int demo_nr;          /* for demo purposes */
int examine = 0;      /* for more detailed output */
int dump_for_rdb;     /* # of dumpfiles for rdb */
int cr_sz;            /* size of crosses */
int display, shaking; /* display flag */
int corp, corc, corn;
int m[4];
int trackallocflag = 0; /* checkflag if mega, c4, t4 already allocated */

double pix_x, pix_y;                       /* pixel size */
double ro;                                 /* 200/pi */
double cn, cnx, cny, csumg, eps0, corrmin; /* correspondences par */
double rmsX, rmsY, rmsZ, mean_sigma0;      /* a priori rms */
double X_lay[2], Zmin_lay[2], Zmax_lay[2]; /* illu. layer data */

FILE *fp1, *fp2, *fp3, *fp4, *fpp;

char	img_name[4][512], img_lp_name[4][512], img_hp_name[4][512];
char	img_cal[4][512], img_ori[4][512], img_ori0[4][512], img_addpar[4][512];
char    img_addpar0[4][512]; // Added definition for img_addpar0
char	seq_name[4][512];
char	track_dir[512];
char	fixp_name[512], res_name[512], par_name[512], base_name[512];

char	cal_img_base_name[4][512];
char	tiff_img_name[4][512];
char	target_name[4][512];
char	linkage_name[4][512];
char	prio_name[512];
char	added_name[512];
char	cal_db_name[512];
char	cor_name[512];
char	seq_log_name[512];
char	track_log_name[512];
char	traj_name[512];
char	rtf_name[512];
char	sum_name[512];
char	connect_name[512];

char	multi_filename[MAX_PLANES][512];
char	filename_in[512], filename_out[512], textline[512];

unsigned char *img[4];  /* image data */
unsigned char *img0[4]; /* image data for filtering etc */
unsigned char *zoomimg; /* zoom image data */

Exterior Ex[4], sEx[4]; /* exterior orientation */
Interior I[4], sI[4];   /* interior orientation */
Glass G[4], sG[4];      /* glass orientation */
ap_52 ap[4], sap[4];    /* add. parameters k1,k2,k3,p1,p2,scx,she */
mm_np mmp;              /* n-media parameters */
target pix[4][nmax];    /* target pixel data */
target pix0[4][4];      /* pixel data for man_ori points */

target *t4[4][4];
int nt4[4][4];

coord_2d crd[4][nmax]; /* (distorted) metric coordinates */
coord_2d geo[4][nmax]; /* corrected metric coordinates */
coord_3d fix[4096];    /* testfield points coordinates */
n_tupel con[nmax];     /* list of correspondences */

corres *c4[4];
trackparameters tpar; /* tracking parameters */

mm_LUT mmLUT[4]; /* LUT for multimedia radial displacement */
coord_3d *p_c3d;
P *mega[4];

/***************************************************************************/

int init_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                const char **argv) {
  int i;
  const char *valp;

  puts("\nMultimedia Particle Positioning and Tracking\n");

  valp = Tcl_GetVar(interp, "examine", TCL_GLOBAL_ONLY);
  examine = atoi(valp);

  ro = 200 / M_PI;

  /*  read from main parameter file  */
  fpp = fopen_r("parameters/ptv.par");

  fscanf(fpp, "%d\\n", &n_img);

  for (i = 0; i < 4; i++) {
    fscanf(fpp, "%511s\\n", img_name[i]);
    fscanf(fpp, "%511s\\n", img_cal[i]);
  }
  fscanf(fpp, "%d\\n", &hp_flag);
  fscanf(fpp, "%d\\n", &tiff_flag);
  fscanf(fpp, "%d\\n", &imx);
  fscanf(fpp, "%d\\n", &imy);
  fscanf(fpp, "%lf\\n", &pix_x);
  fscanf(fpp, "%lf\\n", &pix_y);
  fscanf(fpp, "%d\\n", &chfield);
  fscanf(fpp, "%lf\\n", &mmp.n1);
  fscanf(fpp, "%lf\\n", &mmp.n2[0]);
  fscanf(fpp, "%lf\\n", &mmp.n3);
  fscanf(fpp, "%lf\\n", &mmp.d[0]);
  fclose(fpp);

  /* read illuminated layer data */
  fpp = fopen_r("parameters/criteria.par");
  fscanf(fpp, "%lf\\n", &X_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[0]);
  fscanf(fpp, "%lf\\n", &X_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[1]);
  fscanf(fpp, "%lf", &cnx);
  fscanf(fpp, "%lf", &cny);
  fscanf(fpp, "%lf", &cn);
  fscanf(fpp, "%lf", &csumg);
  fscanf(fpp, "%lf", &corrmin);
  fscanf(fpp, "%lf", &eps0);
  fclose(fpp);

  mmp.nlay = 1;

  /* read sequence parameters (needed for some demos) */

  fpp = fopen_r("parameters/sequence.par");

  for (i = 0; i < 4; i++)
    fscanf(fpp, "%511s\\n", seq_name[i]);
  fscanf(fpp, "%d\\n", &seq_first);
  fscanf(fpp, "%d\\n", &seq_last);
  fclose(fpp);

  /* initialize zoom parameters and image positions */
  for (i = 0; i < n_img; i++) {
    num[i] = 0;
    zoom_x[i] = imx / 2;
    zoom_y[i] = imy / 2;
    zoom_f[i] = 1;
  }
  imgsize = imx * imy;

  /* allocate memory for images */
  for (i = 0; i < n_img; i++) {
    img[i] = (unsigned char *)calloc(imgsize, 1);
    if (!img[i]) {
      printf("calloc for img%d --> error\n", i);
      exit(1);
    }
  }

  for (i = 0; i < n_img; i++) {
    img0[i] = (unsigned char *)calloc(imgsize, 1);
    if (!img0[i]) {
      printf("calloc for img0%d --> error\n", i);
      exit(1);
    }
  }

  zoomimg = (unsigned char *)calloc(imgsize, 1);
  if (!zoomimg) {
    printf("calloc for zoomimg --> error\n");
    return TCL_ERROR;
  }

  parameter_panel_init(interp);
  cr_sz = atoi(Tcl_GetVar2(interp, "mp", "pcrossize", TCL_GLOBAL_ONLY));

  display = 1;
  return TCL_OK;
}

int start_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                 const char **argv) {
  int i, k;
  char val[256];

  /*  read from main parameter file  */
  fpp = fopen_r("parameters/ptv.par");

  fscanf(fpp, "%d\\n", &n_img);

  for (i = 0; i < 4; i++) {
    fscanf(fpp, "%511s\\n", img_name[i]);
    fscanf(fpp, "%511s\\n", img_cal[i]);
  }
  fscanf(fpp, "%d\\n", &hp_flag);
  fscanf(fpp, "%d\\n", &tiff_flag);
  fscanf(fpp, "%d\\n", &imx);
  fscanf(fpp, "%d\\n", &imy);
  fscanf(fpp, "%lf\\n", &pix_x);
  fscanf(fpp, "%lf\\n", &pix_y);
  fscanf(fpp, "%d\\n", &chfield);
  fscanf(fpp, "%lf\\n", &mmp.n1);
  fscanf(fpp, "%lf\\n", &mmp.n2[0]);
  fscanf(fpp, "%lf\\n", &mmp.n3);
  fscanf(fpp, "%lf\\n", &mmp.d[0]);
  fclose(fpp);

  /* read illuminated layer data */
  fpp = fopen_r("parameters/criteria.par");
  fscanf(fpp, "%lf\\n", &X_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[0]);
  fscanf(fpp, "%lf\\n", &X_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[1]);
  fscanf(fpp, "%lf", &cnx);
  fscanf(fpp, "%lf", &cny);
  fscanf(fpp, "%lf", &cn);
  fscanf(fpp, "%lf", &csumg);
  fscanf(fpp, "%lf", &corrmin);
  fscanf(fpp, "%lf", &eps0);
  fclose(fpp);

  mmp.nlay = 1;

  /* read sequence parameters (needed for some demos) */

  fpp = fopen_r("parameters/sequence.par");

  for (i = 0; i < 4; i++)
    fscanf(fpp, "%511s\\n", seq_name[i]);
  fscanf(fpp, "%d\\n", &seq_first);
  fscanf(fpp, "%d\\n", &seq_last);
  fclose(fpp);

  /*  create file names  */
  for (i = 0; i < n_img; i++) {
    snprintf(img_lp_name[i], sizeof(img_lp_name[i]), "%s_lp", img_name[i]);
    snprintf(img_hp_name[i], sizeof(img_hp_name[i]), "%s_hp", img_name[i]);
    snprintf(img_ori[i], sizeof(img_ori[i]), "%s.ori", img_cal[i]);
    snprintf(img_addpar[i], sizeof(img_addpar[i]), "%s.addpar", img_cal[i]);
  }

  /*  read orientation and additional parameters  */
  for (i = 0; i < n_img; i++) {
    read_ori(&Ex[i], &I[i], &G[i], img_ori[i]);
    rotation_matrix(Ex[i], Ex[i].dm);

    fp1 = fopen_r(img_addpar[i]);
    fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i].k1, &ap[i].k2, &ap[i].k3,
           &ap[i].p1, &ap[i].p2, &ap[i].scx, &ap[i].she);
    fclose(fp1);
  }
  /* read and display original images */
  for (i = 0; i < n_img; i++) {
    /* reading */
    snprintf(val, sizeof(val), "camcanvas %d", i + 1);
    Tcl_Eval(interp, val);

    read_image(interp, img_name[i], img[i]);
    snprintf(val, sizeof(val), "newimage %d", i + 1);

    Tcl_Eval(interp, val);
    snprintf(val, sizeof(val), "keepori %d", i + 1);
    Tcl_Eval(interp, val);
  }

  if (!trackallocflag) {
    for (i = 0; i < 4; i++) {
      mega[i] = (P *)calloc(sizeof(P), M);
      c4[i] = (corres *)calloc(sizeof(corres), M);
      for (k = 0; k < 4; k++) {
        t4[i][k] = (target *)calloc(sizeof(target), M);
      }
    }
    trackallocflag = 1;
  }

  snprintf(res_name, sizeof(res_name), "res/rt_is.first");

  return TCL_OK;
}

int pre_processing_c(ClientData clientData, Tcl_Interp *interp, int argc,
                     const char **argv) {
  int i_img, sup;
  char val[256];

  Tk_PhotoHandle img_handle;
  Tk_PhotoImageBlock img_block;

  snprintf(val, sizeof(val), "Filtering with Highpass");
  Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 2");
  Tcl_Eval(interp, ".text insert 2 $tbuf");

  /* read support of unsharp mask */
  fpp = fopen("parameters/unsharp_mask.par", "r");
  if (fpp == 0) {
    sup = 12;
  } else {
    fscanf(fpp, "%d\\n", &sup);
    fclose(fpp);
  }

  for (i_img = 0; i_img < n_img; i_img++) {
    highpass(img_name[i_img], img[i_img], img[i_img], sup, 0, chfield, i_img);

    if (display) {
      img_handle = Tk_FindPhoto(interp, "temp");
      Tk_PhotoGetImage(img_handle, &img_block);
      tclimg2cimg(interp, img[i_img], &img_block);

      snprintf(val, sizeof(val), "newimage %d", i_img + 1);
      Tcl_GlobalEval(interp, val);
    }
  }

  snprintf(val, sizeof(val), "...done");
  Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 3");
  Tcl_Eval(interp, ".text insert 3 $tbuf");

  return TCL_OK;
}

int detection_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                     const char **argv) {
  int i, i_img;
  int xmin, pft_version = 3;
  char val[256];
  char buf[256];

  Tk_PhotoHandle img_handle;
  Tk_PhotoImageBlock img_block;

  /* process info */
  snprintf(val, sizeof(val), "Detection of Particles");
  Tcl_Eval(interp, ".text delete 2");
  Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text insert 2 $tbuf");

  if (display) {
    for (i_img = 0; i_img < n_img; i_img++) {
      img_handle = Tk_FindPhoto(interp, "temp");
      Tk_PhotoGetImage(img_handle, &img_block);
      tclimg2cimg(interp, img[i_img], &img_block);
      snprintf(val, sizeof(val), "newimage %d", i_img + 1);
      Tcl_Eval(interp, val);
    }
  }

  snprintf(val, sizeof(val), ""); // Replaces strcpy(val, "")

  /* xmin set to 10 so v_line is not included in detection, in future xmin
     should be set to 0, peakfitting has to be changed too */
  xmin = 0;

  /*  read pft version  */
  fpp = fopen("parameters/pft_version", "r");
  if (fpp) {
    fscanf(fpp, "%d\\n", &pft_version);
    fclose(fpp);
  }

  /* reset zoom values */
  for (i_img = 0; i_img < n_img; i_img++) {
    zoom_x[i_img] = imx / 2;
    zoom_y[i_img] = imy / 2;
    zoom_f[i_img] = 1;
  }

  /*copy images because the target recognition will set greyvalues to 0*/
  for (i_img = 0; i_img < n_img; i_img++) {
    copy_images(img[i_img], img0[i_img]);
  }

  /* target recognition */
  for (i_img = 0; i_img < n_img; i_img++) {
    switch (pft_version) {
    case 3: /* pft with profile and distance check */
      /* newest version */
      xmin = 0; /* vertical line restriction */
      num[i_img] = peak_fit_new(interp, img[i_img], "parameters/targ_rec.par",
                                xmin, imx, 1, imy, pix[i_img], i_img);
      break;

    case 0: /* without peak fitting technique */
      simple_connectivity(interp, img[i_img], img0[i_img],
                          "parameters/targ_rec.par", xmin, imx, 1, imy,
                          pix[i_img], i_img, &num[i_img]);
      break;

    case 1: /* with old (but fast) peak fitting technique */
      targ_rec(interp, img[i_img], img0[i_img], "parameters/targ_rec.par", xmin,
               imx, 1, imy, pix[i_img], i_img, &num[i_img]);
      break;
    }

    char temp_buf[256]; // Temporary buffer for constructing 'val'
    snprintf(temp_buf, sizeof(temp_buf), "%d: %d,  ", i_img + 1, num[i_img]);
    // Check if val has enough space before strcat, or use snprintf to append
    if (strlen(val) + strlen(temp_buf) < sizeof(val)) {
        strcat(val, temp_buf); // strcat is fine if space is checked, but snprintf is safer
    } else {
        // Handle potential overflow, e.g., by truncating or logging
        fprintf(stderr, "Warning: Buffer 'val' too small in detection_proc_c\\n");
    }


    /* proper sort of targets in y-direction for later binary search */
    /* and for dimitris' tracking */
    quicksort_target_y(pix[i_img], num[i_img]);

    /* reorganize target numbers */
    for (i = 0; i < num[i_img]; i++)
      pix[i_img][i].pnr = i;
  }

  snprintf(buf, sizeof(buf), "Number of detected particles per image");
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 2");
  Tcl_Eval(interp, ".text insert 2 $tbuf");

  Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 3");
  Tcl_Eval(interp, ".text insert 3 $tbuf");

  printf("%s\n", val);
  return TCL_OK;
}

int correspondences_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                           const char **argv) {
  int i, i_img;
  double x, y;
  char filename[512];

  puts("\\nTransformation to metric coordinates\\n");

  /* rearrange point numbers after manual deletion of points */
  for (i_img = 0; i_img < n_img; i_img++)
    for (i = 0; i < num[i_img]; i++)
      pix[i_img][i].pnr = i;
  /* transformations pixel coordinates -> metric coordinates */
  /* transformations metric coordinates -> corrected metric coordinates */
  for (i_img = 0; i_img < n_img; i_img++) {
    for (i = 0; i < num[i_img]; i++) {
      pixel_to_metric(pix[i_img][i].x, pix[i_img][i].y, imx, imy, pix_x, pix_y,
                      &crd[i_img][i].x, &crd[i_img][i].y, chfield);
      crd[i_img][i].pnr = pix[i_img][i].pnr;

      x = crd[i_img][i].x - I[i_img].xh;
      y = crd[i_img][i].y - I[i_img].yh;
      correct_brown_affin(x, y, ap[i_img], &geo[i_img][i].x, &geo[i_img][i].y);

      geo[i_img][i].pnr = crd[i_img][i].pnr;
    }
  }

  /* sort coordinates for binary search in correspondences_proc */
  for (i_img = 0; i_img < n_img; i_img++) {
    quicksort_coord2d_x(geo[i_img], num[i_img]);
  }

  /* init multimedia radial displacement LUTs */
  /* ======================================== */

  if (!mmp.lut && (mmp.n1 != 1 || mmp.n2[0] != 1 || mmp.n3 != 1)) {
    puts("Init multimedia displacement LUTs");
    for (i_img = 0; i_img < n_img; i_img++)
      init_mmLUT(i_img);
    mmp.lut = 1;
  }

  correspondences_4(interp, argv);

  /* --------------- */
  /* save pixel coords for tracking */
  for (i_img = 0; i_img < n_img; i_img++) {
    snprintf(filename, sizeof(filename), "%s_targets", img_name[i_img]);
    fp1 = fopen(filename, "w");

    fprintf(fp1, "%d\\n", num[i_img]);
    for (i = 0; i < num[i_img]; i++) {
      fprintf(fp1, "%4d %9.4f %9.4f %5d %5d %5d %5d %5d\n", pix[i_img][i].pnr,
              pix[i_img][i].x, pix[i_img][i].y, pix[i_img][i].n,
              pix[i_img][i].nx, pix[i_img][i].ny, pix[i_img][i].sumg,
              pix[i_img][i].tnr);
    }
    fclose(fp1);
  }

  return TCL_OK;
}

int determination_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                         const char **argv) {
  int i, j, n, dummy;
  int p[4];
  double x[4], y[4], X, Y, Z;
  double Zlo = 1e20, Zhi = -1e20;
  char buf[256];

  puts("Determinate");

  snprintf(buf, sizeof(buf), "Point positioning (l.sq.)");
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 2");
  Tcl_Eval(interp, ".text insert 2 $tbuf");

  /* Beat Mai 2007 to set the variable examine for mulit-plane calibration*/
  fp1 = fopen_r("parameters/examine.par");
  fscanf(fp1, "%d\\n", &dummy);
  fclose(fp1);
  if (dummy == 1) {
    examine = 4;
  } else {
    examine = 0;
  }
  //////////////////////////////////

  fp1 = fopen(res_name, "w");

  if (!fp1) {
    snprintf(res_name, sizeof(res_name), "res/dt_lsq");
    fp1 = fopen(res_name, "w");
  }
  if (!fp1) {
    printf("cannot find dir: res,  data written to dt_lsq in same dir\\n");
    snprintf(res_name, sizeof(res_name), "dt_lsq");
    fp1 = fopen(res_name, "w");
  }
  /* create dump file for rdb */
  if (examine == 4) {
    /* create filename for dumped dataset */
    snprintf(res_name, sizeof(res_name), "dump_for_rdb");
    printf("dataset dumped into %s\\n", res_name);
    fp2 = fopen(res_name, "w");

    /* write # of points to file */
    fprintf(fp2, "%d\n", match);
  }
  /* first line to be updated in res_name file */
  fprintf(fp1, "%4d\n", match);
  /* least squares determination for triplets */

  rmsX = 0;
  rmsY = 0;
  rmsZ = 0;
  mean_sigma0 = 0;

  for (i = 0; i < match; i++) {
    for (j = 0; j < 4; j++)
      if (con[i].p[j] >= 0)
        p[j] = geo[j][con[i].p[j]].pnr;
      else
        p[j] = -1;

    for (j = 0, n = 0; j < 4; j++) {
      if (p[j] > -1) {
        x[j] = crd[j][p[j]].x;
        y[j] = crd[j][p[j]].y;
        n++;
      } else {
        x[j] = -1e10;
        y[j] = -1e10;
        if (p[j] == -2)
          n = -100;
      }
    }

    /* take only points which are matched in all images */
    /* or triplets/quadruplets which result from object model */
    /* e.g.: quad -> n=4; model triplet -> n=3; model pair -> n=2;
       unrestricted triplet -> n<0; unrestricted pair -> n<0 */
    /*     if (n_img > 2  &&  n < 3)	continue; */

    /* ################################# */
    /* take only points which are matched in all images */
    /* or triplets/quadruplets which result from object model */
    /* e.g.: quad -> n=4; model triplet -> n=3; model pair -> n=2;
       unrestricted triplet -> n<0; unrestricted pair -> n<0 */
    if ((n_img > 2 && num[0] > 64 && num[1] > 64 && num[2] > 64 &&
         num[3] > 64) &&
        n < 3)
      continue;

    /* hack due to problems with approx in det_lsq: */
    X = 0.0;
    Y = 0.0;
    Z = (Zmin_lay[0] + Zmax_lay[0]) / 2.0;
    for (j = 0; j < n_img; j++) {
      X += Ex[j].x0;
      Y += Ex[j].y0;
    }
    X /= n_img;
    Y /= n_img;
    /* ******************************** */

    // det_lsq_old (Ex, I, ap, mmp,
    //     x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3], &X, &Y, &Z);

    det_lsq(Ex, I, G, ap, mmp, x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3],
            &X, &Y, &Z);

    /* write a sequential point number,
       sumg, if the point was used, and the 3D coordinates */
    fprintf(fp1, "%4d", i + 1);

    /*
    if (p[0] > -1)	fprintf (fp1, "  %4d", pix[0][p[0]].sumg);
    else			fprintf (fp1, "   %4d", -1);
    if (p[1] > -1)	fprintf (fp1, "  %4d", pix[1][p[1]].sumg);
    else			fprintf (fp1, "  %4d", -1);
    if (p[2] > -1)	fprintf (fp1, "  %4d", pix[2][p[2]].sumg);
    else			fprintf (fp1, "  %4d", -1);
    if (p[3] > -1)	fprintf (fp1, "  %4d", pix[3][p[3]].sumg);
    else			fprintf (fp1, "  %4d", -1);
    */

    fprintf(fp1, " %9.3f %9.3f %9.3f", X, Y, Z);
    if (p[0] > -1)
      fprintf(fp1, " %4d", pix[0][p[0]].pnr);
    else
      fprintf(fp1, " %4d", -1);
    if (p[1] > -1)
      fprintf(fp1, " %4d", pix[1][p[1]].pnr);
    else
      fprintf(fp1, " %4d", -1);
    if (p[2] > -1)
      fprintf(fp1, " %4d", pix[2][p[2]].pnr);
    else
      fprintf(fp1, " %4d", -1);
    if (p[3] > -1)
      fprintf(fp1, " %4d\n", pix[3][p[3]].pnr);
    else
      fprintf(fp1, " %4d\n", -1);

    /* write data as new points to dump for rdb */
    if (examine == 4) {
      fprintf(fp2, "%d %10.3f %10.3f %10.3f   %d    ", i, X, Y, Z, 3);
      for (j = 0; j < n_img; j++)
        if (x[j] != -1e10)
          fprintf(fp2, "%4d %8.5f %8.5f    ", i, x[j], y[j]);
        else
          fprintf(fp2, "%4d %8.5f %8.5f    ", -999, x[j], y[j]);
      fprintf(fp2, "\n");
      fclose(fp2);
    }

    if (Z < Zlo)
      Zlo = Z;
    if (Z > Zhi)
      Zhi = Z;
  }

  fclose(fp1);

  rmsX = sqrt(rmsX / match);
  rmsY = sqrt(rmsY / match);
  rmsZ = sqrt(rmsZ / match);
  mean_sigma0 = sqrt(mean_sigma0 / match);

  snprintf(buf, sizeof(buf),
           "Match: %d, => sigma0 = %4.2f micron, RMS = %5.3f/%5.3f/%5.3f mm",
           match, mean_sigma0 * 1000, rmsX, rmsY, rmsZ);
  puts(buf);
  Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
  Tcl_Eval(interp, ".text delete 3");
  Tcl_Eval(interp, ".text insert 3 $tbuf");

  /* sort coordinates for binary search in epi line segment drawing */
  for (i = 0; i < n_img; i++)
    quicksort_coord2d_x(geo[0], num[0]);

  puts("Determinate done\n");

  return TCL_OK;
}

int sequence_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                    const char **argv) {
  int i, j, ok, k, nslices = 19, slicepos = 0;
  char seq_ch[128], seq_name[4][128];
  char buf[512];
  Tk_PhotoHandle img_handle;
  Tk_PhotoImageBlock img_block;
  double slice_step;
  double slicethickness;
  double zdim, z_cen_slice[19];

  fpp = fopen_r("parameters/sequence.par");
  for (i = 0; i < 4; i++)
    fscanf(fpp, "%511s\\n", seq_name[i]); /* name of sequence */
  fscanf(fpp, "%d\\n", &seq_first);
  fscanf(fpp, "%d\\n", &seq_last);
  fclose(fpp);

  display = atoi(argv[1]);
  shaking = 0;
  if (display == 2) {
    display = 0;
    shaking = 1;
  }

  if (shaking) {
    fpp = fopen_r("parameters/shaking.par");
    fscanf(fpp, "%d\n", &seq_first);
    fscanf(fpp, "%d\n", &seq_last);
    fclose(fpp);
  }

  /* scanning ptv ************** */
  printf("\nObject volume is scanned in %d slices!\n", nslices);
  slicepos = 0;
  /* read illuminated Volume */
  fpp = fopen_r("parameters/criteria.par");
  fscanf(fpp, "%lf\\n", &X_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[0]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[0]);
  fscanf(fpp, "%lf\\n", &X_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmin_lay[1]);
  fscanf(fpp, "%lf\\n", &Zmax_lay[1]);
  fscanf(fpp, "%lf", &cnx);
  fscanf(fpp, "%lf", &cny);
  fscanf(fpp, "%lf", &cn);
  fscanf(fpp, "%lf", &csumg);
  fscanf(fpp, "%lf", &corrmin);
  fscanf(fpp, "%lf", &eps0);
  fclose(fpp);

  mmp.nlay = 1;

  zdim = Zmax_lay[0] - Zmin_lay[0];
  slice_step = zdim / nslices;
  slicethickness = 5.0;

  printf("\nzdim: %f, max: %f, min: %f, st: %f\n", zdim, Zmax_lay[0],
         Zmin_lay[0], slice_step);

  // for (j=0; j<nslices; j++)
  //	{
  //	  z_cen_slice[j]=Zmax_lay[0]-j*slice_step;

  //	}

  /* ************** */

  for (i = seq_first; i < seq_last + 1; i++) {

    // printf("\nstep: %d, zslice[j]: %f, slicepos: %d\n", i);

    //	  Zmax_lay[0]= z_cen_slice[slicepos] - slicethickness/2.0;
    //	  Zmin_lay[0]= z_cen_slice[slicepos] + slicethickness/2.0;
    //	  Zmax_lay[1]= z_cen_slice[slicepos] - slicethickness/2.0;
    //	  Zmin_lay[1]= z_cen_slice[slicepos] + slicethickness/2.0;
    // printf("in sequence zslice[j]: %f, zmin0: %f, zmax0: %f\n",
    // z_cen_slice[slicepos], Zmax_lay[0],Zmin_lay[0] );

    // slicepos++; if (slicepos==nslices) {slicepos=0;}

    if (i < 10)
      snprintf(seq_ch, sizeof(seq_ch), "%1d", i);
    else if (i < 100)
      snprintf(seq_ch, sizeof(seq_ch), "%2d", i);
    else
      snprintf(seq_ch, sizeof(seq_ch), "%3d", i);

    for (j = 0; j < n_img; j++) {
      snprintf(img_name[j], sizeof(img_name[j]), "%s%s", seq_name[j], seq_ch);
      snprintf(img_lp_name[j], sizeof(img_lp_name[j]), "%s%s_lp", seq_name[j], seq_ch);
      snprintf(img_hp_name[j], sizeof(img_hp_name[j]), "%s%s_hp", seq_name[j], seq_ch);
    }

    if (chfield == 0)
      snprintf(res_name, sizeof(res_name), "res/rt_is.%s", seq_ch);
    else
      snprintf(res_name, sizeof(res_name), "res/rt_is.%s_%1d", seq_ch, chfield);

    snprintf(buf, sizeof(buf), "\\nImages:");
    for (j = 0; j < n_img; j++) {
      size_t len = strlen(buf);
      snprintf(buf + len, sizeof(buf) - len, "  %s", img_name[j]);
    }
    puts(buf);

    /* calling function for each sequence-n-tupel */
    /* read and display original images */

    for (k = 0; k < n_img; k++) {
      /* reading */
      read_image(interp, img_name[k], img[k]);

      if (display) {
        img_handle = Tk_FindPhoto(interp, "temp");
        Tk_PhotoGetImage(img_handle, &img_block);
        tclimg2cimg(interp, img[k], &img_block);
        snprintf(buf, sizeof(buf), "newimage %d", k + 1);
        Tcl_Eval(interp, buf);
      }
    }

    if (hp_flag) { // 1>2
      pre_processing_c(clientData, interp, argc, argv);
      puts("\nHighpass switched on\n");
    } else {
      puts("\nHighpass switched off\n");
    }
    if (display) {
      Tcl_Eval(interp, "update idletasks");
    }
    detection_proc_c(clientData, interp, argc, argv);
    if (display) {
      Tcl_Eval(interp, "update idletasks");
    }
    correspondences_proc_c(clientData, interp, argc, argv);
    if (display) {
      Tcl_Eval(interp, "update idletasks");
    }
    if (n_img > 1) {
      determination_proc_c(clientData, interp, argc, argv);
    }

    /* delete unneeded files */

    for (j = 0; j < n_img; j++) {
      ok = remove(img_lp_name[j]);
      ok = remove(img_hp_name[j]);
    }
  }
  /* reset of display flag */
  display = 1;

  return TCL_OK;
}

int restore_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                   const char **argv) {
  int i_img, i;
  char safety[4][512];
  char safety_addpar[4][512];

  ///////////////////////////////////////////
  fpp = fopen_r("parameters/ptv.par");

  fscanf(fpp, "%d\\n", &n_img);

  for (i = 0; i < n_img; i++) {
    fscanf(fpp, "%511s\\n", img_name[i]);
    fscanf(fpp, "%511s\\n", img_cal[i]);
  }

  fclose(fpp);
  /*  create file names  */
  for (i = 0; i < n_img; i++) {
    snprintf(img_ori[i], sizeof(img_ori[i]), "%s.ori", img_cal[i]);
    snprintf(img_addpar[i], sizeof(img_addpar[i]), "%s.addpar", img_cal[i]);
  }
  ///////////////////////////////////////////

  snprintf(safety[0], sizeof(safety[0]), "safety_0.ori");
  snprintf(safety[1], sizeof(safety[1]), "safety_1.ori");
  snprintf(safety[2], sizeof(safety[2]), "safety_2.ori");
  snprintf(safety[3], sizeof(safety[3]), "safety_3.ori");
  snprintf(safety_addpar[0], sizeof(safety_addpar[0]), "safety_0.addpar");
  snprintf(safety_addpar[1], sizeof(safety_addpar[1]), "safety_1.addpar");
  snprintf(safety_addpar[2], sizeof(safety_addpar[2]), "safety_2.addpar");
  snprintf(safety_addpar[3], sizeof(safety_addpar[3]), "safety_3.addpar");

  for (i_img = 0; i_img < n_img; i_img++) {
    read_ori(&Ex[i_img], &I[i_img], &G[i_img], safety[i_img]);
    fp1 = fopen(safety_addpar[i_img], "r");
    if (!fp1) {
      fp1 = fopen("addpar.raw", "r");
    }

    if (fp1) {
      fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i_img].k1, &ap[i_img].k2,
             &ap[i_img].k3, &ap[i_img].p1, &ap[i_img].p2, &ap[i_img].scx,
             &ap[i_img].she);
      fclose(fp1);
    } else {
      printf("no addpar.raw\n");
      ap[i_img].k1 = ap[i_img].k2 = ap[i_img].k3 = ap[i_img].p1 = ap[i_img].p2 =
          ap[i_img].she = 0.0;
      ap[i_img].scx = 1.0;
    }

    write_ori(Ex[i_img], I[i_img], G[i_img], img_ori[i_img]);
    fp1 = fopen(img_addpar[i_img], "w");
    fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
            ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
            ap[i_img].she);
    fclose(fp1);
  }
  puts("\nOrientation files restored");

  return TCL_OK;
}

int calibration_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                       const char **argv) {
  int i, j, sel, i_img, k, n, sup, dummy, multi, planes;
  int prev, next;
  double dummy_float;
  int intx1, inty1, intx2, inty2;
  coord_2d apfig1[11][11]; /* regular grid for ap figures */
  coord_2d apfig2[11][11]; /* ap figures */
  coord_3d fix4[4];        /* object points for preorientation */
  coord_2d crd0[4][4];     /* image points for preorientation */
  char multi_filename[10][256], filename[256], val[256], buf[512];
  char safety[4][512], safety_addpar[4][512];
  const char *valp;

  FILE *FILEIN;
  char filein[256];
  FILE *FILEIN_ptv;
  char filein_ptv[256];
  FILE *FILEIN_T;
  char filein_T[256];
  int filenumber;
  int dummy, frameCount, currentFrame;
  int a[4];

  Tk_PhotoHandle img_handle;
  Tk_PhotoImageBlock img_block;

  /* read support of unsharp mask */
  fp1 = fopen("parameters/unsharp_mask.par", "r");
  if (!fp1) {
    sup = 12;
  } else {
    fscanf(fp1, "%d\\n", &sup);
    fclose(fp1);
  }

  /* Get Selection value from TclTk */

  valp = Tcl_GetVar(interp, "sel", TCL_GLOBAL_ONLY);
  sel = atoi(valp);

  /* Beat Mai 2007 to set the variable examine for mulit-plane calibration*/
  fp1 = fopen_r("parameters/examine.par");
  fscanf(fp1, "%d\\n", &dummy);
  fscanf(fp1, "%d\\n", &multi);
  fclose(fp1);
  if (dummy == 1) {
    examine = 4;
  } else {
    examine = 0;
  }
  ///////////////////////////////////////////////////////////////////////////////

  switch (sel) {
  case 1: /*  read calibration parameter file  */
    fp1 = fopen_r("parameters/cal_ori.par");
    fscanf(fp1, "%511s\\n", fixp_name);
    for (i = 0; i < 4; i++) {
      fscanf(fp1, "%511s\\n", img_name[i]);
      fscanf(fp1, "%511s\\n", img_ori0[i]);
    }
    fscanf(fpp, "%d\\n", &tiff_flag);
    fscanf(fp1, "%d\\n", &chfield);
    fclose(fp1);

    /*  create file names  */
    for (i = 0; i < n_img; i++) {
      snprintf(img_ori[i], sizeof(img_ori[i]), "%s.ori", img_name[i]);
      snprintf(img_addpar0[i], sizeof(img_addpar0[i]), "%s.addpar0", img_name[i]);
      snprintf(img_addpar[i], sizeof(img_addpar[i]), "%s.addpar", img_name[i]);
      snprintf(img_hp_name[i], sizeof(img_hp_name[i]), "%s_hp", img_name[i]);
    }
    snprintf(safety[0], sizeof(safety[0]), "safety_0.ori");
    snprintf(safety[1], sizeof(safety[1]), "safety_1.ori");
    snprintf(safety[2], sizeof(safety[2]), "safety_2.ori");
    snprintf(safety[3], sizeof(safety[3]), "safety_3.ori");
    snprintf(safety_addpar[0], sizeof(safety_addpar[0]), "safety_0.addpar");
    snprintf(safety_addpar[1], sizeof(safety_addpar[1]), "safety_1.addpar");
    snprintf(safety_addpar[2], sizeof(safety_addpar[2]), "safety_2.addpar");
    snprintf(safety_addpar[3], sizeof(safety_addpar[3]), "safety_3.addpar");

    for (i = 0; i < n_img; i++) {

      zoom_x[i] = imx / 2, zoom_y[i] = imy / 2, zoom_f[i] = 1;

      read_image(interp, img_name[i], img[i]);

      snprintf(val, sizeof(val), "camcanvas %d", i + 1);
      Tcl_Eval(interp, val);

      img_handle = Tk_FindPhoto(interp, "temp");
      Tk_PhotoGetImage(img_handle, &img_block);
      tclimg2cimg(interp, img[i], &img_block);

      snprintf(val, sizeof(val), "newimage %d", i + 1);
      Tcl_Eval(interp, val);
    }

    break;

  case 2:
    puts("Detection procedure");
    snprintf(val, sizeof(val), ""); // Replaces strcpy(val, "")

    /* Highpass Filtering */
    pre_processing_c(clientData, interp, argc, argv);

    /* reset zoom values */
    for (i = 0; i < n_img; i++) {
      zoom_x[i] = imx / 2;
      zoom_y[i] = imy / 2;
      zoom_f[i] = 1;
    }

    /* copy images because the target recognition
        will set greyvalues to zero */

    for (i = 0; i < n_img; i++) {
      copy_images(img[i], img0[i]);
    }

    /* target recognition */
    for (i = 0; i < n_img; i++) {
      targ_rec(interp, img[i], img0[i], "parameters/detect_plate.par", 0, imx,
               1, imy, pix[i], i, &num[i]);

      //snprintf(buf, sizeof(buf), "image %d: %d,  ", i + 1, num[i]);
      //strcat(val, buf); // Potential overflow if val is not large enough
      char temp_buf_case2[256];
      snprintf(temp_buf_case2, sizeof(temp_buf_case2), "image %d: %d,  ", i + 1, num[i]);
      if (strlen(val) + strlen(temp_buf_case2) < sizeof(val)) {
          strcat(val, temp_buf_case2);
      } else {
          fprintf(stderr, "Warning: Buffer 'val' too small in calibration_proc_c case 2\\n");
      }


      if (num[i] > nmax)
        exit(1);
    }

    /* save pixel coord as approx. for template matching */
    if (examine)
      for (i = 0; i < n_img; i++) {
        snprintf(filename, sizeof(filename), "%s_pix", img_name[i]);
        fp1 = fopen(filename, "w");
        for (j = 0; j < num[i]; j++)
          fprintf(fp1, "%4d  %8.3f  %8.3f\\n", pix[i][j].pnr, pix[i][j].x,
                  pix[i][j].y);

        fclose(fp1);
      }

    snprintf(buf, sizeof(buf), "Number of detected targets, interaction enabled");
    Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text delete 2");
    Tcl_Eval(interp, ".text insert 2 $tbuf");
    Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text delete 3");
    Tcl_Eval(interp, ".text insert 3 $tbuf");
    break;

  case 3:
    pp1 = 0;
    pp2 = 0;
    pp3 = 0;
    pp4 = 0;

    for (i = 0; i < n_img; i++) {
      //snprintf(buf, sizeof(buf), "%d targets remain", num[i]);
      //puts(buf);
      printf("%d targets remain\\n", num[i]); // Simpler and safer
    }
    fp1 = fopen_r("parameters/man_ori.par");
    for (i = 0; i < n_img; i++) {
      fscanf(fp1, "%d %d %d %d \n", &nr[i][0], &nr[i][1], &nr[i][2], &nr[i][3]);
    }
    fclose(fp1);

    for (i = 0; i < n_img; i++) {
      snprintf(val, sizeof(val), "measure %d %d %d %d %d", nr[i][0], nr[i][1], nr[i][2],
              nr[i][3], i + 1);
      Tcl_Eval(interp, val);
      valp = Tcl_GetVar(interp, "px0", TCL_GLOBAL_ONLY);
      pix0[i][0].x = atoi(valp);
      valp = Tcl_GetVar(interp, "py0", TCL_GLOBAL_ONLY);
      pix0[i][0].y = atoi(valp);
      valp = Tcl_GetVar(interp, "px1", TCL_GLOBAL_ONLY);
      pix0[i][1].x = atoi(valp);
      valp = Tcl_GetVar(interp, "py1", TCL_GLOBAL_ONLY);
      pix0[i][1].y = atoi(valp);
      valp = Tcl_GetVar(interp, "px2", TCL_GLOBAL_ONLY);
      pix0[i][2].x = atoi(valp);
      valp = Tcl_GetVar(interp, "py2", TCL_GLOBAL_ONLY);
      pix0[i][2].y = atoi(valp);
      valp = Tcl_GetVar(interp, "px3", TCL_GLOBAL_ONLY);
      pix0[i][3].x = atoi(valp);
      valp = Tcl_GetVar(interp, "py3", TCL_GLOBAL_ONLY);
      pix0[i][3].y = atoi(valp);
    }

    /* write measured coordinates to file for next trial */
    fp1 = fopen("man_ori.dat", "w");
    for (i = 0; i < n_img; i++)
      for (j = 0; j < 4; j++)
        fprintf(fp1, "%f %f\n", pix0[i][j].x, pix0[i][j].y);
    fclose(fp1);

    break;

  case 4: /* read pixel coordinates of older pre-orientation */

    /* read point numbers of pre-clicked points */
    fp1 = fopen_r("parameters/man_ori.par");
    for (i = 0; i < n_img; i++) {
      fscanf(fp1, "%d %d %d %d \n", &nr[i][0], &nr[i][1], &nr[i][2], &nr[i][3]);
    }
    fclose(fp1);

    /* read coordinates of pre-clicked points */
    fp1 = fopen("man_ori.dat", "r");
    if (!fp1)
      break;
    for (i_img = 0; i_img < n_img; i_img++)
      for (i = 0; i < 4; i++) {
        fscanf(fp1, "%lf %lf\n", &pix0[i_img][i].x, &pix0[i_img][i].y);
        drawcross(interp, (int)pix0[i_img][i].x, (int)pix0[i_img][i].y,
                  cr_sz + 2, i_img, "red");
        draw_pnr(interp, (int)pix0[i_img][i].x, (int)pix0[i_img][i].y,
                 nr[i_img][i], i_img, "red");
      }
    fclose(fp1);

    break;

  case 5:
    puts("Sort grid points");
    for (i = 0; i < n_img; i++) {
      /* read control point coordinates for man_ori points */
      fp1 = fopen_r(fixp_name);
      k = 0;
      while (fscanf(fp1, "%d %lf %lf %lf", &fix[k].pnr, &fix[k].x, &fix[k].y,
                    &fix[k].z) != EOF) // No string buffer here, so no width specifier needed for strings
        k++;
      fclose(fp1);
      nfix = k;

      /* take clicked points from control point data set */
      for (j = 0; j < 4; j++)
        for (k = 0; k < nfix; k++) {
          if (fix[k].pnr == nr[i][j])
            fix4[j] = fix[k];
        }

      /* get approx for orientation and ap */
      read_ori(&Ex[i], &I[i], &G[i], img_ori0[i]);
      fp1 = fopen(img_addpar0[i], "r");
      if (!fp1)
        fp1 = fopen("addpar.raw", "r");

      if (fp1) {
        fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i].k1, &ap[i].k2,
               &ap[i].k3, &ap[i].p1, &ap[i].p2, &ap[i].scx, &ap[i].she);
        fclose(fp1);
      } else {
        printf("no addpar.raw\n");
        ap[i].k1 = ap[i].k2 = ap[i].k3 = ap[i].p1 = ap[i].p2 = ap[i].she = 0.0;
        ap[i].scx = 1.0;
      }

      /* transform clicked points */
      for (j = 0; j < 4; j++) {
        pixel_to_metric(pix0[i][j].x, pix0[i][j].y, imx, imy, pix_x, pix_y,
                        &crd0[i][j].x, &crd0[i][j].y, chfield);
        correct_brown_affin(crd0[i][j].x, crd0[i][j].y, ap[i], &crd0[i][j].x,
                            &crd0[i][j].y);
      }

      /* raw orientation with 4 points */
      raw_orient_v3(Ex[i], I[i], G[i], ap[i], mmp, 4, fix4, crd0[i], &Ex[i],
                    &G[i], 0);
      snprintf(filename, sizeof(filename), "raw%d.ori", i);
      write_ori(Ex[i], I[i], G[i], filename);

      /* sorting of detected points by back-projection */
      sortgrid_man(interp, Ex[i], I[i], G[i], ap[i], mmp, imx, imy, pix_x,
                   pix_y, nfix, fix, num[i], pix[i], chfield, i);

      /* adapt # of detected points */
      num[i] = nfix;

      for (j = 0; j < nfix; j++) {
        if (pix[i][j].pnr < 0)
          continue;
        intx1 = (int)pix[i][j].x;
        inty1 = (int)pix[i][j].y;

        drawcross(interp, intx1, inty1, cr_sz, i, "white");
        draw_pnr(interp, intx1, inty1, fix[j].pnr, i, "white");
      }
    }

    /* dump dataset for rdb */
    if (examine == 4) {
      /* create filename for dumped dataset */
      snprintf(filename, sizeof(filename), "dump_for_rdb");
      fp1 = fopen(filename, "w");

      /* write # of points to file */
      fprintf(fp1, "%d\n", nfix);

      /* write point and image coord to file */
      for (i = 0; i < nfix; i++) {
        fprintf(fp1, "%4d %10.3f %10.3f %10.3f   %d    ", fix[i].pnr, fix[i].x,
                fix[i].y, fix[i].z, 0);
        for (i_img = 0; i_img < n_img; i_img++) {
          if (pix[i_img][i].pnr >= 0) {
            /* transform pixel coord to metric */
            pixel_to_metric(pix[i_img][i].x, pix[i_img][i].y, imx, imy, pix_x,
                            pix_y, &crd[i_img][i].x, &crd[i_img][i].y, chfield);
            fprintf(fp1, "%4d %8.5f %8.5f    ", pix[i_img][i].pnr,
                    crd[i_img][i].x, crd[i_img][i].y);
          } else {
            fprintf(fp1, "%4d %8.5f %8.5f    ", pix[i_img][i].pnr, 0.0, 0.0);
          }
        }
        fprintf(fp1, "\n");
      }
      fclose(fp1);
      printf("dataset dumped into %s\n", filename);
    }
    break;

  case 6:
    puts("Orientation");
    //strcpy(buf, "");
    snprintf(buf, sizeof(buf), "");


    snprintf(safety[0], sizeof(safety[0]), "safety_0.ori");
    snprintf(safety[1], sizeof(safety[1]), "safety_1.ori");
    snprintf(safety[2], sizeof(safety[2]), "safety_2.ori");
    snprintf(safety[3], sizeof(safety[3]), "safety_3.ori");
    snprintf(safety_addpar[0], sizeof(safety_addpar[0]), "safety_0.addpar");
    snprintf(safety_addpar[1], sizeof(safety_addpar[1]), "safety_1.addpar");
    snprintf(safety_addpar[2], sizeof(safety_addpar[2]), "safety_2.addpar");
    snprintf(safety_addpar[3], sizeof(safety_addpar[3]), "safety_3.addpar");

    for (i_img = 0; i_img < n_img; i_img++) {
      for (i = 0; i < nfix; i++) {
        pixel_to_metric(pix[i_img][i].x, pix[i_img][i].y, imx, imy, pix_x,
                        pix_y, &crd[i_img][i].x, &crd[i_img][i].y, chfield);
        crd[i_img][i].pnr = pix[i_img][i].pnr;
      }

      /* save data for special use of resection routine */
      if (examine == 4 && multi == 0) {
        printf("try write resection data to disk\\n");
        /* point coordinates */
        // sprintf (filename, "resect_%s.fix", img_name[i_img]);
        snprintf(filename, sizeof(filename), "%s.fix", img_name[i_img]);
        write_ori(Ex[i_img], I[i_img], G[i_img], img_ori[i_img]);
        fp1 = fopen(filename, "w");
        for (i = 0; i < nfix; i++)
          fprintf(fp1, "%3d  %10.5f  %10.5f  %10.5f\\n", fix[i].pnr, fix[i].x,
                  fix[i].y, fix[i].z);
        fclose(fp1);

        /* metric image coordinates */
        // sprintf (filename, "resect_%s.crd", img_name[i_img]);
        snprintf(filename, sizeof(filename), "%s.crd", img_name[i_img]);
        fp1 = fopen(filename, "w");
        for (i = 0; i < nfix; i++)
          fprintf(fp1, "%3d  %9.5f  %9.5f\\n", crd[i_img][i].pnr,
                  crd[i_img][i].x, crd[i_img][i].y);
        fclose(fp1);

        /* orientation and calibration approx data */
        write_ori(Ex[i_img], I[i_img], G[i_img], "resect.ori0");
        fp1 = fopen("resect.ap0", "w");
        fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
                ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
                ap[i_img].she);
        fclose(fp1);
        printf("resection data written to disk\n");
      }

      /* resection routine */
      /* ================= */

      if (examine != 4)
        orient_v3(interp, Ex[i_img], I[i_img], G[i_img], ap[i_img], mmp, nfix,
                  fix, crd[i_img], &Ex[i_img], &I[i_img], &G[i_img], &ap[i_img],
                  i_img);

      /* ================= */

      /* resection with dumped datasets */
      if (examine == 4) {

        // printf("Resection with dumped datasets? (y/n)");
        // scanf("%s",buf);
        // if (buf[0] != 'y')	continue;
        // strcpy (buf, "");
        if (multi == 0)
          continue;

        /* read calibration frame datasets */
        // sprintf (multi_filename[0],"img/calib_a_cam");
        // sprintf (multi_filename[1],"img/calib_b_cam");

        fp1 = fopen_r("parameters/multi_planes.par");
        fscanf(fp1, "%d\\n", &planes);
        for (i = 0; i < planes; i++)
          fscanf(fp1, "%255s\\n", multi_filename[i]); // Assuming multi_filename[i] is char[256]
        fclose(fp1);

        for (n = 0, nfix = 0, dump_for_rdb = 0; n < 10; n++) {
          // sprintf (filename, "resect.fix%d", n);

          snprintf(filename, sizeof(filename), "%s%d.tif.fix", multi_filename[n], i_img + 1);
          fp1 = fopen(filename, "r");
          if (!fp1)
            continue;

          printf("reading file: %s\n", filename);
          printf("reading dumped resect data #%d\n", n);
          k = 0;
          while (fscanf(fp1, "%d %lf %lf %lf", &fix[nfix + k].pnr,
                        &fix[nfix + k].x, &fix[nfix + k].y,
                        &fix[nfix + k].z) != EOF)
            k++;
          fclose(fp1);
          /* read metric image coordinates */
          // sprintf (filename, "resect_%d.crd%d", i_img, n);
          snprintf(filename, sizeof(filename), "%s%d.tif.crd", multi_filename[n], i_img + 1);
          printf("reading file: %s\n", filename);
          fp1 = fopen(filename, "r");
          for (i = nfix; i < nfix + k; i++)
            fscanf(fp1, "%d %lf %lf", &crd[i_img][i].pnr, &crd[i_img][i].x,
                   &crd[i_img][i].y);
          nfix += k;
          fclose(fp1);
        }

        /* resection */
        /*Beat Mai 2007*/
        snprintf(filename, sizeof(filename), "raw%d.ori", i_img);
        read_ori(&Ex[i_img], &I[i_img], &G[i_img], filename);
        fp1 = fopen("addpar.raw", "r");

        if (fp1) {
          fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i_img].k1,
                 &ap[i_img].k2, &ap[i_img].k3, &ap[i_img].p1, &ap[i_img].p2,
                 &ap[i_img].scx, &ap[i_img].she);
          fclose(fp1);
        } else {
          printf("no addpar.raw\n");
          ap[i].k1 = ap[i].k2 = ap[i].k3 = ap[i].p1 = ap[i].p2 = ap[i].she =
              0.0;
          ap[i].scx = 1.0;
        }
        ////////////////////////////////////////

        /* markus 14.05.2007 show coordinates combined */

        for (i = 0; i < nfix; i++) {
          /* first crd->pix */
          metric_to_pixel(crd[i_img][i].x, crd[i_img][i].y, imx, imy, pix_x,
                          pix_y, &pix[i_img][i].x, &pix[i_img][i].y, chfield);
          /*then draw crosses*/
          // intx1 = (int) pix[i_img][i].x;
          // inty1 = (int) pix[i_img][i].y;

          // drawcross (interp, intx1, inty1, 3, i_img, "orange");
        }

        orient_v3(interp, Ex[i_img], I[i_img], G[i_img], ap[i_img], mmp, nfix,
                  fix, crd[i_img], &Ex[i_img], &I[i_img], &G[i_img], &ap[i_img],
                  i_img);

        ///////////////////////////////////////////
      }

      /* save orientation and additional parameters */
      fp1 = fopen(img_ori[i_img], "r");
      if (fp1 != NULL) {
        fclose(fp1);
        read_ori(&sEx[i_img], &sI[i_img], &sG[i_img], img_ori[i_img]);
        fp1 = fopen(img_addpar0[i_img], "r");
        if (!fp1)
          fp1 = fopen("addpar.raw", "r");

        if (fp1) {
          fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &sap[i_img].k1,
                 &sap[i_img].k2, &sap[i_img].k3, &sap[i_img].p1, &sap[i_img].p2,
                 &sap[i_img].scx, &sap[i_img].she);
          fclose(fp1);
        } else {
          printf("no addpar.raw\n");
          sap[i_img].k1 = sap[i_img].k2 = sap[i_img].k3 = sap[i_img].p1 =
              sap[i_img].p2 = sap[i_img].she = 0.0;
          sap[i_img].scx = 1.0;
        }

        write_ori(sEx[i_img], sI[i_img], sG[i_img], safety[i_img]);
        fp1 = fopen(safety_addpar[i_img], "w");
        fprintf(fp1, "%f %f %f %f %f %f %f", sap[i_img].k1, sap[i_img].k2,
                sap[i_img].k3, sap[i_img].p1, sap[i_img].p2, sap[i_img].scx,
                sap[i_img].she);
        fclose(fp1);
      } else {
        write_ori(Ex[i_img], I[i_img], G[i_img], safety[i_img]);
        fp1 = fopen(safety_addpar[i_img], "w");
        fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
                ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
                ap[i_img].she);
        fclose(fp1);
      }
      write_ori(Ex[i_img], I[i_img], G[i_img], img_ori[i_img]);
      fp1 = fopen(img_addpar[i_img], "w");
      fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
              ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
              ap[i_img].she);
      fclose(fp1);
    }
    Tcl_Eval(interp, ".text delete 3");
    Tcl_Eval(interp, ".text delete 1");
    Tcl_Eval(interp, ".text insert 1 \"Orientation and self calibration \"");
    Tcl_Eval(interp, ".text delete 2");
    if (examine != 4)
      Tcl_Eval(interp, ".text insert 2 \"...done, sigma0 for each image -> \"");
    if (examine == 4 && multi == 0)
      Tcl_Eval(interp, ".text insert 2 \"resection data written to disk \"");
    Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text insert 3 $tbuf");

    break;

  case 7:
    checkpoint_proc(interp);
    snprintf(val, sizeof(val), "blue: planimetry,   yellow: height");
    Tcl_SetVar(interp, "tbuf", val, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text delete 2");
    Tcl_Eval(interp, ".text insert 2 $tbuf");
    Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text delete 3");
    Tcl_Eval(interp, ".text insert 3 $tbuf");
    break;

  case 8: /* draw additional parameter figures */

    Tcl_Eval(interp, "clearcam");

    /*  read orientation and additional parameters  */
    for (i = 0; i < n_img; i++)
      read_ori(&Ex[i], &I[i], &G[i], img_ori[i]);
    for (i = 0; i < n_img; i++) {
      fp1 = fopen_r(img_addpar[i]);
      fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i].k1, &ap[i].k2,
             &ap[i].k3, &ap[i].p1, &ap[i].p2, &ap[i].scx, &ap[i].she);
      fclose(fp1);
    }
    for (i_img = 0; i_img < n_img; i_img++) {
      /* create undistorted grid */
      for (i = 0; i < 11; i++)
        for (j = 0; j < 11; j++) {
          apfig1[i][j].x = i * imx / 10;
          apfig1[i][j].y = j * imy / 10;
        }
      /* draw undistorted grid */
      for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) {
          intx1 = (int)apfig1[i][j].x;
          inty1 = (int)apfig1[i][j].y;
          intx2 = (int)apfig1[i + 1][j].x;
          inty2 = (int)apfig1[i][j + 1].y;
          drawvector(interp, intx1, inty1, intx2, inty1, 1, i_img, "black");
          drawvector(interp, intx1, inty1, intx1, inty2, 1, i_img, "black");
        }
      for (j = 0; j < 10; j++) {
        intx1 = (int)apfig1[10][j].x;
        inty1 = (int)apfig1[10][j].y;
        inty2 = (int)apfig1[10][j + 1].y;
        drawvector(interp, intx1, inty1, intx1, inty2, 1, i_img, "black");
      }
      for (i = 0; i < 10; i++) {
        intx1 = (int)apfig1[i][10].x;
        inty1 = (int)apfig1[i][10].y;
        intx2 = (int)apfig1[i + 1][10].x;
        drawvector(interp, intx1, inty1, intx2, inty1, 1, i_img, "black");
      }
      /* distort grid */
      for (i = 0; i < 11; i++)
        for (j = 0; j < 11; j++) {
          /* transform to metric, distort and re-transform */
          pixel_to_metric(apfig1[i][j].x, apfig1[i][j].y, imx, imy, pix_x,
                          pix_y, &apfig2[i][j].x, &apfig2[i][j].y, chfield);
          distort_brown_affin(apfig2[i][j].x, apfig2[i][j].y, ap[i_img],
                              &apfig2[i][j].x, &apfig2[i][j].y);
          metric_to_pixel(apfig2[i][j].x, apfig2[i][j].y, imx, imy, pix_x,
                          pix_y, &apfig2[i][j].x, &apfig2[i][j].y, chfield);
          /* exaggerate distortion by factor 5 */
          apfig2[i][j].x = 5 * apfig2[i][j].x - 4 * apfig1[i][j].x;
          apfig2[i][j].y = 5 * apfig2[i][j].y - 4 * apfig1[i][j].y;
        }
      /* draw distorted grid */
      for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) {
          intx1 = (int)apfig2[i][j].x;
          inty1 = (int)apfig2[i][j].y;
          intx2 = (int)apfig2[i + 1][j].x;
          inty2 = (int)apfig2[i + 1][j].y;
          drawvector(interp, intx1, inty1, intx2, inty2, 3, i_img, "magenta");
          intx2 = (int)apfig2[i][j + 1].x;
          inty2 = (int)apfig2[i][j + 1].y;
          drawvector(interp, intx1, inty1, intx2, inty2, 3, i_img, "magenta");
        }
      for (j = 0; j < 10; j++) {
        intx1 = (int)apfig2[10][j].x;
        inty1 = (int)apfig2[10][j].y;
        intx2 = (int)apfig2[10][j + 1].x;
        inty2 = (int)apfig2[10][j + 1].y;
        drawvector(interp, intx1, inty1, intx2, inty2, 3, i_img, "magenta");
      }
      for (i = 0; i < 10; i++) {
        intx1 = (int)apfig2[i][10].x;
        inty1 = (int)apfig2[i][10].y;
        intx2 = (int)apfig2[i + 1][10].x;
        drawvector(interp, intx1, inty1, intx2, inty1, 3, i_img, "magenta");
      }
    }
    break; /* Added break */
  case 9:
    puts("Plot initial guess");
    for (i = 0; i < n_img; i++) {
      /* read control point coordinates for man_ori points */
      fp1 = fopen_r(fixp_name);
      k = 0;
      while (fscanf(fp1, "%d %lf %lf %lf", &fix[k].pnr, &fix[k].x, &fix[k].y,
                    &fix[k].z) != EOF) // No string buffer here, so no width specifier needed for strings
        k++;
      fclose(fp1);
      nfix = k;

      /* take clicked points from control point data set */
      for (j = 0; j < 4; j++)
        for (k = 0; k < nfix; k++) {
          if (fix[k].pnr == nr[i][j])
            fix4[j] = fix[k];
        }

      /* get approx for orientation and ap */
      read_ori(&Ex[i], &I[i], &G[i], img_ori0[i]);
      fp1 = fopen(img_addpar0[i], "r");
      if (!fp1)
        fp1 = fopen("addpar.raw", "r");

      if (fp1) {
        fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &ap[i].k1, &ap[i].k2,
               &ap[i].k3, &ap[i].p1, &ap[i].p2, &ap[i].scx, &ap[i].she);
        fclose(fp1);
      } else {
        printf("no addpar.raw\n");
        ap[i].k1 = ap[i].k2 = ap[i].k3 = ap[i].p1 = ap[i].p2 = ap[i].she = 0.0;
        ap[i].scx = 1.0;
      }

      /* transform clicked points */
      for (j = 0; j < 4; j++) {
        pixel_to_metric(pix0[i][j].x, pix0[i][j].y, imx, imy, pix_x, pix_y,
                        &crd0[i][j].x, &crd0[i][j].y, chfield);
        correct_brown_affin(crd0[i][j].x, crd0[i][j].y, ap[i], &crd0[i][j].x,
                            &crd0[i][j].y);
      }

      /* raw orientation with 4 points */
      raw_orient_v3(Ex[i], I[i], G[i], ap[i], mmp, 4, fix4, crd0[i], &Ex[i],
                    &G[i], 1);

      /* sorting of detected points by back-projection */
      just_plot(interp, Ex[i], I[i], G[i], ap[i], mmp, imx, imy, pix_x, pix_y,
                nfix, fix, chfield, i);
    }

    break; /* Added break */

  case 10:
    puts("Orientation from particles");
    //strcpy(buf, "");
    snprintf(buf, sizeof(buf), "");

    //strcpy(safety[0], "safety_0.ori");
    //strcat(safety[0], ".ori");
    snprintf(safety[0], sizeof(safety[0]), "safety_0.ori");
    //strcpy(safety[1], "safety_1.ori");
    //strcat(safety[1], ".ori");
    snprintf(safety[1], sizeof(safety[1]), "safety_1.ori");
    //strcpy(safety[2], "safety_2.ori");
    //strcat(safety[2], ".ori");
    snprintf(safety[2], sizeof(safety[2]), "safety_2.ori");
    //strcpy(safety[3], "safety_3.ori");
    //strcat(safety[3], ".ori");
    snprintf(safety[3], sizeof(safety[3]), "safety_3.ori");

    //strcpy(safety_addpar[0], "safety_0.addpar");
    //strcat(safety_addpar[0], ".addpar");
    snprintf(safety_addpar[0], sizeof(safety_addpar[0]), "safety_0.addpar");
    //strcpy(safety_addpar[1], "safety_1.addpar");
    //strcat(safety_addpar[1], ".addpar");
    snprintf(safety_addpar[1], sizeof(safety_addpar[1]), "safety_1.addpar");
    //strcpy(safety_addpar[2], "safety_2.addpar");
    //strcat(safety_addpar[2], ".addpar");
    snprintf(safety_addpar[2], sizeof(safety_addpar[2]), "safety_2.addpar");
    //strcpy(safety_addpar[3], "safety_3.addpar");
    //strcat(safety_addpar[3], ".addpar");
    snprintf(safety_addpar[3], sizeof(safety_addpar[3]), "safety_3.addpar");


    for (i_img = 0; i_img < n_img; i_img++) {

      /* read control point coordinates for man_ori points */

      fpp = fopen_r("parameters/sequence.par");
      for (i = 0; i < 4; i++) {
        fscanf(fpp, "%127s\\n", seq_name[i]); /* name of sequence */
                                          // fscanf (fpp,"%d\\n", &seq_first);
        // fscanf (fpp,"%d\\n", &seq_last);
      }
      fclose(fpp);

      fpp = fopen_r("parameters/shaking.par");
      fscanf(fpp, "%d\n", &seq_first);
      fscanf(fpp, "%d\n", &seq_last);
      fclose(fpp);

      /*  read from main parameter file  */
      fpp = fopen_r("parameters/ptv.par");
      fscanf(fpp, "%d\\n", &n_img);
      fclose(fpp);

      i = 0;
      frameCount = 0;
      currentFrame = 0;
      step_shake =
          (int)((double)(seq_last - seq_first + 1) / (double)max_shake_frames +
                0.5);
      printf("\nframe step size for camera %d is %d\n", i_img + 1, step_shake);
      for (filenumber = seq_first + 2; filenumber < seq_last + 1 - 2;
           filenumber = filenumber + step_shake) { // chnaged by Beat Feb 08

        if (filenumber < 10)
          snprintf(filein, sizeof(filein), "res/rt_is.%1d", filenumber);
        else if (filenumber < 100)
          snprintf(filein, sizeof(filein), "res/rt_is.%2d", filenumber);
        else
          snprintf(filein, sizeof(filein), "res/rt_is.%3d", filenumber);

        FILEIN = fopen(filein, "r");
        if (!FILEIN)
          printf("Can't open ascii file: %s\n", filein);
        /////////open target file(s)!
        /* read targets of each camera */

        if (filenumber < 10)
          snprintf(filein_ptv, sizeof(filein_ptv), "res/ptv_is.%1d", filenumber);
        else if (filenumber < 100)
          snprintf(filein_ptv, sizeof(filein_ptv), "res/ptv_is.%2d", filenumber);
        else
          snprintf(filein_ptv, sizeof(filein_ptv), "res/ptv_is.%3d", filenumber);

        // to only use quadruplets for shaking that can be linked
        FILEIN_ptv = fopen(filein_ptv, "r");
        if (!FILEIN_ptv)
          printf("Can't open ascii file: %s\n", filein_ptv);
        /////////open target file(s)!
        /* read targets of each camera */

        nt4[3][i] = 0;
        compose_name_plus_nr_str(seq_name[i_img], "_targets", filenumber,
                                 filein_T);

        FILEIN_T = fopen(filein_T, "r");
        if (!FILEIN_T) { // Added braces for clarity
          printf("Can't open ascii file: %s\\n", filein_T);
        } else { // Added braces for clarity
            fscanf(FILEIN_T, "%d\\n", &nt4[3][i_img]); // This was nt4[3][i] before, likely a typo, changed to i_img
            for (j = 0; j < nt4[3][i_img]; j++) { // This was nt4[3][i] before
              fscanf(FILEIN_T, "%4d %lf %lf %d %d %d %d %d\\n", &t4[3][i_img][j].pnr, // This was t4[3][i] before
                     &t4[3][i_img][j].x, &t4[3][i_img][j].y, &t4[3][i_img][j].n, // This was t4[3][i] before
                     &t4[3][i_img][j].nx, &t4[3][i_img][j].ny, // This was t4[3][i] before
                     &t4[3][i_img][j].sumg, &t4[3][i_img][j].tnr); // This was t4[3][i] before
            }
            fclose(FILEIN_T);
        }
        ////////done reading target files

        if (FILEIN) { // Check if FILEIN is not NULL
            fscanf(FILEIN, "%d\\n", &dummy);     /* read # of 3D points on dummy */
        }
        if (FILEIN_ptv) { // Check if FILEIN_ptv is not NULL
            fscanf(FILEIN_ptv, "%d\\n", &dummy); /* read # of 3D points on dummy */
        }
        
        // Ensure files are open before reading from them in the loop
        if (FILEIN && FILEIN_ptv) {
            do {
              /*read dataset row by row, x,y,z and correspondences */
              a[0] = -1;
              a[1] = -1;
              a[2] = -1;
              a[3] = -1;
              if (n_img == 4) {
                if (fscanf(FILEIN, "%d %lf %lf %lf %d %d %d %d\\n", &dummy, &fix[i].x, // fix[i] uses loop variable i, ensure it's correct context
                       &fix[i].y, &fix[i].z, &a[0], &a[1], &a[2], &a[3]) == EOF) break;
                if (fscanf(FILEIN_ptv, "%d %d %lf %lf %lf\\n", &prev, &next,
                       &dummy_float, &dummy_float, &dummy_float) == EOF) break;
              }
              if (n_img == 3) { // Should be else if or ensure n_img is not both 3 and 4
                if (fscanf(FILEIN, "%d %lf %lf %lf %d %d %d %d\\n", &dummy, &fix[i].x,
                       &fix[i].y, &fix[i].z, &a[0], &a[1], &a[2], &a[3]) == EOF) break;
                if (fscanf(FILEIN_ptv, "%d %d %lf %lf %lf\\n", &prev, &next,
                       &dummy_float, &dummy_float, &dummy_float) == EOF) break;
              }
              ////////////auch pix lesen according a0,a1,a2,a3!!!
              // fix[i].x>-25 &&
              // fix[i].x>-25 &&
              if ((a[i_img] > -1 && next > -1 && prev > -1 &&
                   i < max_shake_points && frameCount < max_shake_frames + 1) ||
                  (a[0] > -1 && a[1] > -1 && a[2] > -1 && a[3] > -1 && next > -1 &&
                   prev > -1 &&
                   filenumber == seq_first + 2)) { // OR ALLE QUADRUPLETS
                pix[i_img][i].x = t4[3][i_img][a[i_img]].x;
                pix[i_img][i].y = t4[3][i_img][a[i_img]].y;
                pix[i_img][i].pnr = i;
                fix[i].pnr = i;

                i++; // This 'i' is from the outer loop for i_img, then reset to 0 inside. This seems problematic.
                     // 'i' should likely be a separate counter for points being collected across frames.
                     // For now, assuming 'i' is intended to be reset for each i_img, but this needs careful review.
                nfix = i;
                if (currentFrame < filenumber) {
                  currentFrame = filenumber;
                  frameCount++;
                }
              }
            } while (!feof(FILEIN) && !feof(FILEIN_ptv)); // Check both files for EOF
        }
        if (FILEIN) fclose(FILEIN); // Close files if they were opened
        if (FILEIN_ptv) fclose(FILEIN_ptv);


      } // end of loop through seq, but loop i_img still open
      if (frameCount == 1) {
        printf("Using %d linked points of %d frame for camera %d\n", nfix,
               frameCount, i_img + 1);
      } else {
        printf("Using %d linked points of %d frames for camera %d\n", nfix,
               frameCount, i_img + 1);
      }
      for (i = 0; i < nfix; i++) {
        pixel_to_metric(pix[i_img][i].x, pix[i_img][i].y, imx, imy, pix_x,
                        pix_y, &crd[i_img][i].x, &crd[i_img][i].y, chfield);
        crd[i_img][i].pnr = pix[i_img][i].pnr;
      }

      /* ================= */

      orient_v3(interp, Ex[i_img], I[i_img], G[i_img], ap[i_img], mmp, nfix,
                fix, crd[i_img], &Ex[i_img], &I[i_img], &G[i_img], &ap[i_img],
                i_img);

      /* ================= */

      /* save orientation and additional parameters */
      // make safety copy of ori files

      fp1 = fopen(img_ori[i_img], "r");
      if (fp1 != NULL) {
        fclose(fp1);
        read_ori(&sEx[i_img], &sI[i_img], &sG[i_img], img_ori[i_img]);
        fp1 = fopen(img_addpar0[i_img], "r");
        if (!fp1)
          fp1 = fopen("addpar.raw", "r");

        if (fp1) {
          fscanf(fp1, "%lf %lf %lf %lf %lf %lf %lf", &sap[i_img].k1,
                 &sap[i_img].k2, &sap[i_img].k3, &sap[i_img].p1, &sap[i_img].p2,
                 &sap[i_img].scx, &sap[i_img].she);
          fclose(fp1);
        } else {
          printf("no addpar.raw\n");
          sap[i_img].k1 = sap[i_img].k2 = sap[i_img].k3 = sap[i_img].p1 =
              sap[i_img].p2 = sap[i_img].she = 0.0;
          sap[i_img].scx = 1.0;
        }

        write_ori(sEx[i_img], sI[i_img], sG[i_img], safety[i_img]);
        fp1 = fopen(safety_addpar[i_img], "w");
        fprintf(fp1, "%f %f %f %f %f %f %f", sap[i_img].k1, sap[i_img].k2,
                sap[i_img].k3, sap[i_img].p1, sap[i_img].p2, sap[i_img].scx,
                sap[i_img].she);
        fclose(fp1);
      } else {
        write_ori(Ex[i_img], I[i_img], G[i_img], safety[i_img]);
        fp1 = fopen(safety_addpar[i_img], "w");
        fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
                ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
                ap[i_img].she);
        fclose(fp1);
      }
      write_ori(Ex[i_img], I[i_img], G[i_img], img_ori[i_img]);
      fp1 = fopen(img_addpar[i_img], "w");
      fprintf(fp1, "%f %f %f %f %f %f %f", ap[i_img].k1, ap[i_img].k2,
              ap[i_img].k3, ap[i_img].p1, ap[i_img].p2, ap[i_img].scx,
              ap[i_img].she);
      fclose(fp1);
    }
    Tcl_Eval(interp, ".text delete 3");
    Tcl_Eval(interp, ".text delete 1");
    Tcl_Eval(interp, ".text insert 1 \"Orientation from particles \"");
    Tcl_Eval(interp, ".text delete 2");
    if (examine != 4)
      Tcl_Eval(interp, ".text insert 2 \"...done, sigma0 for each image -> \"");
    if (examine == 4 && multi == 0)
      Tcl_Eval(interp, ".text insert 2 \"resection data written to disk \"");
    Tcl_SetVar(interp, "tbuf", buf, TCL_GLOBAL_ONLY);
    Tcl_Eval(interp, ".text insert 3 $tbuf");

    break;
  }
  return TCL_OK;
}

int quit_proc_c(ClientData clientData, Tcl_Interp *interp, int argc,
                const char **argv) {
  int i, k;

  for (i = 0; i < n_img; i++) {
    free(img[i]);
    free(img0[i]);
  }
  free(zoomimg);

  /* delete unneeded files */
  for (i = 0; i < n_img; i++)
    k = remove(img_lp_name[i]);
  return TCL_OK;
}
