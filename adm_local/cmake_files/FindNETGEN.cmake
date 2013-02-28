# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

FIND_LIBRARY(NETGEN_LIB_nglib nglib PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)

# temporary, check to be added later
SET(NETGEN_V5 OFF)

IF(NETGEN_V5)
  SET(NETGEN_INCLUDES ${NETGEN_INCLUDES} -DNETGEN_V5)
ENDIF(NETGEN_V5)

SET(NETGEN_LIBS)
IF(WINDOWS)
  FIND_LIBRARY(NETGEN_LIB_csg csg PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_gen gen PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_geom2d geom2d PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_gprim gprim PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_interface interface PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_la la PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_mesh mesh PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_occ occ PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  FIND_LIBRARY(NETGEN_LIB_stl stl PATHS ${NETGENHOME}/lib ${NETGENHOME}/lib/LINUX)
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_csg})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_gen})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_geom2d})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_gprim})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_interface})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_la})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_mesh})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_occ})
  SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_stl})
ENDIF(WINDOWS)
SET(NETGEN_LIBS ${NETGEN_LIBS} ${NETGEN_LIB_nglib})
