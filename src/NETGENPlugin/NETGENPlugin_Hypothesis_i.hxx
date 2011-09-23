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
// File      : NETGENPlugin_Hypothesis_i.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 03/04/2006
// Project   : SALOME
//=============================================================================
//
#ifndef _NETGENPlugin_Hypothesis_i_HXX_
#define _NETGENPlugin_Hypothesis_i_HXX_

#include "NETGENPlugin_Defs.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include "SMESH_Hypothesis_i.hxx"
#include "NETGENPlugin_Hypothesis.hxx"

class SMESH_Gen;
//class GEOM_Object;

// NETGENPlugin parameters hypothesis

class NETGENPLUGIN_EXPORT NETGENPlugin_Hypothesis_i:
  public virtual POA_NETGENPlugin::NETGENPlugin_Hypothesis,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  NETGENPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                             int                     theStudyId,
                             ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~NETGENPlugin_Hypothesis_i();

  void SetMaxSize(CORBA::Double theSize);
  CORBA::Double GetMaxSize();

  void SetMinSize(CORBA::Double theSize);
  CORBA::Double GetMinSize();

  void SetSecondOrder(CORBA::Boolean theVal);
  CORBA::Boolean GetSecondOrder();

  void SetOptimize(CORBA::Boolean theVal);
  CORBA::Boolean GetOptimize();

  void SetFineness(CORBA::Long theFineness);
  CORBA::Long GetFineness();

  void SetGrowthRate(CORBA::Double theRate);
  CORBA::Double GetGrowthRate();

  void SetNbSegPerEdge(CORBA::Double theVal);
  CORBA::Double GetNbSegPerEdge();

  void SetNbSegPerRadius(CORBA::Double theVal);
  CORBA::Double GetNbSegPerRadius();

  void SetLocalSizeOnShape(GEOM::GEOM_Object_ptr GeomObj, CORBA::Double localSize);
  void SetLocalSizeOnEntry(const char* entry, CORBA::Double localSize);
  CORBA::Double GetLocalSizeOnEntry(const char* entry);
  NETGENPlugin::string_array* GetLocalSizeEntries();
  void UnsetLocalSizeOnEntry(const char* entry);

  // Get implementation
  ::NETGENPlugin_Hypothesis* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 protected:

  // to remember whether a parameter is already set (issue 0021364)
  enum SettingMethod
  {
    METH_SetMaxSize          = 1,
    METH_SetMinSize          = 2,
    METH_SetSecondOrder      = 4,
    METH_SetOptimize         = 8,
    METH_SetFineness         = 16,
    METH_SetGrowthRate       = 32,
    METH_SetNbSegPerEdge     = 64,
    METH_SetNbSegPerRadius   = 128,
    METH_SetLocalSizeOnEntry = 256,
    METH_LAST                = METH_SetLocalSizeOnEntry
  };
  int mySetMethodFlags;

  // Return true if a parameter is not yet set, else return true if a parameter changes.
  // PythonDumping depends on the result of this function.
  // Checking only change of a parameter is not enough because then the default values are
  // not dumped and if the defaults will change then the behaviour of scripts
  // created without dump of the default parameters will also change what is not good.
  template<typename T>
    bool isToSetParameter(T curValue, T newValue, /*SettingMethod*/int meth)
  {
    if ( mySetMethodFlags & meth ) // already set, check if a value is changing
      return ( curValue != newValue );
    else
      return ( mySetMethodFlags |= meth ); // == return true
  }
};

#endif
