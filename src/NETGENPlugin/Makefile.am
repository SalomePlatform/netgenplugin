#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# -* Makefile *- 
# Author : Edward AGAPOV (OCC)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
# Module : NETGENPLUGIN
# Date : 10/01/2004
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS =

# Libraries targets
lib_LTLIBRARIES = libNETGENEngine.la

dist_libNETGENEngine_la_SOURCES = \
	NETGENPlugin_NETGEN_3D.cxx \
	NETGENPlugin_NETGEN_3D_i.cxx \
	NETGENPlugin_NETGEN_2D.cxx \
	NETGENPlugin_NETGEN_2D_i.cxx \
	NETGENPlugin_NETGEN_2D3D.cxx \
	NETGENPlugin_NETGEN_2D3D_i.cxx \
	NETGENPlugin_NETGEN_2D_ONLY.cxx \
	NETGENPlugin_NETGEN_2D_ONLY_i.cxx \
	NETGENPlugin_Hypothesis.cxx \
	NETGENPlugin_Hypothesis_i.cxx \
	NETGENPlugin_Hypothesis_2D.cxx \
	NETGENPlugin_Hypothesis_2D_i.cxx \
	NETGENPlugin_Mesher.cxx \
	NETGENPlugin_SimpleHypothesis_2D.cxx \
	NETGENPlugin_SimpleHypothesis_3D.cxx \
	NETGENPlugin_SimpleHypothesis_2D_i.cxx \
	NETGENPlugin_SimpleHypothesis_3D_i.cxx \
	NETGENPlugin_i.cxx

libNETGENEngine_la_CPPFLAGS = \
        $(KERNEL_CXXFLAGS) \
        $(GUI_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
	$(NETGEN_INCLUDES) \
	$(SMESH_CXXFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(top_builddir)/idl \
	-I$(top_builddir)/salome_adm/unix

libNETGENEngine_la_LDFLAGS  = \
	../NETGEN/libNETGEN.la \
	../../idl/libSalomeIDLNETGENPLUGIN.la \
	$(SMESH_LDFLAGS) -lSMESHimpl -lSMESHEngine -lStdMeshersEngine \
	$(KERNEL_LDFLAGS) -lSalomeGenericObj
