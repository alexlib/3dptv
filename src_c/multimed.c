/****************************************************************************

Routine:			multimed.c

Author/Copyright:	Hans-Gerd Maas

Address:			Institute of Geodesy and Photogrammetry
                                        ETH - Hoenggerberg
                                        CH - 8093 Zurich

Creation Date:		20.4.88

Description:
1 point, exterior orientation, multimedia parameters	   ==> X,Y=shifts
                  (special case 3 media, medium 2 = plane parallel)

Routines contained: 	   -

****************************************************************************/

// Repaired 'pointer to array' errors, ad holten, 12-2012
// In all the calls to trans_Cam_Point() the & is stripped of from the arguments
//    &cross_p and &cross_c.	(lines: 699 711 727 737 753 763 780 790 808 831)

#include "ptv.h"

double get_mmf_from_mmLUT();

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void multimed(Exterior ex, mm_3p mm, double X, double Y, double Z, double *Xq,
              double *Yq) {
  int i, it = 0;
  double beta1, beta2, beta3, r, rbeta, rdiff, rq, mmf;
  double ocf = 1.0; /* over compensation factor for faster convergence */

  if (mm.n1 == 1 && mm.n2 == 1 && mm.n3 == 1) { /* 1-medium case */
    *Xq = X;
    *Yq = Y;
    return;
  }

  /* interpolation in mmLUT, if selected (requires some global variables) */
  if (mm.lut) {
    /* check, which is the correct image */
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex.x0 && Ex[i].y0 == ex.y0 && Ex[i].z0 == ex.z0)
        break;
    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0) {
      *Xq = ex.x0 + (X - ex.x0) * mmf;
      *Yq = ex.y0 + (Y - ex.y0) * mmf;
      return;
    }
  }

  /* iterative procedure */
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;
  do {
    beta1 = atan(rq / (ex.z0 - Z));
    beta2 = asin(sin(beta1) * mm.n1 / mm.n2);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d) * tan(beta1) + mm.d * tan(beta2) - Z * tan(beta3);
    rdiff = r - rbeta;
    rdiff *= ocf;
    rq += rdiff;
    it++;
  } while ((rdiff > 0.001 || rdiff < -0.001) && it < 40);

  if (it >= 40) {
    puts("Multimed stopped after 40. Iteration");
    if (fabs(rdiff) < 0.1) {
      *Xq = ex.x0 + (X - ex.x0) * rq / r;
      *Yq = ex.y0 + (Y - ex.y0) * rq / r;
    } else {
      *Xq = X;
      *Yq = Y;
    }
    return;
  }
  if (r != 0) {
    *Xq = ex.x0 + (X - ex.x0) * rq / r;
    *Yq = ex.y0 + (Y - ex.y0) * rq / r;
  } else {
    *Xq = X;
    *Yq = Y;
  }
}
#endif

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
double multimed_r(Exterior ex, mm_3p mm, double X, double Y, double Z)
/* calculates and returns the radial shift */
{
  int i, it = 0;
  double beta1, beta2, beta3, r, rbeta, rdiff, rq, mmf;
  double ocf = 1.0; /* over compensation factor for faster convergence */

  if (mm.n1 == 1 && mm.n2 == 1 && mm.n3 == 1)
    return (1.0); /* 1-medium case */

  /* interpolation in mmLUT, if selected (requires some global variables) */
  if (mm.lut) {
    /* check, which is the correct image */
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex.x0 && Ex[i].y0 == ex.y0 && Ex[i].z0 == ex.z0)
        break;

    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0)
      return (mmf);
  }

  /* iterative procedure */
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    beta2 = asin(sin(beta1) * mm.n1 / mm.n2);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d) * tan(beta1) + mm.d * tan(beta2) - Z * tan(beta3);
    rdiff = r - rbeta;
    rdiff *= ocf;
    rq += rdiff;
    it++;
  } while (((rdiff > 0.001) || (rdiff < -0.001)) && it < 40);

  if (it >= 40) {
    puts("Multimed_r stopped after 40. Iteration");
    return (1.0);
  }
  if (r != 0)
    return (rq / r);
  else
    return (1.0);
}
#endif

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void multimed_nlay_true(Exterior ex, Glass gl, mm_np mm, double X, double Y,
                        double Z, double *Xq, double *Yq) {
  int i, it = 0;
  double beta1, beta2[32], beta3, r, rbeta, rdiff, rq, mmf;
  double ocf = 1.00; /* over compensation factor for faster convergence */

  /* interpolation in mmLUT, if selected (requires some global variables) */
  if (mm.lut) {
    /* check, which is the correct image */
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex.x0 && Ex[i].y0 == ex.y0 && Ex[i].z0 == ex.z0)
        break;

    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0) {
      *Xq = ex.x0 + (X - ex.x0) * mmf;
      *Yq = ex.y0 + (Y - ex.y0) * mmf;
      return;
    }
  }

  /* iterative procedure */
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    for (i = 0; i < mm.nlay; i++)
      beta2[i] = asin(sin(beta1) * mm.n1 / mm.n2[i]);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d[0]) * tan(beta1) - Z * tan(beta3);
    for (i = 0; i < mm.nlay; i++)
      rbeta += (mm.d[i] * tan(beta2[i]));
    rdiff = r - rbeta;
    rdiff *= ocf;
    rq += rdiff;
    it++;
  } while (((rdiff > 0.001) || (rdiff < -0.001)) && it < 40);

  if (it >= 40) {
    *Xq = X;
    *Yq = Y;
    puts("Multimed_nlay_true stopped after 40. Iteration");
    return;
  }

  if (r != 0) {
    *Xq = ex.x0 + (X - ex.x0) * rq / r;
    *Yq = ex.y0 + (Y - ex.y0) * rq / r;
  } else {
    *Xq = X;
    *Yq = Y;
  }
}
#endif

void multimed_nlay(Exterior ex, mm_np mm, double X, double Y, double Z,
                   double *Xq, double *Yq) {
  int i, it = 0;
  double beta1, beta2[32], beta3, r, rbeta, rdiff, rq, mmf;

  /* interpolation in mmLUT, if selected (requires some global variables) */
  if (mm.lut) {
    /* check, which is the correct image */
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex.x0 && Ex[i].y0 == ex.y0 && Ex[i].z0 == ex.z0)
        break;
    mmf = get_mmf_from_mmLUT(i, X, Y, Z);

    if (mmf > 0) {
      *Xq = ex.x0 + (X - ex.x0) * mmf;
      *Yq = ex.y0 + (Y - ex.y0) * mmf;
      return;
    }
  }

  /* iterative procedure (if mmLUT does not exist or has no entry) */
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    for (i = 0; i < mm.nlay; i++)
      beta2[i] = asin(sin(beta1) * mm.n1 / mm.n2[i]);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d[0]) * tan(beta1) - Z * tan(beta3);
    for (i = 0; i < mm.nlay; i++)
      rbeta += (mm.d[i] * tan(beta2[i]));
    rdiff = r - rbeta;
    rq += rdiff;
    it++;
  } while ((rdiff > 0.001 || rdiff < -0.001) && it < 40);

  if (it >= 40) {
    *Xq = X;
    *Yq = Y;
    puts("Multimed_nlay stopped after 40. Iteration");
    return;
  }

  if (r != 0) {
    *Xq = ex.x0 + (X - ex.x0) * rq / r;
    *Yq = ex.y0 + (Y - ex.y0) * rq / r;
  } else {
    *Xq = X;
    *Yq = Y;
  }
}

void multimed_nlay_v2(Exterior ex, Exterior ex_o, mm_np mm, double X, double Y,
                      double Z, double *Xq, double *Yq) {
  // Beat L�thi, Nov 2007 comment actually only Xq is affected since all Y and
  // Yq are always zero
  int i, it = 0;
  double beta1, beta2[32], beta3, r, rbeta, rdiff, rq, mmf;

  // interpolation in mmLUT, if selected (requires some global variables)
  if (mm.lut) {
    // check, which is the correct image
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex_o.x0 && Ex[i].y0 == ex_o.y0 && Ex[i].z0 == ex_o.z0)
        break;

    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0) {
      *Xq = ex.x0 + (X - ex.x0) * mmf;
      *Yq = ex.y0 + (Y - ex.y0) * mmf;
      return;
    }
  }

  // iterative procedure (if mmLUT does not exist or has no entry)
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    for (i = 0; i < mm.nlay; i++)
      beta2[i] = asin(sin(beta1) * mm.n1 / mm.n2[i]);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d[0]) * tan(beta1) - Z * tan(beta3);
    for (i = 0; i < mm.nlay; i++)
      rbeta += (mm.d[i] * tan(beta2[i]));
    rdiff = r - rbeta;
    rq += rdiff;
    it++;
  } while ((rdiff > 0.001 || rdiff < -0.001) && it < 40);

  if (it >= 40) {
    *Xq = X;
    *Yq = Y;
    puts("Multimed_nlay stopped after 40. Iteration");
    return;
  }

  if (r != 0) {
    *Xq = ex.x0 + (X - ex.x0) * rq / r;
    *Yq = ex.y0 + (Y - ex.y0) * rq / r;
  } else {
    *Xq = X;
    *Yq = Y;
  }

  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------

  // Beat L�thi, Nov 2007, I will implement the Ris� method to avoid all this
  // sin, atan etc

  /*double absError=100;
  int counter=0;
  double dir_water_x=-X;
  double dir_water_z=ex.z0-Z;
  double dist=pow(dir_water_x*dir_water_x+dir_water_z*dir_water_z,0.5);
  double
  xInInterFace,comp_parallel,comp_perpendicular,dir_air_x,dir_air_z,error_x,error_z;
  dir_water_x=dir_water_x/dist;
  dir_water_z=dir_water_z/dist;

  while(absError>0.00001 && counter<40){
     //sending the water_ray to air
     xInInterFace=X-Z/dir_water_z*dir_water_x; //
     //snellNormal_y=cross(dir_water,interF.base[2]);
     //snellNormal=snellNormal/norm(snellNormal);
     //dir_phi=cross(interF.base[2],snellNormal);
     comp_parallel=dir_water_x; //dot(dir_water,dir_phi);
     comp_perpendicular=pow(1./(mm.n3/mm.n1*mm.n3/mm.n1)-comp_parallel*comp_parallel,0.5);
  //sqrt(1./(1.333*1.333)-comp_parallel*comp_parallel);
     //dir_air=comp_perpendicular*interF.base[2]+comp_parallel*dir_phi;
     dir_air_x=dir_water_x;
     dir_air_z=comp_perpendicular;
     //dir_air=dir_air/norm(dir_air);
     dist=pow(dir_air_x*dir_air_x+dir_air_z*dir_air_z,0.5);
     dir_air_x=dir_air_x/dist;
     dir_air_z=dir_air_z/dist;

     //measuring how much cam_origo is missed and correcting water_ray
     //error=pCamOrigo-xInInterFace-dot(pCamOrigo-xInInterFace,dir_air)*dir_air/norm2(dir_air);
     error_x=ex.x0-xInInterFace-((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_x;
     error_z=ex.z0-
  ((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_z;
     absError=pow(error_x*error_x+error_z*error_z,0.5);
     //dir_water=xInInterFace-particlePoint+0.1*error;
     dir_water_x=xInInterFace-X+0.1*error_x;
     dir_water_z=-Z+0.1*error_z;
     //dir_water = dir_water/norm(dir_water);
     dist=pow(dir_water_x*dir_water_x+dir_water_z*dir_water_z,0.5);
     dir_water_x=dir_water_x/dist;
     dir_water_z=dir_water_z/dist;
     counter++;
  }
  *Xq = (Z-ex.z0)/dir_air_z*dir_air_x;
  *Yq = 0;
  */
}

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
double multimed_r_nlay(Exterior ex, mm_np mm, double X, double Y, double Z)
// calculates and returns the radial shift
{
  int i, it = 0;
  double beta1, beta2[32], beta3, r, rbeta, rdiff, rq, mmf;
  double ocf = 1.0; // over compensation factor for faster convergence

  double absError = 100;
  int counter = 0;
  double dir_water_x = -X;
  double dir_water_z = ex.z0 - Z;
  double dist = pow(dir_water_x * dir_water_x + dir_water_z * dir_water_z, 0.5);

  dir_water_x = dir_water_x / dist;
  dir_water_z = dir_water_z / dist;

  // 1-medium case
  if (mm.n1 == 1 && mm.nlay == 1 && mm.n2[0] == 1 && mm.n3 == 1)
    return (1.0);

  // interpolation in mmLUT, if selected (requires some global variables)
  if (mm.lut) {
    // check, which is the correct image
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex.x0 && Ex[i].y0 == ex.y0 && Ex[i].z0 == ex.z0)
        break;

    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0)
      return (mmf);
  }

  // iterative procedure
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    for (i = 0; i < mm.nlay; i++)
      beta2[i] = asin(sin(beta1) * mm.n1 / mm.n2[i]);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d[0]) * tan(beta1) - Z * tan(beta3);
    for (i = 0; i < mm.nlay; i++)
      rbeta += (mm.d[i] * tan(beta2[i]));
    rdiff = r - rbeta;
    rdiff *= ocf;
    rq += rdiff;
    it++;
  } while ((rdiff > 0.001 || rdiff < -0.001) && it < 40);

  if (it >= 40) {
    puts("Multimed_r_nlay stopped after 40. Iteration");
    return (1.0);
  }

  if (r != 0)
    return (rq / r);
  else
    return (1.0);

  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------

  // Beat L�thi, Nov 2007, I will implement the Ris� method to avoid all this
  // sin, atan etc

  /*
  while(absError>0.00001 && counter<40){
     //sending the water_ray to air
     xInInterFace=X-Z/dir_water_z*dir_water_x; //
     //snellNormal_y=cross(dir_water,interF.base[2]);
     //snellNormal=snellNormal/norm(snellNormal);
     //dir_phi=cross(interF.base[2],snellNormal);
     comp_parallel=dir_water_x; //dot(dir_water,dir_phi);
     comp_perpendicular=pow(1./(mm.n3/mm.n1*mm.n3/mm.n1)-comp_parallel*comp_parallel,0.5);
  //sqrt(1./(1.333*1.333)-comp_parallel*comp_parallel);
     //dir_air=comp_perpendicular*interF.base[2]+comp_parallel*dir_phi;
     dir_air_x=dir_water_x;
     dir_air_z=comp_perpendicular;
     //dir_air=dir_air/norm(dir_air);
     dist=pow(dir_air_x*dir_air_x+dir_air_z*dir_air_z,0.5);
     dir_air_x=dir_air_x/dist;
     dir_air_z=dir_air_z/dist;

     //measuring how much cam_origo is missed and correcting water_ray
     //error=pCamOrigo-xInInterFace-dot(pCamOrigo-xInInterFace,dir_air)*dir_air/norm2(dir_air);
     error_x=ex.x0-xInInterFace-((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_x;
     error_z=ex.z0-
  ((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_z;
     absError=pow(error_x*error_x+error_z*error_z,0.5);
     //dir_water=xInInterFace-particlePoint+0.1*error;
     dir_water_x=xInInterFace-X+0.1*error_x;
     dir_water_z=-Z+0.1*error_z;
     //dir_water = dir_water/norm(dir_water);
     dist=pow(dir_water_x*dir_water_x+dir_water_z*dir_water_z,0.5);
     dir_water_x=dir_water_x/dist;
     dir_water_z=dir_water_z/dist;
     counter++;
  }
  return( (Z-ex.z0)/dir_air_z*dir_air_x / X  );*/
}
#endif

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void back_trans_Point_back(double X_t, double Y_t, double Z_t, mm_np mm,
                           Glass G, double cross_p[], double cross_c[],
                           double *X, double *Y, double *Z) {
  double nVe, nGl;
  nGl = sqrt(pow(G.vec_x, 2.) + pow(G.vec_y, 2.) + pow(G.vec_z, 2.));
  nVe = sqrt(pow(cross_p[0] - (cross_c[0] - mm.d[0] * G.vec_x / nGl), 2.) +
             pow(cross_p[1] - (cross_c[1] - mm.d[0] * G.vec_y / nGl), 2.) +
             pow(cross_p[2] - (cross_c[2] - mm.d[0] * G.vec_z / nGl), 2.));

  *X = cross_c[0] +
       X_t * (cross_p[0] - (cross_c[0] - mm.d[0] * G.vec_x / nGl)) / nVe +
       Z_t * G.vec_x / nGl;
  *Y = cross_c[1] +
       X_t * (cross_p[1] - (cross_c[1] - mm.d[0] * G.vec_y / nGl)) / nVe +
       Z_t * G.vec_y / nGl;
  *Z = cross_c[2] +
       X_t * (cross_p[2] - (cross_c[2] - mm.d[0] * G.vec_z / nGl)) / nVe +
       Z_t * G.vec_z / nGl;
}
#endif

#ifdef EVER_CALLED // Unused function, ad holten 12-2012
void trans_Cam_Point_back(Exterior ex, mm_np mm, Glass gl, double X, double Y,
                          double Z, Exterior *ex_t, double *X_t, double *Y_t,
                          double *Z_t, double *cross_p, double *cross_c) {
  //--Beat L�thi June 07: I change the stuff to a system perpendicular to the
  //interface
  double dummy;
  double nGl;

  nGl = sqrt(gl.vec_x * gl.vec_x + gl.vec_y * gl.vec_y + gl.vec_z * gl.vec_z);

  dummy = ex.x0 * gl.vec_x / nGl + ex.y0 * gl.vec_y / nGl +
          ex.z0 * gl.vec_z / nGl - nGl - mm.d[0];
  cross_c[0] = ex.x0 - dummy * gl.vec_x / nGl;
  cross_c[1] = ex.y0 - dummy * gl.vec_y / nGl;
  cross_c[2] = ex.z0 - dummy * gl.vec_z / nGl;
  ex_t->x0 = 0.;
  ex_t->y0 = 0.;
  ex_t->z0 = dummy;

  dummy = nGl - X * gl.vec_x / nGl + Y * gl.vec_y / nGl + Z * gl.vec_z / nGl;
  cross_p[0] = X + dummy * gl.vec_x / nGl;
  cross_p[1] = Y + dummy * gl.vec_y / nGl;
  cross_p[2] = Z + dummy * gl.vec_z / nGl;
  *Z_t = -dummy - mm.d[0];
  dummy = sqrt(pow(cross_p[0] - (cross_c[0] - mm.d[0] * gl.vec_x / nGl), 2.) +
               pow(cross_p[1] - (cross_c[1] - mm.d[0] * gl.vec_y / nGl), 2.) +
               pow(cross_p[2] - (cross_c[2] - mm.d[0] * gl.vec_z / nGl), 2.));
  *X_t = dummy;
  *Y_t = 0;
}
#endif

void trans_Cam_Point(Exterior ex, mm_np mm, Glass gl, double X, double Y,
                     double Z, Exterior *ex_t, double *X_t, double *Y_t,
                     double *Z_t, double *cross_p, double *cross_c) {
  //--Beat L�thi June 07: I change the stuff to a system perpendicular to the
  //interface
  double dist_cam_glas, dist_point_glas, dist_o_glas; // glas inside at water

  dist_o_glas =
      sqrt(gl.vec_x * gl.vec_x + gl.vec_y * gl.vec_y + gl.vec_z * gl.vec_z);
  dist_cam_glas = ex.x0 * gl.vec_x / dist_o_glas +
                  ex.y0 * gl.vec_y / dist_o_glas +
                  ex.z0 * gl.vec_z / dist_o_glas - dist_o_glas - mm.d[0];
  dist_point_glas = X * gl.vec_x / dist_o_glas + Y * gl.vec_y / dist_o_glas +
                    Z * gl.vec_z / dist_o_glas - dist_o_glas;

  cross_c[0] = ex.x0 - dist_cam_glas * gl.vec_x / dist_o_glas;
  cross_c[1] = ex.y0 - dist_cam_glas * gl.vec_y / dist_o_glas;
  cross_c[2] = ex.z0 - dist_cam_glas * gl.vec_z / dist_o_glas;
  cross_p[0] = X - dist_point_glas * gl.vec_x / dist_o_glas;
  cross_p[1] = Y - dist_point_glas * gl.vec_y / dist_o_glas;
  cross_p[2] = Z - dist_point_glas * gl.vec_z / dist_o_glas;

  ex_t->x0 = 0.;
  ex_t->y0 = 0.;
  ex_t->z0 = dist_cam_glas + mm.d[0];

  *X_t = sqrt(
      pow(cross_p[0] - (cross_c[0] - mm.d[0] * gl.vec_x / dist_o_glas), 2.) +
      pow(cross_p[1] - (cross_c[1] - mm.d[0] * gl.vec_y / dist_o_glas), 2.) +
      pow(cross_p[2] - (cross_c[2] - mm.d[0] * gl.vec_z / dist_o_glas), 2.));
  *Y_t = 0;
  *Z_t = dist_point_glas;
}

void back_trans_Point(double X_t, double Y_t, double Z_t, mm_np mm, Glass G,
                      double cross_p[], double cross_c[], double *X, double *Y,
                      double *Z) {
  double nVe, nGl;
  nGl = sqrt(pow(G.vec_x, 2.) + pow(G.vec_y, 2.) + pow(G.vec_z, 2.));
  nVe = sqrt(pow(cross_p[0] - (cross_c[0] - mm.d[0] * G.vec_x / nGl), 2.) +
             pow(cross_p[1] - (cross_c[1] - mm.d[0] * G.vec_y / nGl), 2.) +
             pow(cross_p[2] - (cross_c[2] - mm.d[0] * G.vec_z / nGl), 2.));

  *X = cross_c[0] - mm.d[0] * G.vec_x / nGl +
       X_t * (cross_p[0] - (cross_c[0] - mm.d[0] * G.vec_x / nGl)) / nVe +
       Z_t * G.vec_x / nGl;
  *Y = cross_c[1] - mm.d[0] * G.vec_y / nGl +
       X_t * (cross_p[1] - (cross_c[1] - mm.d[0] * G.vec_y / nGl)) / nVe +
       Z_t * G.vec_y / nGl;
  *Z = cross_c[2] - mm.d[0] * G.vec_z / nGl +
       X_t * (cross_p[2] - (cross_c[2] - mm.d[0] * G.vec_z / nGl)) / nVe +
       Z_t * G.vec_z / nGl;
}

double multimed_r_nlay_v2(Exterior ex, Exterior ex_o, mm_np mm, double X,
                          double Y, double Z)
/* calculates and returns the radial shift */
{
  int i, it = 0;
  double beta1, beta2[32], beta3, r, rbeta, rdiff, rq, mmf;
  double ocf = 1.0; // over compensation factor for faster convergence

  double absError = 100;
  int counter = 0;
  double dir_water_x = -X;
  double dir_water_z = ex.z0 - Z;
  double dist = pow(dir_water_x * dir_water_x + dir_water_z * dir_water_z, 0.5);

  dir_water_x = dir_water_x / dist;
  dir_water_z = dir_water_z / dist;

  // 1-medium case
  if (mm.n1 == 1 && mm.nlay == 1 && mm.n2[0] == 1 && mm.n3 == 1)
    return (1.0);

  // interpolation in mmLUT, if selected (requires some global variables)
  if (mm.lut) {
    // check, which is the correct image
    for (i = 0; i < n_img; i++)
      if (Ex[i].x0 == ex_o.x0 && Ex[i].y0 == ex_o.y0 && Ex[i].z0 == ex_o.z0)
        break;

    mmf = get_mmf_from_mmLUT(i, X, Y, Z);
    if (mmf > 0)
      return (mmf);
  }

  // iterative procedure
  r = sqrt((X - ex.x0) * (X - ex.x0) + (Y - ex.y0) * (Y - ex.y0));
  rq = r;

  do {
    beta1 = atan(rq / (ex.z0 - Z));
    for (i = 0; i < mm.nlay; i++)
      beta2[i] = asin(sin(beta1) * mm.n1 / mm.n2[i]);
    beta3 = asin(sin(beta1) * mm.n1 / mm.n3);

    rbeta = (ex.z0 - mm.d[0]) * tan(beta1) - Z * tan(beta3);
    for (i = 0; i < mm.nlay; i++)
      rbeta += (mm.d[i] * tan(beta2[i]));
    rdiff = r - rbeta;
    rdiff *= ocf;
    rq += rdiff;
    it++;
  } while ((rdiff > 0.001 || rdiff < -0.001) && it < 40);

  if (it >= 40) {
    puts("Multimed_r_nlay_v2 stopped after 40. Iteration");
    return (1.0);
  }
  if (r != 0)
    return (rq / r);
  else
    return (1.0);

  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------
  //-----------------------------------------------------------------------------------------

  // Beat L�thi, Nov 2007, I will implement the Ris� method to avoid all this
  // sin, atan etc
  /*

  while(absError>0.00001 && counter<40){
     //sending the water_ray to air
     xInInterFace=X-Z/dir_water_z*dir_water_x; //
     //snellNormal_y=cross(dir_water,interF.base[2]);
     //snellNormal=snellNormal/norm(snellNormal);
     //dir_phi=cross(interF.base[2],snellNormal);
     comp_parallel=dir_water_x; //dot(dir_water,dir_phi);
     comp_perpendicular=pow(1./(mm.n3/mm.n1*mm.n3/mm.n1)-comp_parallel*comp_parallel,0.5);
  //sqrt(1./(1.333*1.333)-comp_parallel*comp_parallel);
     //dir_air=comp_perpendicular*interF.base[2]+comp_parallel*dir_phi;
     dir_air_x=dir_water_x;
     dir_air_z=comp_perpendicular;
     //dir_air=dir_air/norm(dir_air);
     dist=pow(dir_air_x*dir_air_x+dir_air_z*dir_air_z,0.5);
     dir_air_x=dir_air_x/dist;
     dir_air_z=dir_air_z/dist;

     //measuring how much cam_origo is missed and correcting water_ray
     //error=pCamOrigo-xInInterFace-dot(pCamOrigo-xInInterFace,dir_air)*dir_air/norm2(dir_air);
     error_x=ex.x0-xInInterFace-((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_x;
     error_z=ex.z0-
  ((ex.x0-xInInterFace)*dir_air_x+ex.z0*dir_air_z)*dir_air_z;
     absError=pow(error_x*error_x+error_z*error_z,0.5);
     //dir_water=xInInterFace-particlePoint+0.1*error;
     dir_water_x=xInInterFace-X+0.1*error_x;
     dir_water_z=-Z+0.1*error_z;
     //dir_water = dir_water/norm(dir_water);
     dist=pow(dir_water_x*dir_water_x+dir_water_z*dir_water_z,0.5);
     dir_water_x=dir_water_x/dist;
     dir_water_z=dir_water_z/dist;
     counter++;
  }
  return( (Z-ex.z0)/dir_air_z*dir_air_x / X  );*/
}

void init_mmLUT(int i_cam) {
  register int i, j, nr, nz;
  double X, Y, Z, R, X1, Y1, Z1, Zmin, Rmax = 0, Zmax, a, b, c;
  double x, y, *Ri, *Zi;
  double rw = 2; // was 2, has unit [mm]??, Beat L�thi Aug 1, 2007, is ok
  Exterior Ex_t[4];
  double X_t, Y_t, Z_t, cross_p[3], cross_c[3], Zmin_t, Zmax_t;

  /* find extrema of imaged object volume */
  /* ==================================== */

  /* find extrema in depth */

  fpp = fopen_rp(
      "parameters/criteria.par"); // replaced fopen(), ad holten 12-2012
  if (!fpp)
    return;
  fscanf(fpp, "%lf\n", &X);
  fscanf(fpp, "%lf\n", &Zmin);
  fscanf(fpp, "%lf\n", &Zmax);
  fscanf(fpp, "%lf\n", &X);
  fscanf(fpp, "%lf\n", &Z);
  if (Z < Zmin)
    Zmin = Z;
  fscanf(fpp, "%lf\n", &Z);
  if (Z > Zmax)
    Zmax = Z;
  fclose(fpp);

  Zmin -= fmod(Zmin, rw);
  Zmax += (rw - fmod(Zmax, rw));
  Zmin_t = Zmin;
  Zmax_t = Zmax;

  /* intersect with image vertices rays */

  //--00
  pixel_to_metric(0., 0., imx, imy, pix_x, pix_y, &x, &y, chfield);
  x = x - I[i_cam].xh;
  y = y - I[i_cam].yh;
  correct_brown_affin(x, y, ap[i_cam], &x, &y);
  ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                 &b, &c);
  Z = Zmin;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;

  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);

  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  if (R > Rmax)
    Rmax = R;
  Z = Zmax;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  //--0y
  if (R > Rmax)
    Rmax = R;
  pixel_to_metric(0., (double)imy, imx, imy, pix_x, pix_y, &x, &y, chfield);
  x = x - I[i_cam].xh;
  y = y - I[i_cam].yh;
  correct_brown_affin(x, y, ap[i_cam], &x, &y);
  ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                 &b, &c);
  Z = Zmin;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  if (R > Rmax)
    Rmax = R;
  Z = Zmax;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  //--x0
  if (R > Rmax)
    Rmax = R;
  pixel_to_metric((double)imx, 0., imx, imy, pix_x, pix_y, &x, &y, chfield);
  x = x - I[i_cam].xh;
  y = y - I[i_cam].yh;
  correct_brown_affin(x, y, ap[i_cam], &x, &y);
  ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                 &b, &c);
  Z = Zmin;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  if (R > Rmax)
    Rmax = R;
  Z = Zmax;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  //--xy
  if (R > Rmax)
    Rmax = R;
  pixel_to_metric((double)imx, (double)imy, imx, imy, pix_x, pix_y, &x, &y,
                  chfield);
  x = x - I[i_cam].xh;
  y = y - I[i_cam].yh;
  correct_brown_affin(x, y, ap[i_cam], &x, &y);
  ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                 &b, &c);
  Z = Zmin;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  if (R > Rmax)
    Rmax = R;
  Z = Zmax;
  X = X1 + (Z - Z1) * a / c;
  Y = Y1 + (Z - Z1) * b / c;
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);
  if (Z_t < Zmin_t)
    Zmin_t = Z_t;
  if (Z_t > Zmax_t)
    Zmax_t = Z_t;
  //
  R = sqrt((X_t - Ex_t[i_cam].x0) * (X_t - Ex_t[i_cam].x0) +
           (Y_t - Ex_t[i_cam].y0) * (Y_t - Ex_t[i_cam].y0));

  if (R > Rmax)
    Rmax = R;

  /* round values (-> enlarge) */
  Rmax += (rw - fmod(Rmax, rw));

  /* get # of rasterlines in r,z */
  nr = (int)(Rmax / rw + 1);
  nz = (int)((Zmax_t - Zmin_t) / rw + 1);

  /* create twodimensional mmLUT structure */
  // trans
  trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t, &Y_t,
                  &Z_t, cross_p, cross_c);

  mmLUT[i_cam].origin.x = Ex_t[i_cam].x0;
  mmLUT[i_cam].origin.y = Ex_t[i_cam].y0;
  mmLUT[i_cam].origin.z = Zmin_t;
  mmLUT[i_cam].nr = nr;
  mmLUT[i_cam].nz = nz;
  mmLUT[i_cam].rw = rw;
  if (mmLUT[i_cam].data != NULL) // preventing memory leaks, ad holten, 04-2013
    free(mmLUT[i_cam].data);
  mmLUT[i_cam].data = (double *)malloc(nr * nz * sizeof(double));

  /* fill mmLUT structure */
  /* ==================== */

  Ri = (double *)malloc(nr * sizeof(double));
  for (i = 0; i < nr; i++)
    Ri[i] = i * rw;
  Zi = (double *)malloc(nz * sizeof(double));
  for (i = 0; i < nz; i++)
    Zi[i] = Zmin_t + i * rw;

  for (i = 0; i < nr; i++)
    for (j = 0; j < nz; j++) {
      // old mmLUT[i_cam].data[i*nz + j]= multimed_r_nlay (Ex[i_cam], mmp,
      //													Ri[i]+Ex[i_cam].x0,
      //Ex[i_cam].y0, Zi[j]); trans
      trans_Cam_Point(Ex[i_cam], mmp, G[i_cam], X, Y, Z, &Ex_t[i_cam], &X_t,
                      &Y_t, &Z_t, cross_p, cross_c);

      mmLUT[i_cam].data[i * nz + j] =
          multimed_r_nlay_v2(Ex_t[i_cam], Ex[i_cam], mmp,
                             Ri[i] + Ex_t[i_cam].x0, Ex_t[i_cam].y0, Zi[j]);
    }
  free(Ri); // preventing memory leaks, ad holten, 04-2013
  free(Zi);
}

double get_mmf_from_mmLUT(int i_cam, double X, double Y, double Z) {
  int i, ir, iz, nr, nz, v4[4];
  double R, sr, sz, rw, mmf = 1;

  rw = mmLUT[i_cam].rw;

  Z -= mmLUT[i_cam].origin.z;
  sz = Z / rw;
  iz = (int)sz;
  sz -= iz;
  X -= mmLUT[i_cam].origin.x;
  Y -= mmLUT[i_cam].origin.y;
  R = sqrt(X * X + Y * Y);
  sr = R / rw;
  ir = (int)sr;
  sr -= ir;

  nz = mmLUT[i_cam].nz;
  nr = mmLUT[i_cam].nr;

  /* check whether point is inside camera's object volume */
  if (ir > nr)
    return (0);
  if (iz < 0 || iz > nz)
    return (0);

  /* bilinear interpolation in r/z box */
  /* ================================= */

  /* get vertices of box */
  v4[0] = ir * nz + iz;
  v4[1] = ir * nz + (iz + 1);
  v4[2] = (ir + 1) * nz + iz;
  v4[3] = (ir + 1) * nz + (iz + 1);

  /* 2. check wther point is inside camera's object volume */
  /* important for epipolar line computation */
  for (i = 0; i < 4; i++)
    if (v4[i] < 0 || v4[i] > nr * nz)
      return (0);

  /* interpolate */
  mmf = mmLUT[i_cam].data[v4[0]] * (1 - sr) * (1 - sz) +
        mmLUT[i_cam].data[v4[1]] * (1 - sr) * sz +
        mmLUT[i_cam].data[v4[2]] * sr * (1 - sz) +
        mmLUT[i_cam].data[v4[3]] * sr * sz;

  return (mmf);
}

void volumedimension(double *xmax, double *xmin, double *ymax, double *ymin,
                     double *zmax, double *zmin) {
  int i_cam;
  double X, Y, Z, R, X1, Y1, Z1, Zmin, Rmax = 0, Zmax, a, b, c;
  double x, y;

  /* find extrema of imaged object volume */
  /* ==================================== */

  fpp =
      fopen_rp("parameters/criteria.par"); // replaced fopen, ad holten, 12-2012
  if (!fpp)
    return;
  fscanf(fpp, "%lf\n", &X);
  fscanf(fpp, "%lf\n", &Zmin);
  fscanf(fpp, "%lf\n", &Zmax);
  fscanf(fpp, "%lf\n", &X);
  fscanf(fpp, "%lf\n", &Z);
  if (Z < Zmin)
    Zmin = Z;
  fscanf(fpp, "%lf\n", &Z);
  if (Z > Zmax)
    Zmax = Z;
  fclose(fpp);

  *zmin = Zmin;
  *zmax = Zmax;

  for (i_cam = 0; i_cam < n_img; i_cam++) {
    /* intersect with image vertices rays */
    pixel_to_metric(0.0, 0.0, imx, imy, pix_x, pix_y, &x, &y, chfield);
    x = x - I[i_cam].xh;
    y = y - I[i_cam].yh;
    correct_brown_affin(x, y, ap[i_cam], &x, &y);
    ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                   &b, &c);
    Z = Zmin;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    Z = Zmax;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    pixel_to_metric(0.0, (double)imy, imx, imy, pix_x, pix_y, &x, &y, chfield);
    x = x - I[i_cam].xh;
    y = y - I[i_cam].yh;
    correct_brown_affin(x, y, ap[i_cam], &x, &y);
    ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                   &b, &c);
    Z = Zmin;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    Z = Zmax;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;

    pixel_to_metric((double)imx, 0., imx, imy, pix_x, pix_y, &x, &y, chfield);
    x = x - I[i_cam].xh;
    y = y - I[i_cam].yh;
    correct_brown_affin(x, y, ap[i_cam], &x, &y);
    ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                   &b, &c);
    Z = Zmin;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));
    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    Z = Zmax;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    pixel_to_metric((double)imx, (double)imy, imx, imy, pix_x, pix_y, &x, &y,
                    chfield);
    x = x - I[i_cam].xh;
    y = y - I[i_cam].yh;
    correct_brown_affin(x, y, ap[i_cam], &x, &y);
    ray_tracing_v2(x, y, Ex[i_cam], I[i_cam], G[i_cam], mmp, &X1, &Y1, &Z1, &a,
                   &b, &c);
    Z = Zmin;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;

    if (R > Rmax)
      Rmax = R;
    Z = Zmax;
    X = X1 + (Z - Z1) * a / c;
    Y = Y1 + (Z - Z1) * b / c;
    R = sqrt((X - Ex[i_cam].x0) * (X - Ex[i_cam].x0) +
             (Y - Ex[i_cam].y0) * (Y - Ex[i_cam].y0));

    if (X > *xmax)
      *xmax = X;
    if (X < *xmin)
      *xmin = X;
    if (Y > *ymax)
      *ymax = Y;
    if (Y < *ymin)
      *ymin = Y;
  }
}
