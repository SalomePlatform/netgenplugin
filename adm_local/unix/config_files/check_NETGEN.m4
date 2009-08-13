dnl  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
dnl
dnl  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
dnl  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
dnl
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2.1 of the License.
dnl
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public
dnl  License along with this library; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl
AC_DEFUN([CHECK_NETGEN],[

AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_CHECKING(for Netgen 4.5 and higher Libraries)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(netgen,
	    [  --with-netgen=DIR root directory path of NETGEN installation],
	    NETGEN_HOME=$withval,NETGEN_HOME="")

NETGEN_INCLUDES=""
NETGEN_LIBS_DIR=""

Netgen_ok=no

if test "x$NETGEN_HOME" == "x" ; then

# no --with-netgen option used
   if test "x$NETGENHOME" != "x" ; then

    # NETGENHOME environment variable defined
      NETGEN_HOME=$NETGENHOME

   fi
# 
fi

if test "x$NETGEN_HOME" != "x"; then

  echo
  echo
  echo -------------------------------------------------
  echo You are about to choose to use somehow the
  echo Netgen Library to generate Tetrahedric mesh.
  echo
  echo WARNING
  echo ----------------------------------------------------------
  echo ----------------------------------------------------------
  echo You are strongly advised to consult the file
  echo NETGENPLUGIN_SRC/src/NETGEN/ReadMeForNgUsers, particularly about
  echo assumptions made on the installation of the Netgen
  echo application and libraries.
  echo Ask your system administrator for those details.
  echo ----------------------------------------------------------
  echo ----------------------------------------------------------
  echo 
  echo

  

  NETGEN_INCLUDES="-I${NETGEN_HOME}/include"
  
  # check ${NETGEN_HOME}/lib/LINUX directory for libraries
  if test -f ${NETGEN_HOME}/lib/LINUX/libcsg.a ; then
  	NETGEN_LIBS_DIR="${NETGEN_HOME}/lib/LINUX"
  else
	# check ${NETGEN_HOME}/lib/LINUX64 directory for libraries
    	if test -f ${NETGEN_HOME}/lib/LINUX64/libcsg.a ; then
	  	NETGEN_LIBS_DIR="${NETGEN_HOME}/lib/LINUX64"
	else
	  	NETGEN_LIBS_DIR="${NETGEN_HOME}/lib"
	fi
  fi

  echo "NETGEN_LIBS_DIR = $NETGEN_LIBS_DIR"
  
  CPPFLAGS_old="$CPPFLAGS"
  CXXFLAGS_old="$CXXFLAGS"
  CPPFLAGS="$NETGEN_INCLUDES $CAS_CPPFLAGS $CPPFLAGS"
  CXXFLAGS="$NETGEN_INCLUDES $CAS_CPPFLAGS $CXXFLAGS"

  AC_MSG_CHECKING(for Netgen header file)

  AC_CHECK_HEADER(nglib.h,Netgen_ok=yes,Netgen_ok=no)
  if test "x$Netgen_ok" == "xyes"; then

    AC_MSG_CHECKING(for Netgen libraries)

    LDFLAGS_old="$LDFLAGS"
    LDFLAGS="-L. -lNETGEN $CAS_LDPATH -lTKBRep -lTKShHealing -lTKSTEP -lTKXSBase -lTKIGES -lTKSTL -lTKTopAlgo $LDFLAGS"

    AC_TRY_COMPILE(#include <iostream>
#include <fstream>
namespace nglib {
#include "nglib.h"
}
#define OCCGEOMETRY
#include <occgeom.hpp>
,nglib::Ng_Init();
 netgen::OCCGeometry occgeo;
 nglib::Ng_Exit();,Netgen_ok=yes;ar x "$NETGEN_LIBS_DIR/libnginterface.a";
	    ar x "$NETGEN_LIBS_DIR/libocc.a";
            ar x "$NETGEN_LIBS_DIR/libcsg.a";
            ar x "$NETGEN_LIBS_DIR/libgprim.a";
            ar x "$NETGEN_LIBS_DIR/libmesh.a";
            ar x "$NETGEN_LIBS_DIR/libopti.a";
            ar x "$NETGEN_LIBS_DIR/libgen.a";
            ar x "$NETGEN_LIBS_DIR/libla.a";
            ar x "$NETGEN_LIBS_DIR/libstlgeom.a";
            ar x "$NETGEN_LIBS_DIR/libgeom2d.a";
            $CXX -shared linopt.o bfgs.o linsearch.o global.o bisect.o meshtool.o refine.o ruler3.o improve3.o adfront3.o tetrarls.o prism2rls.o profiler.o pyramidrls.o pyramid2rls.o netrule3.o ruler2.o meshclass.o improve2.o adfront2.o netrule2.o triarls.o geomsearch.o secondorder.o meshtype.o parser3.o quadrls.o specials.o parser2.o meshing2.o meshing3.o meshfunc.o localh.o improve2gen.o delaunay.o boundarylayer.o msghandler.o meshfunc2d.o smoothing2.o smoothing3.o topology.o curvedelems_new.o clusters.o zrefine.o ngexception.o geomtest3d.o geom2d.o geom2dmesh.o geom3d.o adtree.o transform3d.o geomfuncs.o polynomial.o densemat.o vector.o basemat.o sparsmat.o algprim.o brick.o manifold.o bspline2d.o meshsurf.o csgeom.o polyhedra.o curve2d.o singularref.o edgeflw.o solid.o explicitcurve2d.o specpoin.o gencyl.o revolution.o genmesh.o genmesh2d.o spline3d.o surface.o identify.o triapprox.o meshstlsurface.o stlline.o stltopology.o stltool.o stlgeom.o stlgeomchart.o stlgeommesh.o table.o optmem.o spbita2d.o hashtabl.o sort.o flags.o seti.o bitarray.o array.o symbolta.o mystring.o moveablemem.o spline.o splinegeometry.o ngnewdelete.o nglib.o hprefinement.o Partition_Inter2d.o Partition_Loop.o Partition_Loop3d.o Partition_Inter3d.o Partition_Loop2d.o Partition_Spliter.o occgeom.o occgenmesh.o occmeshsurf.o -o libNETGEN.so;
            rm -rf linopt.o bfgs.o linsearch.o global.o bisect.o meshtool.o refine.o ruler3.o improve3.o adfront3.o tetrarls.o prism2rls.o profiler.o pyramidrls.o pyramid2rls.o netrule3.o ruler2.o meshclass.o improve2.o adfront2.o netrule2.o triarls.o geomsearch.o secondorder.o meshtype.o parser3.o quadrls.o specials.o parser2.o meshing2.o meshing3.o meshfunc.o localh.o improve2gen.o delaunay.o boundarylayer.o msghandler.o meshfunc2d.o smoothing2.o smoothing3.o topology.o curvedelems_new.o clusters.o zrefine.o ngexception.o geomtest3d.o geom2d.o geom2dmesh.o geom3d.o adtree.o transform3d.o geomfuncs.o polynomial.o densemat.o vector.o basemat.o sparsmat.o algprim.o brick.o manifold.o bspline2d.o meshsurf.o csgeom.o polyhedra.o curve2d.o singularref.o edgeflw.o solid.o explicitcurve2d.o specpoin.o gencyl.o revolution.o genmesh.o genmesh2d.o spline3d.o surface.o identify.o triapprox.o meshstlsurface.o stlline.o stltopology.o stltool.o stlgeom.o stlgeomchart.o stlgeommesh.o table.o optmem.o spbita2d.o hashtabl.o sort.o flags.o seti.o bitarray.o array.o symbolta.o mystring.o moveablemem.o spline.o splinegeometry.o ngnewdelete.o nglib.o hprefinement.o Partition_Inter2d.o Partition_Loop.o Partition_Loop3d.o Partition_Inter3d.o Partition_Loop2d.o Partition_Spliter.o occgeom.o occgenmesh.o occmeshsurf.o csgparser.o dynamicmem.o extrusion.o occconstruction.o parthreads.o readuser.o writeabaqus.o writediffpack.o writeelmer.o writefeap.o writefluent.o writegmsh.o writejcm.o writepermas.o writetecplot.o writetochnog.o writeuser.o wuchemnitz.o,
            Netgen_ok=no)

    AC_CACHE_VAL(salome_cv_netgen_lib,[
                 AC_TRY_LINK([
                     #include <iostream>
                     #include <fstream>
                     namespace nglib {
                     #include "nglib.h"
                     }
                     #define OCCGEOMETRY
                     #include <occgeom.hpp>
                  ],[
                     nglib::Ng_Init();
                     netgen::OCCGeometry occgeo;
                     nglib::Ng_Exit();
                  ],
                  [eval "salome_cv_netgen_lib=yes";rm -rf libNETGEN.so],
                  [eval "salome_cv_netgen_lib=no";rm -rf libNETGEN.so])
    ])
    Netgen_ok="$salome_cv_netgen_lib"


    LDFLAGS="$LDFLAGS_old"
  fi

  CPPFLAGS="$CPPFLAGS_old"
  CXXFLAGS="$CXXFLAGS_old"

  if test "x$Netgen_ok" == xno ; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN(Netgen libraries not found or not properly installed)
  else
    AC_MSG_RESULT(yes)
  fi
fi

AC_SUBST(NETGEN_INCLUDES)
AC_SUBST(NETGEN_LIBS_DIR)

AC_LANG_RESTORE

])dnl
