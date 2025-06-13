/*********************************************************************

Author/Copyright:	 Jochen Willneff

Address:			 Institute of Geodesy and Photogrammetry
                                         ETH - Hoenggerberg
                                         CH - 8093 Zurich

Creation Date:		 September'97

Description:		display of image sequences

Routines contained: flow_demo_c

**********************************************************************/

#include "ptv.h"


int flow_demo_c(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
  int i, nr = 0;
  char buf[256];
  char filename[128];
  char name[128];
  int max_iterations;
  double dt;
  target *pix;

  if (objc < 2) {
    nr = 0;
    snprintf(name, sizeof(name), "dummy");
  } else {
    Tcl_GetIntFromObj(interp, objv[1], &nr);
    snprintf(name, sizeof(name), "%s", Tcl_GetString(objv[2]));
  }

  max_iterations = 4;
  dt = 0.02;

  for (i = 0; i < max_iterations; i++) {
    snprintf(buf, sizeof(buf), "newimage %d", nr + 1);
    Tcl_Eval(interp, buf);
    nr++;
    snprintf(buf, sizeof(buf), "after %d", (int)(dt * 1000));  // NOLINT
    Tcl_Eval(interp, buf);
  }
  printf("done\n\n");
  return TCL_OK;
}
