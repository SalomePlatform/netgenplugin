//  NETGENPlugin : C++ implementation
//
//  Copyright (C) 2006  OPEN CASCADE, CEA/DEN, EDF R&D
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
// File      : NETGENPlugin_NETGEN_2D3D.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 20/03/2006
// Project   : SALOME
// $Header$
//=============================================================================

#ifndef _NETGENPlugin_NETGEN_2D3D_HXX_
#define _NETGENPlugin_NETGEN_2D3D_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "StdMeshers_MaxElementVolume.hxx"
#include "Utils_SALOME_Exception.hxx"

class NETGENPlugin_Hypothesis;

class NETGENPlugin_NETGEN_2D3D: public SMESH_3D_Algo
{
public:
  NETGENPlugin_NETGEN_2D3D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~NETGENPlugin_NETGEN_2D3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, NETGENPlugin_NETGEN_2D3D & hyp);
  friend istream & operator >> (istream & load, NETGENPlugin_NETGEN_2D3D & hyp);

protected:
  const NETGENPlugin_Hypothesis* _hypothesis;
};

#endif