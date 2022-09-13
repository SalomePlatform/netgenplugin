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

//  File   : netgen_param.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#ifndef _NETGEN_PARAM_HXX_
#define _NETGEN_PARAM_HXX_

#include <string>

class NETGENPlugin_Hypothesis;
class StdMeshers_MaxElementVolume;
class StdMeshers_ViscousLayers;

struct netgen_params{
  // Params from NETGENPlugin_Mesher
  // True if _hypParameters is not null
  bool has_netgen_param=true;
  double maxh;
  double minh;
  double segmentsperedge;
  double grading;
  double curvaturesafety;
  int secondorder;
  int quad;
  bool optimize;
  int fineness;
  bool uselocalh;
  bool merge_solids;
  double chordalError;
  int optsteps2d;
  int optsteps3d;
  double elsizeweight;
  int opterrpow;
  bool delaunay;
  bool checkoverlap;
  bool checkchartboundary;
  int closeedgefac;


  // TODO: add localsize (pass through local size file ?)
  // True if we have a mesh size file or local size info
  bool has_local_size = false;
  std::string meshsizefilename;

  // Params from NETGEN3D
  // True if _hypMaxElementVolume is not null
  bool has_maxelementvolume_hyp=false;
  double maxElementVolume=0.0;

  // to replace
  //NETGENPlugin_Hypothesis *    _hypParameters=nullptr;
  // to remove ?
  StdMeshers_MaxElementVolume* _hypMaxElementVolume=nullptr;
  // to remove ?
  StdMeshers_ViscousLayers*    _viscousLayersHyp=nullptr;
  //double                       _progressByTic;
  bool _quadraticMesh=false;

  // Params from NETGEN2D
  bool has_LengthFromEdges_hyp=false;

  // Number of threads for the mesher
  int nbThreads;
};

void print_netgen_params(netgen_params& aParams);

void import_netgen_params(const std::string param_file, netgen_params& aParams);
void export_netgen_params(const std::string param_file, netgen_params& aParams);

bool diff_netgen_params(netgen_params params1, netgen_params params2);

#endif