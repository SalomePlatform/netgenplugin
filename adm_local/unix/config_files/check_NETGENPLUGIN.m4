dnl Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
dnl
dnl Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
dnl CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl

#------------------------------------------------------------
#  Check availability of Salome NETGEN mesh plugin module
#   distribution
#------------------------------------------------------------

AC_DEFUN([CHECK_NETGENPLUGIN],[

AC_CHECKING(for NETGEN mesh plugin)

NGplugin_ok=no

NETGENPLUGIN_LDLAGS=""
NETGENPLUGIN_CXXFLAGS=""

AC_ARG_WITH(netgenplugin,
	    [  --with-netgenplugin=DIR root directory path of NETGEN mesh plugin installation ],
	    NETGENPLUGIN_DIR="$withval",NETGENPLUGIN_DIR="")

if test "x$NETGENPLUGIN_DIR" == "x" ; then

# no --with-netgenplugin-dir option used

   if test "x$NETGENPLUGIN_ROOT_DIR" != "x" ; then

    # NETGENPLUGIN_ROOT_DIR environment variable defined
      NETGENPLUGIN_DIR=$NETGENPLUGIN_ROOT_DIR

   fi
# 
fi

if test -f ${NETGENPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome/libNETGENEngine.so ; then
   NGplugin_ok=yes
   AC_MSG_RESULT(Using NETGEN mesh plugin distribution in ${NETGENPLUGIN_DIR})

   if test "x$NETGENPLUGIN_ROOT_DIR" == "x" ; then
      NETGENPLUGIN_ROOT_DIR=${NETGENPLUGIN_DIR}
   fi
   AC_SUBST(NETGENPLUGIN_ROOT_DIR)

   NETGENPLUGIN_LDFLAGS=-L${NETGENPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
   NETGENPLUGIN_CXXFLAGS=-I${NETGENPLUGIN_DIR}/include/salome

   AC_SUBST(NETGENPLUGIN_LDFLAGS)
   AC_SUBST(NETGENPLUGIN_CXXFLAGS)

else
   AC_MSG_WARN("Cannot find compiled NETGEN mesh plugin distribution")
fi

AC_MSG_RESULT(for NETGEN mesh plugin: $NGplugin_ok)
 
])dnl
