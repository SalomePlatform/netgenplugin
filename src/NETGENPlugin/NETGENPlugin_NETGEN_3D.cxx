// Copyright (C) 2007-2022  CEA/DEN, EDF R&D, OPEN CASCADE
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

//=============================================================================
// File      : NETGENPlugin_NETGEN_3D.cxx
//             Moved here from SMESH_NETGEN_3D.cxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
//=============================================================================
//
#include "NETGENPlugin_NETGEN_3D.hxx"

#include "NETGENPlugin_Hypothesis.hxx"

// TODO: remove use of netgen_param
#include "NETGENPlugin_DriverParam.hxx"

#include <SMDS_MeshElement.hxx>
#include <SMDS_MeshNode.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_ControlsDef.hxx>
#include <SMESH_Gen.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MeshEditor.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_subMesh.hxx>
#include <StdMeshers_MaxElementVolume.hxx>
#include <StdMeshers_QuadToTriaAdaptor.hxx>
#include <StdMeshers_ViscousLayers.hxx>
#include <SMESH_subMesh.hxx>


#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <utilities.h>

#include <list>
#include <vector>
#include <map>

#include <cstdlib>

/*
  Netgen include files
*/

#ifndef OCCGEOMETRY
#define OCCGEOMETRY
#endif
#include <occgeom.hpp>

#ifdef NETGEN_V5
#include <ngexception.hpp>
#endif
#ifdef NETGEN_V6
#include <core/exception.hpp>
#endif

namespace nglib {
#include <nglib.h>
}
namespace netgen {

  NETGENPLUGIN_DLL_HEADER
  extern MeshingParameters mparam;

  NETGENPLUGIN_DLL_HEADER
  extern volatile multithreadt multithread;
}
using namespace nglib;
using namespace std;

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::NETGENPlugin_NETGEN_3D(int hypId, SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, gen)
{
  _name = "NETGEN_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
  _compatibleHypothesis.push_back("MaxElementVolume");
  _compatibleHypothesis.push_back("NETGEN_Parameters");
  _compatibleHypothesis.push_back("ViscousLayers");

  _maxElementVolume = 0.;

  _hypMaxElementVolume = NULL;
  _hypParameters = NULL;
  _viscousLayersHyp = NULL;

  _requireShape = false; // can work without shape
}

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::~NETGENPlugin_NETGEN_3D()
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool NETGENPlugin_NETGEN_3D::CheckHypothesis (SMESH_Mesh&         aMesh,
                                              const TopoDS_Shape& aShape,
                                              Hypothesis_Status&  aStatus)
{
  _hypMaxElementVolume = NULL;
  _hypParameters = NULL;
  _viscousLayersHyp = NULL;
  _maxElementVolume = DBL_MAX;

  // for correct work of GetProgress():
  //netgen::multithread.percent = 0.;
  //netgen::multithread.task = "Volume meshing";
  _progressByTic = -1.;

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  //const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps =
    GetUsedHypothesis(aMesh, aShape, /*ignoreAuxiliary=*/false);
  list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    aStatus = SMESH_Hypothesis::HYP_OK;
    return true;  // can work with no hypothesis
  }

  aStatus = HYP_OK;
  for ( ; h != hyps.end(); ++h )
  {
    if ( !_hypMaxElementVolume )
      _hypMaxElementVolume = dynamic_cast< const StdMeshers_MaxElementVolume*> ( *h );
    if ( !_viscousLayersHyp ) // several _viscousLayersHyp's allowed
      _viscousLayersHyp = dynamic_cast< const StdMeshers_ViscousLayers*> ( *h );
    if ( ! _hypParameters )
      _hypParameters = dynamic_cast< const NETGENPlugin_Hypothesis*> ( *h );

    if ( *h != _hypMaxElementVolume &&
         *h != _viscousLayersHyp &&
         *h != _hypParameters &&
         !dynamic_cast< const StdMeshers_ViscousLayers*>(*h)) // several VL hyps allowed
      aStatus = HYP_INCOMPATIBLE;
  }
  if ( _hypMaxElementVolume && _hypParameters )
    aStatus = HYP_INCOMPATIBLE;
  else if ( aStatus == HYP_OK && _viscousLayersHyp )
    error( _viscousLayersHyp->CheckHypothesis( aMesh, aShape, aStatus ));

  if ( _hypMaxElementVolume )
    _maxElementVolume = _hypMaxElementVolume->GetMaxVolume();

  return aStatus == HYP_OK;
}



//=============================================================================
/*!
 *Here we are going to use the NETGEN mesher
 */
//=============================================================================

/**
 * @brief Get an iterator on the Surface element with their orientation
 *
 */

bool NETGENPlugin_NETGEN_3D::getSurfaceElements(
    SMESH_Mesh&         aMesh,
    const TopoDS_Shape& aShape,
    SMESH_ProxyMesh::Ptr proxyMesh,
    NETGENPlugin_Internals &internals,
    SMESH_MesherHelper &helper,
    netgen_params &aParams,
    std::map<const SMDS_MeshElement*, tuple<bool, bool>>& listElements
)
{
  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  TopAbs_ShapeEnum mainType = aMesh.GetShapeToMesh().ShapeType();
  bool checkReverse = ( mainType == TopAbs_COMPOUND || mainType == TopAbs_COMPSOLID );

  for ( TopExp_Explorer exFa( aShape, TopAbs_FACE ); exFa.More(); exFa.Next())
  {
    const TopoDS_Shape& aShapeFace = exFa.Current();
    int faceID = meshDS->ShapeToIndex( aShapeFace );
    bool isInternalFace = internals.isInternalShape( faceID );
    bool isRev = false;
    if ( checkReverse && !isInternalFace &&
          helper.NbAncestors(aShapeFace, aMesh, aShape.ShapeType()) > 1 )
      // IsReversedSubMesh() can work wrong on strongly curved faces,
      // so we use it as less as possible
      isRev = helper.IsReversedSubMesh( TopoDS::Face( aShapeFace ));

    const SMESHDS_SubMesh * aSubMeshDSFace = proxyMesh->GetSubMesh( aShapeFace );
    if ( !aSubMeshDSFace ) continue;

    SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
    if ( aParams._quadraticMesh &&
          dynamic_cast< const SMESH_ProxyMesh::SubMesh*>( aSubMeshDSFace ))
    {
      // add medium nodes of proxy triangles to helper (#16843)
      while ( iteratorElem->more() )
        helper.AddTLinks( static_cast< const SMDS_MeshFace* >( iteratorElem->next() ));

      iteratorElem = aSubMeshDSFace->GetElements();
    }
    while(iteratorElem->more()){
      const SMDS_MeshElement* elem = iteratorElem->next();
      // check mesh face
      if ( !elem ){
        aParams._error = COMPERR_BAD_INPUT_MESH;
        aParams._comment = "Null element encounters";
        return true;
      }
      if ( elem->NbCornerNodes() != 3 ){
        aParams._error = COMPERR_BAD_INPUT_MESH;
        aParams._comment = "Not triangle element encounters";
        return true;
      }
      listElements[elem] = tuple(isRev, isInternalFace);
    }
  }

  return false;
}


bool NETGENPlugin_NETGEN_3D::computeFillNgMesh(
  SMESH_Mesh&         aMesh,
  const TopoDS_Shape& aShape,
  vector< const SMDS_MeshNode* > &nodeVec,
  NETGENPlugin_NetgenLibWrapper &ngLib,
  SMESH_MesherHelper &helper,
  netgen_params &aParams,
  int &Netgen_NbOfNodes)
{
  netgen::multithread.terminate = 0;
  netgen::multithread.task = "Volume meshing";
  aParams._progressByTic = -1.;

  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();

  aParams._quadraticMesh = helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );

  Netgen_NbOfNodes = 0;
  double Netgen_point[3];
  int Netgen_triangle[3];

  Ng_Mesh * Netgen_mesh = (Ng_Mesh*)ngLib._ngMesh;

  {
    const int invalid_ID = -1;

    SMESH::Controls::Area areaControl;
    SMESH::Controls::TSequenceOfXYZ nodesCoords;

    // maps nodes to ng ID
    typedef map< const SMDS_MeshNode*, int, TIDCompare > TNodeToIDMap;
    typedef TNodeToIDMap::value_type                     TN2ID;
    TNodeToIDMap nodeToNetgenID;

    // find internal shapes
    NETGENPlugin_Internals internals( aMesh, aShape, /*is3D=*/true );

    // ---------------------------------
    // Feed the Netgen with surface mesh
    // ---------------------------------
    bool isRev=false;
    bool isInternalFace=false;

    SMESH_ProxyMesh::Ptr proxyMesh( new SMESH_ProxyMesh( aMesh ));
    if ( aParams._viscousLayersHyp )
    {
      netgen::multithread.percent = 3;
      proxyMesh = aParams._viscousLayersHyp->Compute( aMesh, aShape );
      if ( !proxyMesh )
        return false;
    }
    if ( aMesh.NbQuadrangles() > 0 )
    {
      netgen::multithread.percent = 6;
      StdMeshers_QuadToTriaAdaptor* Adaptor = new StdMeshers_QuadToTriaAdaptor;
      Adaptor->Compute(aMesh,aShape,proxyMesh.get());
      proxyMesh.reset( Adaptor );
    }

    std::map<const SMDS_MeshElement*, tuple<bool, bool>> listElements;
    bool ret = getSurfaceElements(aMesh, aShape, proxyMesh, internals, helper, aParams, listElements);
    if(ret)
      return ret;

    for ( auto const& [elem, info] : listElements ) // loop on elements on a geom face
    {
      isRev = get<0>(info);
      isInternalFace = get<1>(info);
      // Add nodes of triangles and triangles them-selves to netgen mesh

      // add three nodes of triangle
      bool hasDegen = false;
      for ( int iN = 0; iN < 3; ++iN )
      {
        const SMDS_MeshNode* node = elem->GetNode( iN );
        const int shapeID = node->getshapeId();
        if ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_EDGE &&
              helper.IsDegenShape( shapeID ))
        {
          // ignore all nodes on degeneraged edge and use node on its vertex instead
          TopoDS_Shape vertex = TopoDS_Iterator( meshDS->IndexToShape( shapeID )).Value();
          node = SMESH_Algo::VertexNode( TopoDS::Vertex( vertex ), meshDS );
          hasDegen = true;
        }
        int& ngID = nodeToNetgenID.insert(TN2ID( node, invalid_ID )).first->second;
        if ( ngID == invalid_ID )
        {
          ngID = ++Netgen_NbOfNodes;
          Netgen_point [ 0 ] = node->X();
          Netgen_point [ 1 ] = node->Y();
          Netgen_point [ 2 ] = node->Z();
          Ng_AddPoint(Netgen_mesh, Netgen_point);
        }
        Netgen_triangle[ isRev ? 2-iN : iN ] = ngID;
      }
      // add triangle
      if ( hasDegen && (Netgen_triangle[0] == Netgen_triangle[1] ||
                        Netgen_triangle[0] == Netgen_triangle[2] ||
                        Netgen_triangle[2] == Netgen_triangle[1] ))
        continue;

      Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);

      if ( isInternalFace && !proxyMesh->IsTemporary( elem ))
      {
        swap( Netgen_triangle[1], Netgen_triangle[2] );
        Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
      }
    } // loop on elements on a face

    // insert old nodes into nodeVec
    nodeVec.resize( nodeToNetgenID.size() + 1, 0 );
    TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
    for ( ; n_id != nodeToNetgenID.end(); ++n_id )
      nodeVec[ n_id->second ] = n_id->first;
    nodeToNetgenID.clear();

    if ( internals.hasInternalVertexInSolid() )
    {
      netgen::OCCGeometry occgeo;
      NETGENPlugin_Mesher::AddIntVerticesInSolids( occgeo,
                                                   (netgen::Mesh&) *Netgen_mesh,
                                                   nodeVec,
                                                   internals);
    }
  }
  Netgen_NbOfNodes = Ng_GetNP( Netgen_mesh );
  return false;
}

bool NETGENPlugin_NETGEN_3D::computePrepareParam(
  SMESH_Mesh&         aMesh,
  NETGENPlugin_NetgenLibWrapper &ngLib,
  netgen::OCCGeometry &occgeo,
  SMESH_MesherHelper &helper,
  netgen_params &aParams,
  int &endWith)

{
  netgen::multithread.terminate = 0;

  netgen::Mesh* ngMesh = ngLib._ngMesh;

  NETGENPlugin_Mesher aMesher( &aMesh, helper.GetSubShape(), /*isVolume=*/true );


  if ( aParams._hypParameters )
  {
    aMesher.SetParameters( aParams._hypParameters );

    if ( !aParams._hypParameters->GetLocalSizesAndEntries().empty() ||
         !aParams._hypParameters->GetMeshSizeFile().empty() )
    {
      if ( ! &ngMesh->LocalHFunction() )
      {
        netgen::Point3d pmin, pmax;
        ngMesh->GetBox( pmin, pmax, 0 );
        ngMesh->SetLocalH( pmin, pmax, aParams._hypParameters->GetGrowthRate() );
      }
      aMesher.SetLocalSize( occgeo, *ngMesh );

      try {
        ngMesh->LoadLocalMeshSize( netgen::mparam.meshsizefilename );
      } catch (netgen::NgException & ex) {
        aParams._error = COMPERR_BAD_PARMETERS;
        aParams._comment = ex.What();
        return false;
      }
    }
    if ( !aParams._hypParameters->GetOptimize() )
      endWith = netgen::MESHCONST_MESHVOLUME;
  }
  else if ( aParams._hypMaxElementVolume )
  {
    netgen::mparam.maxh = pow( 72, 1/6. ) * pow( aParams.maxElementVolume, 1/3. );
    // limitVolumeSize( ngMesh, mparam.maxh ); // result is unpredictable
  }
  else if ( aMesh.HasShapeToMesh() )
  {
    aMesher.PrepareOCCgeometry( occgeo, helper.GetSubShape(), aMesh );
    netgen::mparam.maxh = occgeo.GetBoundingBox().Diam()/2;
  }
  else
  {
    netgen::Point3d pmin, pmax;
    ngMesh->GetBox (pmin, pmax);
    netgen::mparam.maxh = Dist(pmin, pmax)/2;
  }

  if ( !aParams._hypParameters && aMesh.HasShapeToMesh() )
  {
    netgen::mparam.minh = aMesher.GetDefaultMinSize( helper.GetSubShape(), netgen::mparam.maxh );
  }
  return false;
}

bool NETGENPlugin_NETGEN_3D::computeRunMesher(
  netgen::OCCGeometry &occgeo,
  vector< const SMDS_MeshNode* > &nodeVec,
  netgen::Mesh* ngMesh,
  NETGENPlugin_NetgenLibWrapper &ngLib,
  netgen_params &aParams,
  int &startWith, int &endWith)
{
  int err = 1;

  try
  {
    OCC_CATCH_SIGNALS;

    ngLib.CalcLocalH(ngMesh);
    err = ngLib.GenerateMesh(occgeo, startWith, endWith);

    if(netgen::multithread.terminate)
      return false;
    if ( err ){
      aParams._comment = SMESH_Comment("Error in netgen::OCCGenerateMesh() at ") << netgen::multithread.task;
      return true;
    }
  }
  catch (Standard_Failure& ex)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    str << " at " << netgen::multithread.task
        << ": " << ex.DynamicType()->Name();
    if ( ex.GetMessageString() && strlen( ex.GetMessageString() ))
      str << ": " << ex.GetMessageString();
    aParams._comment = str;
    return true;
  }
  catch (netgen::NgException& exc)
  {
    SMESH_Comment str("NgException");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    str << ": " << exc.What();
    aParams._comment = str;
    return true;
  }
  catch (...)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    aParams._comment = str;
    return true;
  }

  if ( err )
  {
    SMESH_ComputeErrorPtr ce = NETGENPlugin_Mesher::ReadErrors(nodeVec);
    if ( ce && ce->HasBadElems() ){
      aParams._error = ce->myName;
      aParams._comment = ce->myComment;
      return true;
    }
  }

  return false;
}

bool NETGENPlugin_NETGEN_3D::computeFillMesh(
  vector< const SMDS_MeshNode* > &nodeVec,
  NETGENPlugin_NetgenLibWrapper &ngLib,
  SMESH_MesherHelper &helper,
  int &Netgen_NbOfNodes
  )
{
  Ng_Mesh* Netgen_mesh = ngLib.ngMesh();

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);
  int Netgen_NbOfTetra    = Ng_GetNE(Netgen_mesh);

  bool isOK = ( /*status == NG_OK &&*/ Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK )
  {
    double Netgen_point[3];
    int    Netgen_tetrahedron[4];

    // create and insert new nodes into nodeVec
    nodeVec.resize( Netgen_NbOfNodesNew + 1, 0 );
    int nodeIndex = Netgen_NbOfNodes + 1;
    for ( ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint( Netgen_mesh, nodeIndex, Netgen_point );
      nodeVec.at(nodeIndex) = helper.AddNode(Netgen_point[0], Netgen_point[1], Netgen_point[2]);
    }

    // create tetrahedrons
    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      Ng_GetVolumeElement(Netgen_mesh, elemIndex, Netgen_tetrahedron);
      try
      {
        helper.AddVolume (nodeVec.at( Netgen_tetrahedron[0] ),
                          nodeVec.at( Netgen_tetrahedron[1] ),
                          nodeVec.at( Netgen_tetrahedron[2] ),
                          nodeVec.at( Netgen_tetrahedron[3] ));
      }
      catch (...)
      {
      }
    }
  }
  return false;
}

bool NETGENPlugin_NETGEN_3D::Compute(
  SMESH_Mesh&         aMesh,
  const TopoDS_Shape& aShape)
{
  // vector of nodes in which node index == netgen ID
  vector< const SMDS_MeshNode* > nodeVec;
  NETGENPlugin_NetgenLibWrapper ngLib;
  SMESH_MesherHelper helper(aMesh);
  int startWith = netgen::MESHCONST_MESHVOLUME;
  int endWith   = netgen::MESHCONST_OPTVOLUME;
  int Netgen_NbOfNodes;

  netgen_params aParams;

  aParams._hypParameters = const_cast<NETGENPlugin_Hypothesis*>(_hypParameters);
  aParams._hypMaxElementVolume = const_cast<StdMeshers_MaxElementVolume*>(_hypMaxElementVolume);
  aParams.maxElementVolume = _maxElementVolume;
  aParams._progressByTic = _progressByTic;
  aParams._quadraticMesh = _quadraticMesh;
  aParams._viscousLayersHyp = const_cast<StdMeshers_ViscousLayers*>(_viscousLayersHyp);

  bool ret;
  ret = computeFillNgMesh(aMesh, aShape, nodeVec, ngLib, helper, aParams, Netgen_NbOfNodes);
  if(ret)
    return error( aParams._error, aParams._comment);

  netgen::OCCGeometry occgeo;
  computePrepareParam(aMesh, ngLib, occgeo, helper, aParams, endWith);
  ret = computeRunMesher(occgeo, nodeVec, ngLib._ngMesh, ngLib, aParams, startWith, endWith);
  if(ret){
    if(aParams._error)
      return error(aParams._error, aParams._comment);

    error(aParams._comment);
    return true;
  }
  computeFillMesh(nodeVec, ngLib, helper, Netgen_NbOfNodes);

  return false;

}

//================================================================================
/*!
 * \brief set parameters and generate the volume mesh
 */
//================================================================================

bool NETGENPlugin_NETGEN_3D::compute(SMESH_Mesh&                     aMesh,
                                     SMESH_MesherHelper&             helper,
                                     vector< const SMDS_MeshNode* >& nodeVec,
                                     NETGENPlugin_NetgenLibWrapper&  ngLib)
{
  auto time0 = std::chrono::high_resolution_clock::now();

  netgen::multithread.terminate = 0;

  netgen::Mesh* ngMesh = ngLib._ngMesh;
  Ng_Mesh* Netgen_mesh = ngLib.ngMesh();
  int Netgen_NbOfNodes = Ng_GetNP( Netgen_mesh );

  int startWith = netgen::MESHCONST_MESHVOLUME;
  int endWith   = netgen::MESHCONST_OPTVOLUME;
  int err = 1;

  NETGENPlugin_Mesher aMesher( &aMesh, helper.GetSubShape(), /*isVolume=*/true );
  netgen::OCCGeometry occgeo;

  if ( _hypParameters )
  {
    aMesher.SetParameters( _hypParameters );

    if ( !_hypParameters->GetLocalSizesAndEntries().empty() ||
         !_hypParameters->GetMeshSizeFile().empty() )
    {
      if ( ! &ngMesh->LocalHFunction() )
      {
        netgen::Point3d pmin, pmax;
        ngMesh->GetBox( pmin, pmax, 0 );
        ngMesh->SetLocalH( pmin, pmax, _hypParameters->GetGrowthRate() );
      }
      aMesher.SetLocalSize( occgeo, *ngMesh );

      try {
        ngMesh->LoadLocalMeshSize( netgen::mparam.meshsizefilename );
      } catch (netgen::NgException & ex) {
        return error( COMPERR_BAD_PARMETERS, ex.What() );
      }
    }
    if ( !_hypParameters->GetOptimize() )
      endWith = netgen::MESHCONST_MESHVOLUME;
  }
  else if ( _hypMaxElementVolume )
  {
    netgen::mparam.maxh = pow( 72, 1/6. ) * pow( _maxElementVolume, 1/3. );
    // limitVolumeSize( ngMesh, mparam.maxh ); // result is unpredictable
  }
  else if ( aMesh.HasShapeToMesh() )
  {
    aMesher.PrepareOCCgeometry( occgeo, helper.GetSubShape(), aMesh );
    netgen::mparam.maxh = occgeo.GetBoundingBox().Diam()/2;
  }
  else
  {
    netgen::Point3d pmin, pmax;
    ngMesh->GetBox (pmin, pmax);
    netgen::mparam.maxh = Dist(pmin, pmax)/2;
  }

  if ( !_hypParameters && aMesh.HasShapeToMesh() )
  {
    netgen::mparam.minh = aMesher.GetDefaultMinSize( helper.GetSubShape(), netgen::mparam.maxh );
  }

  try
  {
    OCC_CATCH_SIGNALS;
    auto time0 = std::chrono::high_resolution_clock::now();

    ngLib.CalcLocalH(ngMesh);
    err = ngLib.GenerateMesh(occgeo, startWith, endWith);

    if(netgen::multithread.terminate)
      return false;
    if ( err )
      error(SMESH_Comment("Error in netgen::OCCGenerateMesh() at ") << netgen::multithread.task);
  }
  catch (Standard_Failure& ex)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    str << " at " << netgen::multithread.task
        << ": " << ex.DynamicType()->Name();
    if ( ex.GetMessageString() && strlen( ex.GetMessageString() ))
      str << ": " << ex.GetMessageString();
    error(str);
  }
  catch (netgen::NgException& exc)
  {
    SMESH_Comment str("NgException");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    str << ": " << exc.What();
    error(str);
  }
  catch (...)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    error(str);
  }
  auto time1 = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time1-time0);
  std::cout << "Time for seq:compute: " << elapsed.count() * 1e-9 << std::endl;

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);
  int Netgen_NbOfTetra    = Ng_GetNE(Netgen_mesh);

  // -------------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Volume Elements
  // -------------------------------------------------------------------

  if ( err )
  {
    SMESH_ComputeErrorPtr ce = NETGENPlugin_Mesher::ReadErrors(nodeVec);
    if ( ce && ce->HasBadElems() )
      error( ce );
  }

  bool isOK = ( /*status == NG_OK &&*/ Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK )
  {
    double Netgen_point[3];
    int    Netgen_tetrahedron[4];

    // create and insert new nodes into nodeVec
    nodeVec.resize( Netgen_NbOfNodesNew + 1, 0 );
    int nodeIndex = Netgen_NbOfNodes + 1;
    for ( ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint( Netgen_mesh, nodeIndex, Netgen_point );
      nodeVec.at(nodeIndex) = helper.AddNode(Netgen_point[0], Netgen_point[1], Netgen_point[2]);
    }

    // create tetrahedrons
    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      Ng_GetVolumeElement(Netgen_mesh, elemIndex, Netgen_tetrahedron);
      try
      {
        helper.AddVolume (nodeVec.at( Netgen_tetrahedron[0] ),
                          nodeVec.at( Netgen_tetrahedron[1] ),
                          nodeVec.at( Netgen_tetrahedron[2] ),
                          nodeVec.at( Netgen_tetrahedron[3] ));
      }
      catch (...)
      {
      }
    }
  }
  auto time2 = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1);
  std::cout << "Time for seq:compute: " << elapsed.count() * 1e-9 << std::endl;


  return !err;
}

//================================================================================
/*!
 * \brief Compute tetrahedral mesh from 2D mesh without geometry
 */
//================================================================================

bool NETGENPlugin_NETGEN_3D::Compute(SMESH_Mesh&         aMesh,
                                     SMESH_MesherHelper* aHelper)
{
  const int invalid_ID = -1;

  netgen::multithread.terminate = 0;
  _progressByTic = -1.;

  SMESH_MesherHelper::MType MeshType = aHelper->IsQuadraticMesh();
  if ( MeshType == SMESH_MesherHelper::COMP )
    return error( COMPERR_BAD_INPUT_MESH,
                  SMESH_Comment("Mesh with linear and quadratic elements given"));

  aHelper->SetIsQuadratic( MeshType == SMESH_MesherHelper::QUADRATIC );

  // ---------------------------------
  // Feed the Netgen with surface mesh
  // ---------------------------------

  int Netgen_NbOfNodes = 0;
  double Netgen_point[3];
  int Netgen_triangle[3];

  NETGENPlugin_NetgenLibWrapper ngLib;
  Ng_Mesh * Netgen_mesh = ngLib.ngMesh();

  SMESH_ProxyMesh::Ptr proxyMesh( new SMESH_ProxyMesh( aMesh ));
  if ( aMesh.NbQuadrangles() > 0 )
  {
    StdMeshers_QuadToTriaAdaptor* Adaptor = new StdMeshers_QuadToTriaAdaptor;
    Adaptor->Compute(aMesh);
    proxyMesh.reset( Adaptor );

    if ( aHelper->IsQuadraticMesh() )
    {
      SMDS_ElemIteratorPtr fIt = proxyMesh->GetFaces();
      while( fIt->more())
        aHelper->AddTLinks( static_cast< const SMDS_MeshFace* >( fIt->next() ));
    }
  }

  // maps nodes to ng ID
  typedef map< const SMDS_MeshNode*, int, TIDCompare > TNodeToIDMap;
  typedef TNodeToIDMap::value_type                     TN2ID;
  TNodeToIDMap nodeToNetgenID;

  SMDS_ElemIteratorPtr fIt = proxyMesh->GetFaces();
  while( fIt->more())
  {
    // check element
    const SMDS_MeshElement* elem = fIt->next();
    if ( !elem )
      return error( COMPERR_BAD_INPUT_MESH, "Null element encounters");
    if ( elem->NbCornerNodes() != 3 )
      return error( COMPERR_BAD_INPUT_MESH, "Not triangle element encounters");

    // add three nodes of triangle
    for ( int iN = 0; iN < 3; ++iN )
    {
      const SMDS_MeshNode* node = elem->GetNode( iN );
      int& ngID = nodeToNetgenID.insert(TN2ID( node, invalid_ID )).first->second;
      if ( ngID == invalid_ID )
      {
        ngID = ++Netgen_NbOfNodes;
        Netgen_point [ 0 ] = node->X();
        Netgen_point [ 1 ] = node->Y();
        Netgen_point [ 2 ] = node->Z();
        Ng_AddPoint(Netgen_mesh, Netgen_point);
      }
      Netgen_triangle[ iN ] = ngID;
    }
    Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
  }
  proxyMesh.reset(); // delete tmp faces

  // vector of nodes in which node index == netgen ID
  vector< const SMDS_MeshNode* > nodeVec ( nodeToNetgenID.size() + 1 );
  // insert old nodes into nodeVec
  TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
  for ( ; n_id != nodeToNetgenID.end(); ++n_id )
    nodeVec.at( n_id->second ) = n_id->first;
  nodeToNetgenID.clear();

  // -------------------------
  // Generate the volume mesh
  // -------------------------

  return ( ngLib._isComputeOk = compute( aMesh, *aHelper, nodeVec, ngLib ));
}

void NETGENPlugin_NETGEN_3D::CancelCompute()
{
  SMESH_Algo::CancelCompute();
  netgen::multithread.terminate = 1;
}

//================================================================================
/*!
 * \brief Return Compute progress
 */
//================================================================================

double NETGENPlugin_NETGEN_3D::GetProgress() const
{
  double res;
  const char* volMeshing = "Volume meshing";
  const char* dlnMeshing = "Delaunay meshing";
  const double meshingRatio = 0.15;
  const_cast<NETGENPlugin_NETGEN_3D*>( this )->_progressTic++;

  if ( _progressByTic < 0. &&
       ( strncmp( netgen::multithread.task, dlnMeshing, 3 ) == 0 ||
         strncmp( netgen::multithread.task, volMeshing, 3 ) == 0 ))
  {
    res = 0.001 + meshingRatio * netgen::multithread.percent / 100.;
    //cout << netgen::multithread.task << " " <<_progressTic << "-" << netgen::multithread.percent << endl;
  }
  else // different otimizations
  {
    if ( _progressByTic < 0. )
      ((NETGENPlugin_NETGEN_3D*)this)->_progressByTic = meshingRatio / _progressTic;
    res = _progressByTic * _progressTic;
    //cout << netgen::multithread.task << " " << _progressTic << " " << res << endl;
  }
  return Min ( res, 0.98 );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool NETGENPlugin_NETGEN_3D::Evaluate(SMESH_Mesh& aMesh,
                                      const TopoDS_Shape& aShape,
                                      MapShapeNbElems& aResMap)
{
  smIdType nbtri = 0, nbqua = 0;
  double fullArea = 0.0;
  for (TopExp_Explorer expF(aShape, TopAbs_FACE); expF.More(); expF.Next()) {
    TopoDS_Face F = TopoDS::Face( expF.Current() );
    SMESH_subMesh *sm = aMesh.GetSubMesh(F);
    MapShapeNbElemsItr anIt = aResMap.find(sm);
    if( anIt==aResMap.end() ) {
      SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
      smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,"Submesh can not be evaluated",this));
      return false;
    }
    std::vector<smIdType> aVec = (*anIt).second;
    nbtri += std::max(aVec[SMDSEntity_Triangle],aVec[SMDSEntity_Quad_Triangle]);
    nbqua += std::max(aVec[SMDSEntity_Quadrangle],aVec[SMDSEntity_Quad_Quadrangle]);
    GProp_GProps G;
    BRepGProp::SurfaceProperties(F,G);
    double anArea = G.Mass();
    fullArea += anArea;
  }

  // collect info from edges
  smIdType nb0d_e = 0, nb1d_e = 0;
  bool IsQuadratic = false;
  bool IsFirst = true;
  TopTools_MapOfShape tmpMap;
  for (TopExp_Explorer expF(aShape, TopAbs_EDGE); expF.More(); expF.Next()) {
    TopoDS_Edge E = TopoDS::Edge(expF.Current());
    if( tmpMap.Contains(E) )
      continue;
    tmpMap.Add(E);
    SMESH_subMesh *aSubMesh = aMesh.GetSubMesh(expF.Current());
    MapShapeNbElemsItr anIt = aResMap.find(aSubMesh);
    if( anIt==aResMap.end() ) {
      SMESH_ComputeErrorPtr& smError = aSubMesh->GetComputeError();
      smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,
                                            "Submesh can not be evaluated",this));
      return false;
    }
    std::vector<smIdType> aVec = (*anIt).second;
    nb0d_e += aVec[SMDSEntity_Node];
    nb1d_e += std::max(aVec[SMDSEntity_Edge],aVec[SMDSEntity_Quad_Edge]);
    if(IsFirst) {
      IsQuadratic = (aVec[SMDSEntity_Quad_Edge] > aVec[SMDSEntity_Edge]);
      IsFirst = false;
    }
  }
  tmpMap.Clear();

  double ELen_face = sqrt(2.* ( fullArea/double(nbtri+nbqua*2) ) / sqrt(3.0) );
  double ELen_vol = pow( 72, 1/6. ) * pow( _maxElementVolume, 1/3. );
  double ELen = Min(ELen_vol,ELen_face*2);

  GProp_GProps G;
  BRepGProp::VolumeProperties(aShape,G);
  double aVolume = G.Mass();
  double tetrVol = 0.1179*ELen*ELen*ELen;
  double CoeffQuality = 0.9;
  smIdType nbVols = (smIdType)( aVolume/tetrVol/CoeffQuality );
  smIdType nb1d_f = (nbtri*3 + nbqua*4 - nb1d_e) / 2;
  smIdType nb1d_in = (nbVols*6 - nb1d_e - nb1d_f ) / 5;
  std::vector<smIdType> aVec(SMDSEntity_Last);
  for(smIdType i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aVec[i]=0;
  if( IsQuadratic ) {
    aVec[SMDSEntity_Node] = nb1d_in/6 + 1 + nb1d_in;
    aVec[SMDSEntity_Quad_Tetra] = nbVols - nbqua*2;
    aVec[SMDSEntity_Quad_Pyramid] = nbqua;
  }
  else {
    aVec[SMDSEntity_Node] = nb1d_in/6 + 1;
    aVec[SMDSEntity_Tetra] = nbVols - nbqua*2;
    aVec[SMDSEntity_Pyramid] = nbqua;
  }
  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);
  aResMap.insert(std::make_pair(sm,aVec));

  return true;
}


