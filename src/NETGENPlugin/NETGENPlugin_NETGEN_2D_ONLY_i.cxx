// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : NETGENPlugin_NETGEN_2D_ONLY_i.cxx
//  Author : Edward AGAPOV (OCC)
//  Module : SMESH
//
#include "NETGENPlugin_NETGEN_2D_ONLY_i.hxx"
#include "SMESH_Gen.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  NETGENPlugin_NETGEN_2D_ONLY_i::NETGENPlugin_NETGEN_2D_ONLY_i
 *
 *  Constructor
 */
//=============================================================================

NETGENPlugin_NETGEN_2D_ONLY_i::NETGENPlugin_NETGEN_2D_ONLY_i( PortableServer::POA_ptr thePOA,
                                                              ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA ),
       SMESH_2D_Algo_i( thePOA )
{
  myBaseImpl = new ::NETGENPlugin_NETGEN_2D_ONLY( theGenImpl->GetANewId(),
                                                  theGenImpl );
}

//=============================================================================
/*!
 *  NETGENPlugin_NETGEN_2D_ONLY_i::~NETGENPlugin_NETGEN_2D_ONLY_i
 *
 *  Destructor
 */
//=============================================================================

NETGENPlugin_NETGEN_2D_ONLY_i::~NETGENPlugin_NETGEN_2D_ONLY_i()
{
}

//=============================================================================
/*!
 *  NETGENPlugin_NETGEN_2D_ONLY_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::NETGENPlugin_NETGEN_2D_ONLY* NETGENPlugin_NETGEN_2D_ONLY_i::GetImpl()
{
  return ( ::NETGENPlugin_NETGEN_2D_ONLY* )myBaseImpl;
}

