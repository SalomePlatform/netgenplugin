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
