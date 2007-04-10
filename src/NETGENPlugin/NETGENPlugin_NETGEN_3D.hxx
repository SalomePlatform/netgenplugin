// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS, L3S, LJLL, MENSI
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful
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
// Copyright : CEA 2003
// $Header$
//=============================================================================

#ifndef _NETGENPlugin_NETGEN_3D_HXX_
#define _NETGENPlugin_NETGEN_3D_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "StdMeshers_MaxElementVolume.hxx"
#include "Utils_SALOME_Exception.hxx"

class NETGENPlugin_NETGEN_3D: public SMESH_3D_Algo
{
public:
  NETGENPlugin_NETGEN_3D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~NETGENPlugin_NETGEN_3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

protected:
  double _maxElementVolume;

  const StdMeshers_MaxElementVolume* _hypMaxElementVolume;
};

#endif
