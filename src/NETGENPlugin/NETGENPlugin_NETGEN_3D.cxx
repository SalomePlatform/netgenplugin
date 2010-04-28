//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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

#include "NETGENPlugin_Mesher.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_MeshEditor.hxx"
#include "StdMeshers_QuadToTriaAdaptor.hxx"

#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include "utilities.h"

#include <list>
#include <vector>
#include <map>

/*
  Netgen include files
*/

#define OCCGEOMETRY
#include <occgeom.hpp>
namespace nglib {
#include <nglib.h>
}
using namespace nglib;
using namespace std;

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::NETGENPlugin_NETGEN_3D(int hypId, int studyId,
                             SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::NETGENPlugin_NETGEN_3D");
  _name = "NETGEN_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
  _compatibleHypothesis.push_back("MaxElementVolume");

  _maxElementVolume = 0.;

  _hypMaxElementVolume = NULL;

  _requireShape = false; // can work without shape
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::~NETGENPlugin_NETGEN_3D()
{
  MESSAGE("NETGENPlugin_NETGEN_3D::~NETGENPlugin_NETGEN_3D");
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
  MESSAGE("NETGENPlugin_NETGEN_3D::CheckHypothesis");

  _hypMaxElementVolume = NULL;
  _maxElementVolume = DBL_MAX;

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (!nbHyp)
  {
    aStatus = SMESH_Hypothesis::HYP_OK;
    //aStatus = SMESH_Hypothesis::HYP_MISSING;
    return true;  // can work with no hypothesis
  }

  itl = hyps.begin();
  theHyp = (*itl); // use only the first hypothesis

  string hypName = theHyp->GetName();

  bool isOk = false;

  if (hypName == "MaxElementVolume")
  {
    _hypMaxElementVolume = static_cast<const StdMeshers_MaxElementVolume*> (theHyp);
    ASSERT(_hypMaxElementVolume);
    _maxElementVolume = _hypMaxElementVolume->GetMaxVolume();
    isOk =true;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return isOk;
}

//=============================================================================
/*!
 *Here we are going to use the NETGEN mesher
 */
//=============================================================================

bool NETGENPlugin_NETGEN_3D::Compute(SMESH_Mesh&         aMesh,
                                     const TopoDS_Shape& aShape)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::Compute with maxElmentsize = " << _maxElementVolume);

  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();

  SMESH_MesherHelper helper(aMesh);
  bool _quadraticMesh = helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );

  int Netgen_NbOfNodes     = 0;
  int Netgen_param2ndOrder = 0;
  double Netgen_paramFine  = 1.;
  double Netgen_paramSize  = pow( 72, 1/6. ) * pow( _maxElementVolume, 1/3. );

  double Netgen_point[3];
  int Netgen_triangle[3];
  int Netgen_tetrahedron[4];

  NETGENPlugin_NetgenLibWrapper ngLib;
  Ng_Mesh * Netgen_mesh = ngLib._ngMesh;

  // vector of nodes in which node index == netgen ID
  vector< const SMDS_MeshNode* > nodeVec;
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

    TopAbs_ShapeEnum mainType = aMesh.GetShapeToMesh().ShapeType();
    bool checkReverse = ( mainType == TopAbs_COMPOUND || mainType == TopAbs_COMPSOLID );

    StdMeshers_QuadToTriaAdaptor Adaptor;
    if ( aMesh.NbQuadrangles() > 0 )
      Adaptor.Compute(aMesh,aShape);

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
        isRev = SMESH_Algo::IsReversedSubMesh( TopoDS::Face(aShapeFace), meshDS );

      const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements( aShapeFace );
      if ( !aSubMeshDSFace ) continue;
      SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
      while ( iteratorElem->more() ) // loop on elements on a geom face
      {
        // check mesh face
        const SMDS_MeshElement* elem = iteratorElem->next();
        if ( !elem )
          return error( COMPERR_BAD_INPUT_MESH, "Null element encounters");
        vector< const SMDS_MeshElement* > trias;
        bool isTraingle = ( elem->NbNodes() == ( elem->IsQuadratic() ? 6 : 3 ));
        if ( !isTraingle )
        {
          // use adaptor to convert quadrangle face into triangles
          const list<const SMDS_FaceOfNodes*>* faces = Adaptor.GetTriangles(elem);
          if(faces==0)
            return error( COMPERR_BAD_INPUT_MESH,
                          SMESH_Comment("No triangles in adaptor for element ")<<elem->GetID());
          trias.assign( faces->begin(), faces->end() );
        }
        else
        {
          trias.push_back( elem );
        }
        // Add nodes of triangles and triangles them-selves to netgen mesh

        for ( int i = 0; i < trias.size(); ++i )
        {
          // add three nodes of triangle
          bool hasDegen = false;
          for ( int iN = 0; iN < 3; ++iN )
          {
            const SMDS_MeshNode* node = trias[i]->GetNode( iN );
            int shapeID = node->GetPosition()->GetShapeId();
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

          if ( isInternalFace && isTraingle )
          {
            swap( Netgen_triangle[1], Netgen_triangle[2] );
            Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
          }
        }
#ifdef _DEBUG_
        // check if a trainge is degenerated
        areaControl.GetPoints( elem, nodesCoords );
        double area = areaControl.GetValue( nodesCoords );
        if ( area <= DBL_MIN ) {
          MESSAGE( "Warning: Degenerated " << elem );
        }
#endif
      } // loop on elements on a face
    } // loop on faces of a SOLID or SHELL

    // insert old nodes into nodeVec
    nodeVec.resize( nodeToNetgenID.size() + 1, 0 );
    TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
    for ( ; n_id != nodeToNetgenID.end(); ++n_id )
      nodeVec[ n_id->second ] = n_id->first;
    nodeToNetgenID.clear();

    if ( internals.hasInternalVertexInSolid() )
    {
      netgen::OCCGeometry occgeo;
      NETGENPlugin_Mesher::addIntVerticesInSolids( occgeo,
                                                   (netgen::Mesh&) *Netgen_mesh,
                                                   nodeVec,
                                                   internals);
      Netgen_NbOfNodes = Ng_GetNP(Netgen_mesh);
    }
  }

  // -------------------------
  // Generate the volume mesh
  // -------------------------

  Ng_Meshing_Parameters Netgen_param;

  Netgen_param.secondorder = Netgen_param2ndOrder;
  Netgen_param.fineness    = Netgen_paramFine;
  Netgen_param.maxh        = Netgen_paramSize;

  Ng_Result status;

  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    status = Ng_GenerateVolumeMesh(Netgen_mesh, &Netgen_param);
  }
  catch (Standard_Failure& exc) {
    error(COMPERR_OCC_EXCEPTION, exc.GetMessageString());
    status = NG_VOLUME_FAILURE;
  }
  catch (...) {
    error("Exception in Ng_GenerateVolumeMesh()");
    status = NG_VOLUME_FAILURE;
  }
  if ( GetComputeError()->IsOK() ) {
    switch ( status ) {
    case NG_SURFACE_INPUT_ERROR:error( status, "NG_SURFACE_INPUT_ERROR");
    case NG_VOLUME_FAILURE:     error( status, "NG_VOLUME_FAILURE");
    case NG_STL_INPUT_ERROR:    error( status, "NG_STL_INPUT_ERROR");
    case NG_SURFACE_FAILURE:    error( status, "NG_SURFACE_FAILURE");
    case NG_FILE_NOT_FOUND:     error( status, "NG_FILE_NOT_FOUND");
    };
  }

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);
  int Netgen_NbOfTetra    = Ng_GetNE(Netgen_mesh);

  MESSAGE("End of Volume Mesh Generation. status=" << status <<
          ", nb new nodes: " << Netgen_NbOfNodesNew - Netgen_NbOfNodes <<
          ", nb tetra: " << Netgen_NbOfTetra);

  // -------------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Volume Elements
  // -------------------------------------------------------------------

  if ( status == NG_VOLUME_FAILURE )
  {
    SMESH_ComputeErrorPtr err = NETGENPlugin_Mesher::readErrors(nodeVec);
    if ( err && !err->myBadElements.empty() )
      error( err );
  }

  bool isOK = ( /*status == NG_OK &&*/ Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK )
  {
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
      helper.AddVolume (nodeVec.at( Netgen_tetrahedron[0] ),
                        nodeVec.at( Netgen_tetrahedron[1] ),
                        nodeVec.at( Netgen_tetrahedron[2] ),
                        nodeVec.at( Netgen_tetrahedron[3] ));
    }
  }

  return (status == NG_OK);
}

//================================================================================
/*!
 * \brief Compute tetrahedral mesh from 2D mesh without geometry
 */
//================================================================================

bool NETGENPlugin_NETGEN_3D::Compute(SMESH_Mesh&         aMesh,
                                     SMESH_MesherHelper* aHelper)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::Compute with maxElmentsize = " << _maxElementVolume);  
  const int invalid_ID = -1;
  bool _quadraticMesh = false;
  typedef map< const SMDS_MeshNode*, int, TIDCompare > TNodeToIDMap;
  TNodeToIDMap nodeToNetgenID;
  list< const SMDS_MeshElement* > triangles;
  SMESHDS_Mesh* MeshDS = aHelper->GetMeshDS();

  SMESH_MesherHelper::MType MeshType = aHelper->IsQuadraticMesh();
  
  if(MeshType == SMESH_MesherHelper::COMP)
    return error( COMPERR_BAD_INPUT_MESH,
                  SMESH_Comment("Mesh with linear and quadratic elements given."));
  else if (MeshType == SMESH_MesherHelper::QUADRATIC)
    _quadraticMesh = true;
    
  StdMeshers_QuadToTriaAdaptor Adaptor;
  Adaptor.Compute(aMesh);

  SMDS_FaceIteratorPtr fIt = MeshDS->facesIterator();
  TIDSortedElemSet sortedFaces; //  0020279: control the "random" use when using mesh algorithms
  while( fIt->more()) sortedFaces.insert( fIt->next() );

  TIDSortedElemSet::iterator itFace = sortedFaces.begin(), fEnd = sortedFaces.end();
  for ( ; itFace != fEnd; ++itFace )
  {
    // check element
    const SMDS_MeshElement* elem = *itFace;
    if ( !elem )
      return error( COMPERR_BAD_INPUT_MESH, "Null element encounters");

    vector< const SMDS_MeshElement* > trias;
    bool isTraingle = ( elem->NbNodes() == ( elem->IsQuadratic() ? 6 : 3 ));
    if ( !isTraingle ) {
      // using adaptor
      const list<const SMDS_FaceOfNodes*>* faces = Adaptor.GetTriangles(elem);
      if(faces==0)
        continue; // Issue 0020682. There already can be 3d mesh
      trias.assign( faces->begin(), faces->end() );
    }
    else {
      trias.push_back( elem );
    }
    for ( int i = 0; i < trias.size(); ++i )
    {
      triangles.push_back( trias[i] );
      for ( int iN = 0; iN < 3; ++iN )
      {
        const SMDS_MeshNode* node = trias[i]->GetNode( iN );
        // put elem nodes to nodeToNetgenID map
        nodeToNetgenID.insert( make_pair( node, invalid_ID ));
      }
    }
  }

  // ---------------------------------
  // Feed the Netgen with surface mesh
  // ---------------------------------

  int Netgen_NbOfNodes = 0;
  int Netgen_param2ndOrder = 0;
  double Netgen_paramFine = 1.;
  double Netgen_paramSize = pow( 72, 1/6. ) * pow( _maxElementVolume, 1/3. );
  
  double Netgen_point[3];
  int Netgen_triangle[3];
  int Netgen_tetrahedron[4];

  NETGENPlugin_NetgenLibWrapper ngLib;
  Ng_Mesh * Netgen_mesh = ngLib._ngMesh;

  // set nodes and remember thier netgen IDs
  
  TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
  for ( ; n_id != nodeToNetgenID.end(); ++n_id )
  {
    const SMDS_MeshNode* node = n_id->first;

    Netgen_point [ 0 ] = node->X();
    Netgen_point [ 1 ] = node->Y();
    Netgen_point [ 2 ] = node->Z();
    Ng_AddPoint(Netgen_mesh, Netgen_point);
    n_id->second = ++Netgen_NbOfNodes; // set netgen ID
  }

  // set triangles
  list< const SMDS_MeshElement* >::iterator tria = triangles.begin();
  for ( ; tria != triangles.end(); ++tria)
  {
    int i = 0;
    SMDS_ElemIteratorPtr triangleNodesIt = (*tria)->nodesIterator();
    while ( triangleNodesIt->more() ) {
      const SMDS_MeshNode * node =
        static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
      if(aHelper->IsMedium(node))
        continue;
      Netgen_triangle[ i ] = nodeToNetgenID[ node ];
      ++i;
    }
    Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
  }

  // -------------------------
  // Generate the volume mesh
  // -------------------------

  Ng_Meshing_Parameters Netgen_param;

  Netgen_param.secondorder = Netgen_param2ndOrder;
  Netgen_param.fineness = Netgen_paramFine;
  Netgen_param.maxh = Netgen_paramSize;

  Ng_Result status;

  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    status = Ng_GenerateVolumeMesh(Netgen_mesh, &Netgen_param);
  }
  catch (Standard_Failure& exc) {
    error(COMPERR_OCC_EXCEPTION, exc.GetMessageString());
    status = NG_VOLUME_FAILURE;
  }
  catch (...) {
    error("Bad mesh input!!!");
    status = NG_VOLUME_FAILURE;
  }
  if ( GetComputeError()->IsOK() ) {
    error( status, "Bad mesh input!!!");
  }

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);

  int Netgen_NbOfTetra = Ng_GetNE(Netgen_mesh);

  MESSAGE("End of Volume Mesh Generation. status=" << status <<
          ", nb new nodes: " << Netgen_NbOfNodesNew - Netgen_NbOfNodes <<
          ", nb tetra: " << Netgen_NbOfTetra);

  // -------------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Volume Elements
  // -------------------------------------------------------------------

  bool isOK = ( Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK )
  {
    // vector of nodes in which node index == netgen ID
    vector< const SMDS_MeshNode* > nodeVec ( Netgen_NbOfNodesNew + 1 );
    // insert old nodes into nodeVec
    for ( n_id = nodeToNetgenID.begin(); n_id != nodeToNetgenID.end(); ++n_id ) {
      nodeVec.at( n_id->second ) = n_id->first;
    }
    // create and insert new nodes into nodeVec
    int nodeIndex = Netgen_NbOfNodes + 1;
    
    for ( ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint( Netgen_mesh, nodeIndex, Netgen_point );
      SMDS_MeshNode * node = aHelper->AddNode(Netgen_point[0],
                                              Netgen_point[1],
                                              Netgen_point[2]);
      nodeVec.at(nodeIndex) = node;
    }

    // create tetrahedrons
    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      Ng_GetVolumeElement(Netgen_mesh, elemIndex, Netgen_tetrahedron);
      aHelper->AddVolume (nodeVec.at( Netgen_tetrahedron[0] ),
                          nodeVec.at( Netgen_tetrahedron[1] ),
                          nodeVec.at( Netgen_tetrahedron[2] ),
                          nodeVec.at( Netgen_tetrahedron[3] ));
    }
  }

  return (status == NG_OK);
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
  int nbtri = 0, nbqua = 0;
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
    std::vector<int> aVec = (*anIt).second;
    nbtri += Max(aVec[SMDSEntity_Triangle],aVec[SMDSEntity_Quad_Triangle]);
    nbqua += Max(aVec[SMDSEntity_Quadrangle],aVec[SMDSEntity_Quad_Quadrangle]);
    GProp_GProps G;
    BRepGProp::SurfaceProperties(F,G);
    double anArea = G.Mass();
    fullArea += anArea;
  }

  // collect info from edges
  int nb0d_e = 0, nb1d_e = 0;
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
    std::vector<int> aVec = (*anIt).second;
    nb0d_e += aVec[SMDSEntity_Node];
    nb1d_e += Max(aVec[SMDSEntity_Edge],aVec[SMDSEntity_Quad_Edge]);
    if(IsFirst) {
      IsQuadratic = (aVec[SMDSEntity_Quad_Edge] > aVec[SMDSEntity_Edge]);
      IsFirst = false;
    }
  }
  tmpMap.Clear();

  double ELen_face = sqrt(2.* ( fullArea/(nbtri+nbqua*2) ) / sqrt(3.0) );
  double ELen_vol = pow( 72, 1/6. ) * pow( _maxElementVolume, 1/3. );
  double ELen = Min(ELen_vol,ELen_face*2);

  GProp_GProps G;
  BRepGProp::VolumeProperties(aShape,G);
  double aVolume = G.Mass();
  double tetrVol = 0.1179*ELen*ELen*ELen;
  double CoeffQuality = 0.9;
  int nbVols = int( aVolume/tetrVol/CoeffQuality );
  int nb1d_f = (nbtri*3 + nbqua*4 - nb1d_e) / 2;
  int nb1d_in = (nbVols*6 - nb1d_e - nb1d_f ) / 5;
  std::vector<int> aVec(SMDSEntity_Last);
  for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aVec[i]=0;
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
