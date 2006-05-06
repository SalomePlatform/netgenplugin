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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
// File      : NETGENPlugin_Hypothesis_2D.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 28/03/2006
// Project   : SALOME
// $Header$
//=============================================================================

using namespace std;
#include <NETGENPlugin_Hypothesis_2D.hxx>
#include <utilities.h>

//=============================================================================
/*!
 *  
 */
//=============================================================================
NETGENPlugin_Hypothesis_2D::NETGENPlugin_Hypothesis_2D (int hypId, int studyId,
                                                        SMESH_Gen * gen)
  : NETGENPlugin_Hypothesis(hypId, studyId, gen),
    _quadAllowed (GetDefaultQuadAllowed())
{
  _name = "NETGEN_Parameters_2D";
  _param_algo_dim = 2;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
void NETGENPlugin_Hypothesis_2D::SetQuadAllowed(bool theVal)
{
  if (theVal != _quadAllowed)
  {
    _quadAllowed = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
bool NETGENPlugin_Hypothesis_2D::GetDefaultQuadAllowed()
{
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
ostream & NETGENPlugin_Hypothesis_2D::SaveTo(ostream & save)
{
  NETGENPlugin_Hypothesis::SaveTo(save);

  save << " " << (int)_quadAllowed;

  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
istream & NETGENPlugin_Hypothesis_2D::LoadFrom(istream & load)
{
  NETGENPlugin_Hypothesis::LoadFrom(load);

  bool isOK = true;
  int is;

  isOK = (load >> is);
  if (isOK)
    _quadAllowed = (bool) is;
  else
    load.clear(ios::badbit | load.rdstate());

  return load;
}
