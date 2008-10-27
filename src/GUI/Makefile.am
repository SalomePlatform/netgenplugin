#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS, L3S, LJLL, MENSI
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
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#  File   : Makefile.in
#  Author : Michael Sazonov
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : NETGENPLUGIN
#  $Header$

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS =

# Libraries targets
lib_LTLIBRARIES = libNETGENPluginGUI.la

dist_libNETGENPluginGUI_la_SOURCES = \
	NETGENPluginGUI.cxx \
	NETGENPluginGUI.h \
	NETGENPluginGUI_HypothesisCreator.h \
	NETGENPluginGUI_HypothesisCreator.cxx \
	NETGENPluginGUI_SimpleCreator.h \
	NETGENPluginGUI_SimpleCreator.cxx

MOC_FILES = \
	NETGENPluginGUI_HypothesisCreator_moc.cxx \
	NETGENPluginGUI_SimpleCreator_moc.cxx

nodist_libNETGENPluginGUI_la_SOURCES= \
	$(MOC_FILES)

# additionnal information to compil and link file

libNETGENPluginGUI_la_CPPFLAGS = \
	$(QT_INCLUDES) \
	$(CAS_CPPFLAGS) \
	$(PYTHON_INCLUDES) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(SMESH_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	-I$(srcdir)/../NETGENPlugin \
	-I$(top_builddir)/idl \
	-I$(top_builddir)/salome_adm/unix

libNETGENPluginGUI_la_LDFLAGS  = \
	../NETGENPlugin/libNETGENEngine.la \
	${SMESH_LDFLAGS} -lSMESH \
	$(CAS_KERNEL)

# resources files
nodist_salomeres_DATA= \
	NETGENPlugin_images.qm \
	NETGENPlugin_msg_en.qm

