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
// File      : NETGENPlugin_Mesher.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 31/03/2006
// Project   : SALOME
// $Header$
//=============================================================================

#ifndef _NETGENPlugin_Mesher_HXX_
#define _NETGENPlugin_Mesher_HXX_

class SMESHDS_Mesh;
class TopoDS_Shape;
class NETGENPlugin_Hypothesis;

/*!
 * \brief This class calls the NETGEN mesher of OCC geometry
 */

class NETGENPlugin_Mesher 
{
 public:
  // ---------- PUBLIC METHODS ----------

  NETGENPlugin_Mesher (SMESHDS_Mesh* meshDS, const TopoDS_Shape& aShape,
                       const bool isVolume);

  void SetParameters(const NETGENPlugin_Hypothesis* hyp);

  bool Compute();

 private:
  SMESHDS_Mesh*        _meshDS;
  const TopoDS_Shape&  _shape;
  bool                 _isVolume;
  bool                 _optimize;
};

#endif
