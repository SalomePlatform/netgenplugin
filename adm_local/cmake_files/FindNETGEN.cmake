# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

SET(NETGENHOME $ENV{NETGENHOME})
FIND_PATH(NETGEN_INCLUDES_DIR nglib.h ${NETGENHOME}/include)
SET(NETGEN_INCLUDES)
SET(NETGEN_INCLUDES ${NETGEN_INCLUDES} -I${NETGEN_INCLUDES_DIR} -I${NETGENHOME}/share/netgen/include)
SET(NETGEN_INCLUDES ${NETGEN_INCLUDES} -DNO_PARALLEL_THREADS -DOCCGEOMETRY)

SET(NETGEN_LIBS)
FIND_LIBRARY(NETGEN_LIB_csg csg PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_gen gen PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_geom2d geom2d PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_gprim gprim PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_interface interface PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_la la PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_mesh mesh PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_occ occ PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_stl stl PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
FIND_LIBRARY(NETGEN_LIB_nglib nglib PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)

FOREACH(LIBNAME 
    ${NETGEN_LIB_csg} 
    ${NETGEN_LIB_gen}
    ${NETGEN_LIB_geom2d} 
    ${NETGEN_LIB_gprim}
    ${NETGEN_LIB_interface} 
    ${NETGEN_LIB_la}
    ${NETGEN_LIB_mesh} 
    ${NETGEN_LIB_occ} 
    ${NETGEN_LIB_stl} 
    ${NETGEN_LIB_nglib}
    )
  IF(LIBNAME)
    SET(NETGEN_LIBS ${NETGEN_LIBS} ${LIBNAME})
  ENDIF(LIBNAME)
ENDFOREACH(LIBNAME )

# Check Netgen version
SET(NETGEN_V5 OFF)
SET(tmp_check_netgen ${CMAKE_BINARY_DIR}/tmp_check_netgen.cxx)
FILE(WRITE ${tmp_check_netgen}
  "#include <iostream>      \n"
  "#include <fstream>       \n"
  "namespace nglib {        \n"
  "#include \"nglib.h\"     \n"
  "}                        \n"
  "#include <occgeom.hpp>   \n"
  "int main() {             \n"
  "nglib::Ng_Init();        \n"
  "netgen::Mesh* ngMesh;    \n"
  "ngMesh->CalcLocalH(1.0); \n"
  "nglib::Ng_Exit();        \n" 
  "return 0;                \n"
  "}                        \n"
  )
IF(WINDOWS)
 STRING(REPLACE "\\" "/" CAS_CPPFLAGS_TMP ${CAS_CPPFLAGS})
 STRING(REPLACE "\\" "/" NETGEN_INCLUDES_TMP ${NETGEN_INCLUDES})
ENDIF(WINDOWS)

TRY_COMPILE(NETGEN_V5
  ${CMAKE_BINARY_DIR}
  ${tmp_check_netgen}
  CMAKE_FLAGS "-DLINK_LIBRARIES:STRING=${NETGEN_LIB_nglib}"
  COMPILE_DEFINITIONS ${CAS_CPPFLAGS_TMP} ${NETGEN_INCLUDES_TMP}
  OUTPUT_VARIABLE OUTPUT
  )
FILE(REMOVE ${tmp_check_netgen})

IF(NETGEN_V5)
  SET(NETGEN_INCLUDES ${NETGEN_INCLUDES} -DNETGEN_V5)
ENDIF(NETGEN_V5)
