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
// File      : NETGENPlugin_Hypothesis_i.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 03/04/2006
// Project   : SALOME
// $Header$
//=============================================================================
using namespace std;

#include "NETGENPlugin_Hypothesis_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::NETGENPlugin_Hypothesis_i
 *
 *  Constructor
 */
//=============================================================================
NETGENPlugin_Hypothesis_i::
NETGENPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                           int                     theStudyId,
                           ::SMESH_Gen*            theGenImpl)
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "NETGENPlugin_Hypothesis_i::NETGENPlugin_Hypothesis_i" );
  myBaseImpl = new ::NETGENPlugin_Hypothesis (theGenImpl->GetANewId(),
                                              theStudyId,
                                              theGenImpl);
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::~NETGENPlugin_Hypothesis_i
 *
 *  Destructor
 */
//=============================================================================
NETGENPlugin_Hypothesis_i::~NETGENPlugin_Hypothesis_i()
{
  MESSAGE( "NETGENPlugin_Hypothesis_i::~NETGENPlugin_Hypothesis_i" );
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetMaxSize
 *
 *  Set MaxSize
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetMaxSize (CORBA::Double theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetMaxSize");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetMaxSize(theValue);
  SMESH::TPythonDump() << _this() << ".SetMaxSize( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetMaxSize
 *
 *  Get MaxSize
 */
//=============================================================================
CORBA::Double NETGENPlugin_Hypothesis_i::GetMaxSize()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetMaxSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMaxSize();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetSecondOrder
 *
 *  Set SecondOrder flag
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetSecondOrder (CORBA::Boolean theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetSecondOrder");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetSecondOrder(theValue);
  SMESH::TPythonDump() << _this() << ".SetSecondOrder( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetSecondOrder
 *
 *  Get SecondOrder flag
 */
//=============================================================================
CORBA::Boolean NETGENPlugin_Hypothesis_i::GetSecondOrder()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetSecondOrder");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetSecondOrder();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetOptimize
 *
 *  Set Optimize flag
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetOptimize (CORBA::Boolean theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetOptimize");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetOptimize(theValue);
  SMESH::TPythonDump() << _this() << ".SetOptimize( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetOptimize
 *
 *  Get Optimize flag
 */
//=============================================================================
CORBA::Boolean NETGENPlugin_Hypothesis_i::GetOptimize()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetOptimize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetOptimize();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetFineness
 *
 *  Set Fineness
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetFineness (CORBA::Long theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetFineness");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetFineness((::NETGENPlugin_Hypothesis::Fineness)theValue);
  SMESH::TPythonDump() << _this() << ".SetFineness( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetFineness
 *
 *  Get Fineness
 */
//=============================================================================
CORBA::Long NETGENPlugin_Hypothesis_i::GetFineness()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetFineness");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetFineness();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetGrowthRate
 *
 *  Set GrowthRate
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetGrowthRate (CORBA::Double theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetGrowthRate");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetGrowthRate(theValue);
  SMESH::TPythonDump() << _this() << ".SetGrowthRate( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetGrowthRate
 *
 *  Get GrowthRate
 */
//=============================================================================
CORBA::Double NETGENPlugin_Hypothesis_i::GetGrowthRate()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetGrowthRate");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetGrowthRate();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetNbSegPerEdge
 *
 *  Set NbSegPerEdge
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetNbSegPerEdge (CORBA::Double theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetNbSegPerEdge");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetNbSegPerEdge(theValue);
  SMESH::TPythonDump() << _this() << ".SetNbSegPerEdge( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetNbSegPerEdge
 *
 *  Get NbSegPerEdge
 */
//=============================================================================
CORBA::Double NETGENPlugin_Hypothesis_i::GetNbSegPerEdge()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetNbSegPerEdge");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetNbSegPerEdge();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetNbSegPerRadius
 *
 *  Set NbSegPerRadius
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetNbSegPerRadius (CORBA::Double theValue)
{
  MESSAGE("NETGENPlugin_Hypothesis_i::SetNbSegPerRadius");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetNbSegPerRadius(theValue);
  SMESH::TPythonDump() << _this() << ".SetNbSegPerRadius( " << theValue << " )";
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetNbSegPerRadius
 *
 *  Get NbSegPerRadius
 */
//=============================================================================
CORBA::Double NETGENPlugin_Hypothesis_i::GetNbSegPerRadius()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetNbSegPerRadius");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetNbSegPerRadius();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================
::NETGENPlugin_Hypothesis* NETGENPlugin_Hypothesis_i::GetImpl()
{
  MESSAGE("NETGENPlugin_Hypothesis_i::GetImpl");
  return (::NETGENPlugin_Hypothesis*)myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  
CORBA::Boolean NETGENPlugin_Hypothesis_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}
