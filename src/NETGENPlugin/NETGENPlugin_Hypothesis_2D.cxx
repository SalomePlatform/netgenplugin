// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  NETGENPlugin : C++ implementation
// File      : NETGENPlugin_Hypothesis_2D.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 28/03/2006
// Project   : SALOME
//=============================================================================
//
#include "NETGENPlugin_Hypothesis_2D.hxx"

using namespace std;

//=============================================================================
/*!
 *  
 */
//=============================================================================
NETGENPlugin_Hypothesis_2D::NETGENPlugin_Hypothesis_2D (int hypId, int studyId,
                                                        SMESH_Gen * gen)
  : NETGENPlugin_Hypothesis(hypId, studyId, gen)/*,
    _quadAllowed (GetDefaultQuadAllowed())*/
{
  _name = "NETGEN_Parameters_2D";
  _param_algo_dim = 2;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
NETGENPlugin_RemesherHypothesis_2D::
NETGENPlugin_RemesherHypothesis_2D (int hypId, int studyId, SMESH_Gen * gen)
  : NETGENPlugin_Hypothesis(hypId, studyId, gen)
{
  _name = "NETGEN_RemesherParameters_2D";
  _param_algo_dim = 2;

  _ridgeAngle = DefaultRidgeAngle();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void NETGENPlugin_RemesherHypothesis_2D::SetRidgeAngle( double angle )
{
  if ( _ridgeAngle != angle )
  {
    _ridgeAngle = angle;
    NotifySubMeshesHypothesisModification();
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

double NETGENPlugin_RemesherHypothesis_2D::GetRidgeAngle() const
{
  return _ridgeAngle;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

std::ostream & NETGENPlugin_RemesherHypothesis_2D::SaveTo(std::ostream & save)
{
  NETGENPlugin_Hypothesis::SaveTo( save );
  save << " " << _ridgeAngle;

  return save;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

std::istream & NETGENPlugin_RemesherHypothesis_2D::LoadFrom(std::istream & load)
{
  NETGENPlugin_Hypothesis::LoadFrom( load );
  if ( !load )
    load.clear(ios::badbit | load.rdstate());

  load >> _ridgeAngle;

  if ( !load )
    _ridgeAngle = DefaultRidgeAngle();

  return load;
}
