// Copyright (C) 2007-2022  CEA/DEN, EDF R&D, OPEN CASCADE
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

//=============================================================================
// File      : NETGENPlugin_NETGEN_3D.hxx
//             Moved here from SMESH_NETGEN_3D.hxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
//=============================================================================
//
#ifndef _NETGENPlugin_NETGEN_3D_HXX_
#define _NETGENPlugin_NETGEN_3D_HXX_

#include "NETGENPlugin_Defs.hxx"
#include "NETGENPlugin_Mesher.hxx"

#include "SMESH_Algo.hxx"
#include "Utils_SALOME_Exception.hxx"

#include <vector>
#include <tuple>

class StdMeshers_ViscousLayers;
class StdMeshers_MaxElementVolume;
class NETGENPlugin_Hypothesis;
class NETGENPlugin_NetgenLibWrapper;
class netgen_params;
class SMDS_MeshNode;

class NETGENPLUGIN_EXPORT NETGENPlugin_NETGEN_3D: public SMESH_3D_Algo
{
 public:
  NETGENPlugin_NETGEN_3D(int hypId, SMESH_Gen* gen);
  virtual ~NETGENPlugin_NETGEN_3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
                       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
                       SMESH_MesherHelper* aHelper);

  virtual void CancelCompute();

  virtual double GetProgress() const;

  virtual bool Evaluate(SMESH_Mesh& aMesh,
                        const TopoDS_Shape& aShape,
                        MapShapeNbElems& aResMap);

  bool computeFillNgMesh(
    SMESH_Mesh&         aMesh,
    const TopoDS_Shape& aShape,
    std::vector< const SMDS_MeshNode* > &nodeVec,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    int &Netgen_NbOfNodes);

  bool computePrepareParam(
    SMESH_Mesh&         aMesh,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    netgen::OCCGeometry &occgeo,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    int &endWith);

  bool computeRunMesher(
    netgen::OCCGeometry &occgeo,
    std::vector< const SMDS_MeshNode* > &nodeVec,
    netgen::Mesh* ngMesh,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    netgen_params &aParams,
    int &startWith, int &endWith);

  bool computeFillMesh(
    std::vector< const SMDS_MeshNode* > &nodeVec,
    NETGENPlugin_NetgenLibWrapper &ngLib,
    SMESH_MesherHelper &helper,
    int &Netgen_NbOfNodes);

 protected:

  virtual bool getSurfaceElements(
    SMESH_Mesh&         aMesh,
    const TopoDS_Shape& aShape,
    SMESH_ProxyMesh::Ptr proxyMesh,
    NETGENPlugin_Internals &internals,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    std::map<const SMDS_MeshElement*, std::tuple<bool, bool>>& listElements);

  bool compute(SMESH_Mesh&                          mesh,
               SMESH_MesherHelper&                  helper,
               std::vector< const SMDS_MeshNode* >& nodeVec,
               NETGENPlugin_NetgenLibWrapper&       ngLib);

  double _maxElementVolume;

  const NETGENPlugin_Hypothesis *    _hypParameters;
  const StdMeshers_MaxElementVolume* _hypMaxElementVolume;
  const StdMeshers_ViscousLayers*    _viscousLayersHyp;
  double                             _progressByTic;
};

#endif
