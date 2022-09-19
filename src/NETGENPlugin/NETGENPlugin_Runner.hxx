// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  File   : NETGENPlugin_Runner.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : NETGEN
//

#ifndef _NETGENPLUGIN_RUNNER_HXX_
#define _NETGENPLUGIN_RUNNER_HXX_

#include <string>
#include <iostream>

#include "NETGENPlugin_Defs.hxx"
#include "NETGENPlugin_Mesher.hxx"

#include "SMESH_Algo.hxx"
#include "Utils_SALOME_Exception.hxx"

class TopoDS_Shape;
class SMESH_Mesh;
class SMESH_Comment;
class netgen_params;
class NETGENPlugin_NetgenLibWrapper;
class SMDS_MeshNode;

// Netgen 2d functions
int netgen2dInternal(TopoDS_Shape &aShape,
             SMESH_Mesh& aMesh,
             netgen_params& aParams,
             std::string new_element_file,
             std::string element_orientation_file,
             bool output_mesh);
int netgen2d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orientation_file,
             const std::string new_element_file,
             const std::string output_mesh_file);

// Netgen 3D functions
bool mycomputeFillNgMesh(
    SMESH_Mesh&         aMesh,
    const TopoDS_Shape& aShape,
    std::vector< const SMDS_MeshNode* > &nodeVec,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    std::string element_orientation_file,
    int &Netgen_NbOfNodes);

bool mycomputePrepareParam(
    SMESH_Mesh&         aMesh,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    netgen::OCCGeometry &occgeo,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    int &endWith);

bool mycomputeRunMesher(
    netgen::OCCGeometry &occgeo,
    std::vector< const SMDS_MeshNode* > &nodeVec,
    netgen::Mesh* ngMesh,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    int &startWith, int &endWith);

bool mycomputeFillNewElementFile(
    std::vector< const SMDS_MeshNode* > &nodeVec,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    std::string new_element_file,
    int &Netgen_NbOfNodes);

bool mycomputeFillMesh(
    std::vector< const SMDS_MeshNode* > &nodeVec,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    SMESH_MesherHelper &helper,
    int &Netgen_NbOfNodes);

int netgen3dInternal(TopoDS_Shape &aShape,
             SMESH_Mesh& aMesh,
             netgen_params& aParams,
             std::string new_element_file,
             std::string element_orientation_file,
             bool output_mesh);
int netgen3d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orientation_file,
             const std::string new_element_file,
             const std::string output_mesh_file,
             int nbThreads);

//TODO: Tmp function replace by real error handling
int error(int error_type, std::string msg);
int error(const SMESH_Comment& comment);

#endif