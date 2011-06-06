// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
// File      : NETGENPlugin_Hypothesis_2D_i.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 03/04/2006
// Project   : SALOME
// $Header$
//=============================================================================
//
#ifndef _NETGENPlugin_Hypothesis_2D_i_HXX_
#define _NETGENPlugin_Hypothesis_2D_i_HXX_

#include "NETGENPlugin_Defs.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include "NETGENPlugin_Hypothesis_i.hxx"
#include "NETGENPlugin_Hypothesis_2D.hxx"

class SMESH_Gen;

// NETGENPlugin parameters hypothesis (2D case)

class NETGENPLUGIN_EXPORT  NETGENPlugin_Hypothesis_2D_i:
  public virtual POA_NETGENPlugin::NETGENPlugin_Hypothesis_2D,
  public NETGENPlugin_Hypothesis_i
{
 public:
  // Constructor
  NETGENPlugin_Hypothesis_2D_i (PortableServer::POA_ptr thePOA,
                                int                     theStudyId,
                                ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~NETGENPlugin_Hypothesis_2D_i();

  void SetQuadAllowed(CORBA::Boolean theVal);
  CORBA::Boolean GetQuadAllowed();

  // Get implementation
  ::NETGENPlugin_Hypothesis_2D* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
