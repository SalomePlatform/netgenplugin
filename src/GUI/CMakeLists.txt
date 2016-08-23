# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${CAS_INCLUDE_DIRS}
  ${NETGEN_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${PYTHON_INCLUDES}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${PROJECT_SOURCE_DIR}/src/NETGENPlugin
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_MT_LIBS}
  ${GUI_SalomeApp}
  ${GUI_qtx}
  ${GUI_suit}
  ${GUI_SalomeObject}
  ${GUI_LightApp}
  ${SMESH_SMESH}
  ${SMESH_PluginUtils}
  ${CAS_KERNEL}
  SalomeIDLNETGENPLUGIN
  NETGENEngine
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  NETGENPluginGUI_HypothesisCreator.h
  NETGENPluginGUI_SimpleCreator.h
)

# header files / no moc processed
SET(_other_HEADERS
  NETGENPluginGUI.h
)

SET(NETGENPluginGUI_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  NETGENPluginGUI.cxx
  NETGENPluginGUI_HypothesisCreator.cxx
  NETGENPluginGUI_SimpleCreator.cxx
)

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  NETGENPlugin_images.ts
  NETGENPlugin_msg_en.ts
  NETGENPlugin_msg_fr.ts
  NETGENPlugin_msg_ja.ts
) 

# sources / to compile
SET(NETGENPluginGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- rules ---

ADD_LIBRARY(NETGENPluginGUI ${NETGENPluginGUI_SOURCES})
TARGET_LINK_LIBRARIES(NETGENPluginGUI ${_link_LIBRARIES} )
INSTALL(TARGETS NETGENPluginGUI EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${NETGENPluginGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_NETGENPLUGIN_INSTALL_RES_DATA}")
