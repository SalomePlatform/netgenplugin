// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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
// File      : NETGENPlugin_Hypothesis_i.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 03/04/2006
// Project   : SALOME
//=============================================================================
//
#include "NETGENPlugin_Hypothesis_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

using namespace std;

//=============================================================================
/*!
 *  Specialization of isToSetParameter<T> for double
 */
//=============================================================================

template<>
bool NETGENPlugin_Hypothesis_i::isToSetParameter<double>(double curValue,
                                                         double newValue,
                                                         /*SettingMethod*/int meth)
{
  return isToSetParameter(true, (fabs(curValue - newValue) < 1e-20), meth);
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::NETGENPlugin_Hypothesis_i
 *
 *  Constructor
 */
//=============================================================================
NETGENPlugin_Hypothesis_i::
NETGENPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                           ::SMESH_Gen*            theGenImpl)
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA ),
    mySetMethodFlags(0)
{
  myBaseImpl = new ::NETGENPlugin_Hypothesis (theGenImpl->GetANewId(),
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
  if ( isToSetParameter( GetMaxSize(), theValue, METH_SetMaxSize ))
  {
    this->GetImpl()->SetMaxSize(theValue);
    SMESH::TPythonDump() << _this() << ".SetMaxSize( " << SMESH::TVar(theValue) << " )";
  }
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
  return this->GetImpl()->GetMaxSize();
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::SetMinSize
 *
 *  Set MinSize
 */
//=============================================================================
void NETGENPlugin_Hypothesis_i::SetMinSize (CORBA::Double theValue)
{
  if ( isToSetParameter( GetMinSize(), theValue, METH_SetMinSize ))
  {
    this->GetImpl()->SetMinSize(theValue);
    SMESH::TPythonDump() << _this() << ".SetMinSize( " << SMESH::TVar(theValue) << " )";
  }
}

//=============================================================================
/*!
 *  NETGENPlugin_Hypothesis_i::GetMinSize
 *
 *  Get MinSize
 */
//=============================================================================
CORBA::Double NETGENPlugin_Hypothesis_i::GetMinSize()
{
  return this->GetImpl()->GetMinSize();
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
  if ( isToSetParameter( GetSecondOrder(), theValue, METH_SetSecondOrder ))
  {
    this->GetImpl()->SetSecondOrder(theValue);
    SMESH::TPythonDump() << _this() << ".SetSecondOrder( " << theValue << " )";
  }
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
  if ( isToSetParameter( GetOptimize(), theValue, METH_SetOptimize ))
  {
    this->GetImpl()->SetOptimize(theValue);
    SMESH::TPythonDump() << _this() << ".SetOptimize( " << theValue << " )";
  }
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
  if ( isToSetParameter( GetFineness(), theValue, METH_SetFineness ))
  {
    this->GetImpl()->SetFineness((::NETGENPlugin_Hypothesis::Fineness)theValue);
    SMESH::TPythonDump() << _this() << ".SetFineness( " << theValue << " )";
  }
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
  if ( isToSetParameter( GetGrowthRate(), theValue, METH_SetGrowthRate ))
  {
    this->GetImpl()->SetGrowthRate(theValue);
    SMESH::TPythonDump() << _this() << ".SetGrowthRate( " << SMESH::TVar(theValue) << " )";
  }
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
  if ( isToSetParameter( GetNbSegPerEdge(), theValue, METH_SetNbSegPerEdge ))
  {
    this->GetImpl()->SetNbSegPerEdge(theValue);
    SMESH::TPythonDump() << _this() << ".SetNbSegPerEdge( " << SMESH::TVar(theValue) << " )";
  }
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
  if ( isToSetParameter( GetNbSegPerRadius(), theValue, METH_SetNbSegPerRadius ))
  {
    this->GetImpl()->SetNbSegPerRadius(theValue);
    SMESH::TPythonDump() << _this() << ".SetNbSegPerRadius( " << SMESH::TVar(theValue) << " )";
  }
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
  return this->GetImpl()->GetNbSegPerRadius();
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetChordalErrorEnabled(CORBA::Boolean theValue)
{
  if ( isToSetParameter( GetChordalErrorEnabled(), theValue, METH_SetChordalErrorEnabled ))
  {
    this->GetImpl()->SetChordalErrorEnabled(theValue);
    SMESH::TPythonDump() << _this() << ".SetChordalErrorEnabled( " << theValue << " )";
  }
}

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetChordalErrorEnabled()
{
  return GetImpl()->GetChordalErrorEnabled();
}

void NETGENPlugin_Hypothesis_i::SetChordalError(CORBA::Double theValue)
{
  if ( isToSetParameter( GetChordalError(), theValue, METH_SetChordalError ))
  {
    this->GetImpl()->SetChordalError(theValue);
    SMESH::TPythonDump() << _this() << ".SetChordalError( " << SMESH::TVar(theValue) << " )";
  }
}

CORBA::Double NETGENPlugin_Hypothesis_i::GetChordalError()
{
  return GetImpl()->GetChordalError();
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetLocalSizeOnShape(GEOM::GEOM_Object_ptr GeomObj,
                                                    CORBA::Double         localSize)
{
  string entry;
  entry = GeomObj->GetStudyEntry();
  if ( entry.empty() )
    THROW_SALOME_CORBA_EXCEPTION( "SetLocalSizeOnShape(), shape is not published in study!",
                                  SALOME::BAD_PARAM );
  SetLocalSizeOnEntry(entry.c_str(), localSize);
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetLocalSizeOnEntry(const char*   entry,
                                                    CORBA::Double localSize)
{
  if ( isToSetParameter( GetLocalSizeOnEntry(entry), localSize, METH_SetLocalSizeOnEntry ))
  {
    this->GetImpl()->SetLocalSizeOnEntry(entry, localSize);
    SMESH::TPythonDump()
      << _this() << ".SetLocalSizeOnShape(" << entry << ", " << localSize << ")";
  }
}

//=============================================================================

CORBA::Double NETGENPlugin_Hypothesis_i::GetLocalSizeOnEntry(const char* entry)
{
  return this->GetImpl()->GetLocalSizeOnEntry(entry);
}

//=============================================================================

NETGENPlugin::string_array* NETGENPlugin_Hypothesis_i::GetLocalSizeEntries()
{
  NETGENPlugin::string_array_var result = new NETGENPlugin::string_array();
  const ::NETGENPlugin_Hypothesis::TLocalSize localSizes =
    this->GetImpl()->GetLocalSizesAndEntries();
  result->length((CORBA::ULong) localSizes.size());
  ::NETGENPlugin_Hypothesis::TLocalSize::const_iterator it = localSizes.begin();
  for (int i=0 ; it != localSizes.end() ; i++, it++)
    {
      string entry = (*it).first;
      result[i] = CORBA::string_dup(entry.c_str());
    }
  return result._retn();
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::UnsetLocalSizeOnEntry(const char* entry)
{
  this->GetImpl()->UnsetLocalSizeOnEntry(entry);
  SMESH::TPythonDump() << _this() << ".UnsetLocalSizeOnEntry(\"" << entry << "\")";
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetMeshSizeFile(const char* fileName)
{
  if ( GetImpl()->GetMeshSizeFile() != fileName )
  {
    GetImpl()->SetMeshSizeFile( fileName );
    SMESH::TPythonDump() << _this() << ".SetMeshSizeFile( '" << fileName << "' )";
  }
}

//=============================================================================

char* NETGENPlugin_Hypothesis_i::GetMeshSizeFile()
{
  return CORBA::string_dup( GetImpl()->GetMeshSizeFile().c_str() );
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetQuadAllowed (CORBA::Boolean theValue)
{
  if ( NETGENPlugin_Hypothesis_i::isToSetParameter( GetQuadAllowed(),
                                                    theValue,
                                                    METH_SetQuadAllowed ))
  {
    this->GetImpl()->SetQuadAllowed(theValue);
    SMESH::TPythonDump() << _this() << ".SetQuadAllowed( " << theValue << " )";
  }
}

//=============================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetQuadAllowed()
{
  return this->GetImpl()->GetQuadAllowed();
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetUseSurfaceCurvature (CORBA::Boolean theValue)
{
  if ( NETGENPlugin_Hypothesis_i::isToSetParameter( GetUseSurfaceCurvature(),
                                                    theValue,
                                                    METH_SetSurfaceCurvature ))
  {
    this->GetImpl()->SetSurfaceCurvature(theValue);
    SMESH::TPythonDump() << _this() << ".SetUseSurfaceCurvature( " << theValue << " )";
  }
}

//=============================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetUseSurfaceCurvature()
{
  return this->GetImpl()->GetSurfaceCurvature();
}

//=============================================================================

void NETGENPlugin_Hypothesis_i::SetFuseEdges (CORBA::Boolean theValue)
{
  if ( NETGENPlugin_Hypothesis_i::isToSetParameter( GetFuseEdges(),
                                                    theValue,
                                                    METH_SetFuseEdges ))
  {
    this->GetImpl()->SetFuseEdges(theValue);
    SMESH::TPythonDump() << _this() << ".SetFuseEdges( " << theValue << " )";
  }
}

//=============================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetFuseEdges()
{
  return this->GetImpl()->GetFuseEdges();
}

//=======================================================================
//function : SetNbSurfOptSteps
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetNbSurfOptSteps(CORBA::Short nb )
{
  if ( GetNbSurfOptSteps() != nb )
  {
    this->GetImpl()->SetNbSurfOptSteps( nb );
    SMESH::TPythonDump() << _this() << ".SetNbSurfOptSteps( " << SMESH::TVar(nb) << " )";
  }
}

//=======================================================================
//function : GetNbSurfOptSteps
//purpose  :
//=======================================================================

CORBA::Short NETGENPlugin_Hypothesis_i::GetNbSurfOptSteps()
{
  return (CORBA::Short) GetImpl()->GetNbSurfOptSteps();
}

//=======================================================================
//function : SetNbVolOptSteps
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetNbVolOptSteps(CORBA::Short nb )
{
  if ( GetNbVolOptSteps() != nb )
  {
    this->GetImpl()->SetNbVolOptSteps( nb );
    SMESH::TPythonDump() << _this() << ".SetNbVolOptSteps( " << SMESH::TVar(nb) << " )";
  }

}

//=======================================================================
//function : GetNbVolOptSteps
//purpose  :
//=======================================================================

CORBA::Short NETGENPlugin_Hypothesis_i::GetNbVolOptSteps()
{
  return (CORBA::Short) GetImpl()->GetNbVolOptSteps();
}

//=======================================================================
//function : SetElemSizeWeight
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetElemSizeWeight(CORBA::Double size )
{
  if ( GetElemSizeWeight() != size )
  {
    this->GetImpl()->SetElemSizeWeight( size );
    SMESH::TPythonDump() << _this() << ".SetElemSizeWeight( " << SMESH::TVar(size) << " )";
  }
}

//=======================================================================
//function : GetElemSizeWeight
//purpose  :
//=======================================================================

CORBA::Double NETGENPlugin_Hypothesis_i::GetElemSizeWeight()
{
  return GetImpl()->GetElemSizeWeight();
}

//=======================================================================
//function : SetWorstElemMeasure
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetWorstElemMeasure(CORBA::Short val )
{
  if ( GetWorstElemMeasure() != val )
  {
    this->GetImpl()->SetWorstElemMeasure( val );
    SMESH::TPythonDump() << _this() << ".SetWorstElemMeasure( " << SMESH::TVar(val) << " )";
  }
}

//=======================================================================
//function : GetWorstElemMeasure
//purpose  :
//=======================================================================

CORBA::Short NETGENPlugin_Hypothesis_i::GetWorstElemMeasure()
{
  return (CORBA::Short) GetImpl()->GetWorstElemMeasure();
}

//=======================================================================
//function : SetNbThreads
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetNbThreads(CORBA::Short val )
{
  if ( GetNbThreads() != val )
  {
    this->GetImpl()->SetNbThreads( val );
    SMESH::TPythonDump() << _this() << ".SetNbThreads( " << SMESH::TVar(val) << " )";
  }
}

//=======================================================================
//function : GetNbThreads
//purpose  :
//=======================================================================

CORBA::Short NETGENPlugin_Hypothesis_i::GetNbThreads()
{
  return (CORBA::Short) GetImpl()->GetNbThreads();
}

//=======================================================================
//function : SetUseDelauney
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetUseDelauney(CORBA::Boolean toUse)
{
  if ( GetUseDelauney() != toUse )
  {
    this->GetImpl()->SetUseDelauney( toUse );
    SMESH::TPythonDump() << _this() << ".SetUseDelauney( " << toUse << " )";
  }
}

//=======================================================================
//function : GetUseDelauney
//purpose  :
//=======================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetUseDelauney()
{
  return GetImpl()->GetUseDelauney();
}

//=======================================================================
//function : SetCheckOverlapping
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetCheckOverlapping(CORBA::Boolean toCheck )
{
  if ( GetCheckOverlapping() != toCheck )
  {
    this->GetImpl()->SetCheckOverlapping( toCheck );
    SMESH::TPythonDump() << _this() << ".SetCheckOverlapping( " << toCheck << " )";
  }
}

//=======================================================================
//function : GetCheckOverlapping
//purpose  :
//=======================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetCheckOverlapping()
{
  return GetImpl()->GetCheckOverlapping();
}

//=======================================================================
//function : SetCheckChartBoundary
//purpose  :
//=======================================================================

void NETGENPlugin_Hypothesis_i::SetCheckChartBoundary(CORBA::Boolean toCheck )
{
  if ( GetCheckChartBoundary() != toCheck )
  {
    this->GetImpl()->SetCheckChartBoundary( toCheck );
    SMESH::TPythonDump() << _this() << ".SetCheckChartBoundary( " << toCheck << " )";
  }
}

//=======================================================================
//function : GetCheckChartBoundary
//purpose  : Get implementation
//=======================================================================

CORBA::Boolean NETGENPlugin_Hypothesis_i::GetCheckChartBoundary()
{
  return GetImpl()->GetCheckChartBoundary();
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

//================================================================================
/*!
 * \brief method intended to remove explicit treatment of Netgen hypotheses from SMESH_NoteBook
 */
//================================================================================

int NETGENPlugin_Hypothesis_i::getParamIndex(const TCollection_AsciiString& method,
                                             int nbVars) const
{
  if ( method == "SetMaxSize"        ) return 0;
  if ( method == "SetGrowthRate"     ) return 1;
  if ( method == "SetNbSegPerEdge"   ) return 2;
  if ( method == "SetNbSegPerRadius" ) return 3;
  if ( method == "SetMinSize" )        return nbVars-1;

  return SMESH_Hypothesis_i::getParamIndex( method, nbVars ); // return default value
}

//================================================================================
/*!
 * \brief Method used to convert variable parameters stored in an old study
 * into myMethod2VarParams. It should return a method name for an index of
 * variable parameters. Index is countered from zero
 */
//================================================================================

std::string NETGENPlugin_Hypothesis_i::getMethodOfParameter(const int paramIndex,
                                                            int nbVars) const
{
  switch ( paramIndex ) {
  case 0: return "SetMaxSize";
  case 1: return nbVars == 2 ? "SetMinSize" : "SetGrowthRate";
  case 2: return "SetNbSegPerEdge";
  case 3: return "SetNbSegPerRadius";
  case 4: return "SetMinSize";
  }
  return "";
}

//================================================================================
/*!
 * \brief Return geometry this hypothesis depends on. Return false if there is no geometry parameter
 */
//================================================================================

bool
NETGENPlugin_Hypothesis_i::getObjectsDependOn( std::vector< std::string > & entryArray,
                                               std::vector< int >         & /*subIDArray*/ ) const
{
  typedef ::NETGENPlugin_Hypothesis THyp;

  const THyp* h = static_cast< ::NETGENPlugin_Hypothesis* >( myBaseImpl );
  const THyp::TLocalSize& ls = h->GetLocalSizesAndEntries();

  THyp::TLocalSize::const_iterator entry2size = ls.cbegin();
  for ( ; entry2size != ls.cend(); ++entry2size )
    entryArray.push_back( entry2size->first );

  return true;
}

//================================================================================
/*!
 * \brief Set new geometry instead of that returned by getObjectsDependOn()
 */
//================================================================================

bool
NETGENPlugin_Hypothesis_i::setObjectsDependOn( std::vector< std::string > & entryArray,
                                               std::vector< int >         & /*subIDArray*/ )
{
  typedef ::NETGENPlugin_Hypothesis THyp;

  const THyp* h = static_cast< ::NETGENPlugin_Hypothesis* >( myBaseImpl );

  THyp::TLocalSize& lsNew = const_cast< THyp::TLocalSize& >( h->GetLocalSizesAndEntries() );
  THyp::TLocalSize ls;
  lsNew.swap( ls );

  THyp::TLocalSize::const_iterator entry2size = ls.cbegin();
  for ( int i = 0; entry2size != ls.cend(); ++entry2size, ++i )
    if ( !entryArray[ i ].empty() )
      lsNew[ entryArray[ i ]] = entry2size->second;

  return true;
}
