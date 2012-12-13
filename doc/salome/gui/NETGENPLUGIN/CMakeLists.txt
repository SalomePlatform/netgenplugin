# Copyright (C) 2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(${KERNEL_ROOT_DIR}/salome_adm/cmake_files/SalomeMacros.cmake)

SET(top_builddir ${CMAKE_BINARY_DIR})
SET(top_srcdir ${CMAKE_SOURCE_DIR})
SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(builddir ${CMAKE_CURRENT_BINARY_DIR})
SET(datadir${CMAKE_INSTALL_PREFIX}/share)
SET(docdir ${datadir}/doc/salome)
SET(guidocdir ${docdir}/gui/NETGENPLUGIN)

SALOME_CONFIGURE_FILE(doxyfile.in doxyfile)
SALOME_CONFIGURE_FILE(doxyfile_py.in doxyfile_py)
SALOME_CONFIGURE_FILE(static/header.html.in ${builddir}/static/header.html)
SALOME_CONFIGURE_FILE(static/header_py.html.in ${builddir}/static/header_py.html)

SET(DOC_SMESH_MeshersList NETGENPlugin)
SET(f "$(SMESH_ROOT_DIR)/bin/salome/collect_mesh_methods.py")
IF(WINDOWS)
  STRING(REPLACE "/" "\\" f ${f})
  STRING(REPLACE "/" "\\" SCR "@SET PYTHONPATH=${OMNIORB_ROOT_USER}/lib/x86_win32\;%PYTHONPATH%
  @SET PYTHONPATH=${OMNIORB_ROOT_USER}/lib/python\;%PYTHONPATH%
  @SET PATH=${OMNIORB_ROOT_USER}/lib/x86_win32\;%PATH%
  @SET PATH=$ENV{KERNEL_ROOT_DIR}/lib/salome\;%PATH%
  @SET PYTHONPATH=$ENV{KERNEL_ROOT_DIR}/bin/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{KERNEL_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{MED_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{MED_ROOT_DIR}/bin/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{GEOM_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{GEOM_ROOT_DIR}/bin/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{SMESH_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome\;%PYTHONPATH%
  @SET PYTHONPATH=$ENV{SMESH_ROOT_DIR}/bin/salome\;%PYTHONPATH%
  @SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/python${PYTHON_VERSION}/site-packages/salome\;%PYTHONPATH%
  @SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}/bin/salome\;%PYTHONPATH%
  @SET SMESH_MeshersList=${DOC_SMESH_MeshersList}
  ")
  SET(EXT "bat")
  SET(CALL_STR "call")
ELSE(WINDOWS)
  SET(DOC_PYTHONPATH ${CMAKE_INSTALL_PREFIX}/bin/salome:${SMESH_ROOT_DIR}/bin/salome:${SMESH_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome:${MED_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome:${GEOM_ROOT_DIR}/bin/salome:${GEOM_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome:${KERNEL_ROOT_DIR}/bin/salome:${KERNEL_ROOT_DIR}/lib/python${PYTHON_VERSION}/site-packages/salome:${OMNIORB_ROOT_USER}/lib/python${PYTHON_VERSION}/site-packages:${OMNIORB_ROOT_USER}/lib64/python${PYTHON_VERSION}/site-packages)
  SET(SCR "export PYTHONPATH=${DOC_PYTHONPATH}:${PYTHONPATH}
  export SMESH_MeshersList=${DOC_SMESH_MeshersList}
  ")
  SET(EXT "sh")
  SET(CALL_STR ".")
ENDIF(WINDOWS)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tmp_env.${EXT} "${SCR}")

ADD_CUSTOM_TARGET(usr_docs ${CALL_STR} ${CMAKE_CURRENT_BINARY_DIR}/tmp_env.${EXT} && ${PYTHON_EXECUTABLE} ${f} -d -o smesh.py NETGENPlugin
  COMMAND ${DOXYGEN_EXECUTABLE} doxyfile_py
  COMMAND ${DOXYGEN_EXECUTABLE} doxyfile
  COMMAND ${PYTHON_EXECUTABLE} -c "import os; os.remove(r'''smesh.py'''); os.remove(r'''tmp_env.${EXT}''')"
  COMMAND ${PYTHON_EXECUTABLE} -c "import shutil, sys; shutil.rmtree(r'''${CMAKE_INSTALL_PREFIX}/share/doc/salome/gui/NETGENPLUGIN''',True); shutil.copytree(r'''${CMAKE_CURRENT_BINARY_DIR}''',r'''${CMAKE_INSTALL_PREFIX}/share/doc/salome/gui/NETGENPLUGIN''', ignore=shutil.ignore_patterns('*usr_docs*', '*CMakeFiles*', '*.cmake', 'doxyfile*', '*.vcproj', 'static', 'Makefile*')); shutil.copy(r'''${CMAKE_CURRENT_SOURCE_DIR}/images/head.png''',r'''${CMAKE_INSTALL_PREFIX}/share/doc/salome/gui/NETGENPLUGIN'''); shutil.copy(r'''${CMAKE_CURRENT_SOURCE_DIR}/images/head.png''',r'''${CMAKE_INSTALL_PREFIX}/share/doc/salome/gui/NETGENPLUGIN/netgenpluginpy_doc''')"
  VERBATIM 
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}             
)
