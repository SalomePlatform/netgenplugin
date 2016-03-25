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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${NETGEN_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(  
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${NETGEN_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${NETGEN_LIBRARIES}
  ${CAS_TKernel}
  ${CAS_TKBRep}
  ${CAS_TKShHealing}
  ${CAS_TKSTEP}
  ${CAS_TKXSBase}
  ${CAS_TKIGES}
  ${CAS_TKMesh}
  ${CAS_TKSTL}
  ${CAS_TKG3d}
  ${CAS_TKTopAlgo}
  ${CAS_TKG2d}
  ${CAS_TKBool}
  ${CAS_TKGeomAlgo}
  ${CAS_TKOffset}
  ${CAS_TKGeomBase}
  ${CAS_TKBO}
  ${CAS_TKMath}
  ${CAS_TKFillet}
  ${CAS_TKMeshVS}
  ${CAS_TKPrim}
  ${CAS_TKSTEPBase}
  ${CAS_TKSTEPAttr}
  ${CAS_TKSTEP209}
  ${CAS_TKXDESTEP}
  ${CAS_TKXDEIGES}
  ${CAS_TKXCAF}
  ${CAS_TKLCAF}
  ${CAS_FWOSPlugin}
  ${GEOM_GEOMbasic}
  ${SMESH_SMESHimpl}
  ${SMESH_SMESHEngine}
  ${SMESH_SMESHUtils}
  ${SMESH_StdMeshersEngine}
  ${SMESH_StdMeshers}
  ${SMESH_SMESHDS}
  ${SMESH_SMDS}
  ${SMESH_SMESHControls}
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SalomeNS}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_OpUtil}
  SalomeIDLNETGENPLUGIN
)

# --- headers ---

# header files
SET(NETGENEngine_HEADERS
  NETGENPlugin_NETGEN_3D.hxx 
  NETGENPlugin_NETGEN_3D_i.hxx 
  NETGENPlugin_NETGEN_2D.hxx 
  NETGENPlugin_NETGEN_2D_i.hxx 
  NETGENPlugin_NETGEN_2D3D.hxx 
  NETGENPlugin_NETGEN_2D3D_i.hxx 
  NETGENPlugin_NETGEN_2D_ONLY.hxx 
  NETGENPlugin_NETGEN_2D_ONLY_i.hxx 
  NETGENPlugin_Hypothesis.hxx 
  NETGENPlugin_Hypothesis_i.hxx 
  NETGENPlugin_Hypothesis_2D.hxx 
  NETGENPlugin_Hypothesis_2D_i.hxx 
  NETGENPlugin_Hypothesis_3D_i.hxx 
  NETGENPlugin_Hypothesis_2D_ONLY_i.hxx 
  NETGENPlugin_SimpleHypothesis_2D.hxx 
  NETGENPlugin_SimpleHypothesis_3D.hxx 
  NETGENPlugin_SimpleHypothesis_2D_i.hxx 
  NETGENPlugin_SimpleHypothesis_3D_i.hxx 
  NETGENPlugin_Mesher.hxx 
  NETGENPlugin_Defs.hxx
)

# --- sources ---

# sources / static
SET(NETGENEngine_SOURCES
  NETGENPlugin_NETGEN_3D.cxx
  NETGENPlugin_NETGEN_3D_i.cxx
  NETGENPlugin_Mesher.cxx
  NETGENPlugin_NETGEN_2D.cxx
  NETGENPlugin_NETGEN_2D_i.cxx
  NETGENPlugin_NETGEN_2D3D.cxx
  NETGENPlugin_NETGEN_2D3D_i.cxx
  NETGENPlugin_NETGEN_2D_ONLY.cxx
  NETGENPlugin_NETGEN_2D_ONLY_i.cxx
  NETGENPlugin_Hypothesis.cxx
  NETGENPlugin_Hypothesis_i.cxx
  NETGENPlugin_Hypothesis_2D.cxx
  NETGENPlugin_Hypothesis_2D_i.cxx
  NETGENPlugin_Hypothesis_3D_i.cxx
  NETGENPlugin_Hypothesis_2D_ONLY_i.cxx
  NETGENPlugin_SimpleHypothesis_2D.cxx
  NETGENPlugin_SimpleHypothesis_3D.cxx
  NETGENPlugin_SimpleHypothesis_2D_i.cxx
  NETGENPlugin_SimpleHypothesis_3D_i.cxx
  NETGENPlugin_i.cxx
)

# --- scripts ---

# scripts / static
SET(_bin_SCRIPTS
  __init__.py
  NETGENPluginBuilder.py
)

# --- rules ---

ADD_LIBRARY(NETGENEngine ${NETGENEngine_SOURCES})
TARGET_LINK_LIBRARIES(NETGENEngine ${_link_LIBRARIES} )
INSTALL(TARGETS NETGENEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${NETGENEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

SALOME_INSTALL_SCRIPTS("${_bin_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/NETGENPlugin)
