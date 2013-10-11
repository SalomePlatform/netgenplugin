# - Find NETGEN
# Sets the following variables:
#   NETGEN_INCLUDE_DIRS - path to the NETGEN include directory
#   NETGEN_LIBRARIES    - path to the NETGEN libraries to be linked against
#

#########################################################################
# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

# ------

MESSAGE(STATUS "Check for NETGEN ...")

# ------

SET(NETGEN_ROOT_DIR $ENV{NETGEN_ROOT_DIR})

IF(NETGEN_ROOT_DIR)
 LIST(APPEND CMAKE_PREFIX_PATH "${NETGEN_ROOT_DIR}")
ENDIF(NETGEN_ROOT_DIR)

FIND_PATH(NETGEN_INCLUDE_DIRS nglib.h)
FIND_LIBRARY(NETGEN_LIBRARIES NAMES nglib)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NETGEN REQUIRED_VARS NETGEN_INCLUDE_DIRS NETGEN_LIBRARIES)
