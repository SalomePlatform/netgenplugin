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

//  File   : netgen_mesher.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#ifndef _NETGEN_MESHER_HXX_
#define _NETGEN_MESHER_HXX_

#include <string>
#include <iostream>

class TopoDS_Shape;
class SMESH_Mesh;
class SMESH_Comment;
class netgen_params;

int netgen2d_internal(TopoDS_Shape &aShape,
             SMESH_Mesh& aMesh,
             netgen_params& aParams,
             std::string new_element_file,
             std::string element_orientation_file,
             bool output_mesh);
int netgen2d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orienation_file,
             const std::string new_element_file,
             bool output_mesh,
             const std::string output_mesh_file);

int netgen3d_internal(TopoDS_Shape &aShape,
             SMESH_Mesh& aMesh,
             netgen_params& aParams,
             std::string new_element_file,
             std::string element_orientation_file,
             bool output_mesh);
int netgen3d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orienation_file,
             const std::string new_element_file,
             bool output_mesh,
             const std::string output_mesh_file,
             int nbThreads);

//TODO: Tmp function replace by real error handling
int error(int error_type, std::string msg);
int error(const SMESH_Comment& comment);

#endif