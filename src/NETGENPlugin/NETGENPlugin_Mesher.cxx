//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  NETGENPlugin : C++ implementation
// File      : NETGENPlugin_Mesher.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 31/03/2006
// Project   : SALOME
//=============================================================================
//
#include "NETGENPlugin_Mesher.hxx"
#include "NETGENPlugin_Hypothesis_2D.hxx"
#include "NETGENPlugin_SimpleHypothesis_3D.hxx"

#include <SMDS_MeshElement.hxx>
#include <SMDS_MeshNode.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_ComputeError.hxx>
#include <SMESH_File.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_subMesh.hxx>
#include <utilities.h>

#include <vector>
#include <limits>

#include <BRep_Tool.hxx>
#include <NCollection_Map.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_ProgramError.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeInteger.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>

// Netgen include files
#define OCCGEOMETRY
#include <occgeom.hpp>
#include <meshing.hpp>
//#include <ngexception.hpp>
namespace netgen {
  extern int OCCGenerateMesh (OCCGeometry&, Mesh*&, int, int, char*);
  extern MeshingParameters mparam;
}

using namespace nglib;
using namespace std;

#ifdef _DEBUG_
#define nodeVec_ACCESS(index) (SMDS_MeshNode*) nodeVec.at((index))
#else
#define nodeVec_ACCESS(index) (SMDS_MeshNode*) nodeVec[index]
#endif

// dump elements added to ng mesh
//#define DUMP_SEGMENTS
//#define DUMP_TRIANGLES

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_Mesher::NETGENPlugin_Mesher (SMESH_Mesh* mesh,
                                          const TopoDS_Shape& aShape,
                                          const bool isVolume)
  : _mesh    (mesh),
    _shape   (aShape),
    _isVolume(isVolume),
    _optimize(true),
    _simpleHyp(NULL)
{
  defaultParameters();
}

//================================================================================
/*!
 * \brief Initialize global NETGEN parameters with default values
 */
//================================================================================

void NETGENPlugin_Mesher::defaultParameters()
{
  netgen::MeshingParameters& mparams = netgen::mparam;
  // maximal mesh edge size
  mparams.maxh = NETGENPlugin_Hypothesis::GetDefaultMaxSize();
  // minimal number of segments per edge
  mparams.segmentsperedge = NETGENPlugin_Hypothesis::GetDefaultNbSegPerEdge();
  // rate of growth of size between elements
  mparams.grading = NETGENPlugin_Hypothesis::GetDefaultGrowthRate();
  // safety factor for curvatures (elements per radius)
  mparams.curvaturesafety = NETGENPlugin_Hypothesis::GetDefaultNbSegPerRadius();
  // create elements of second order
  mparams.secondorder = NETGENPlugin_Hypothesis::GetDefaultSecondOrder() ? 1 : 0;
  // quad-dominated surface meshing
  if (_isVolume)
    mparams.quad = 0;
  else
    mparams.quad = NETGENPlugin_Hypothesis_2D::GetDefaultQuadAllowed() ? 1 : 0;
}

//=============================================================================
/*!
 * Pass parameters to NETGEN
 */
//=============================================================================
void NETGENPlugin_Mesher::SetParameters(const NETGENPlugin_Hypothesis* hyp)
{
  if (hyp)
  {
    netgen::MeshingParameters& mparams = netgen::mparam;
    // Initialize global NETGEN parameters:
    // maximal mesh segment size
    mparams.maxh = hyp->GetMaxSize();
    // minimal number of segments per edge
    mparams.segmentsperedge = hyp->GetNbSegPerEdge();
    // rate of growth of size between elements
    mparams.grading = hyp->GetGrowthRate();
    // safety factor for curvatures (elements per radius)
    mparams.curvaturesafety = hyp->GetNbSegPerRadius();
    // create elements of second order
    mparams.secondorder = hyp->GetSecondOrder() ? 1 : 0;
    // quad-dominated surface meshing
    // only triangles are allowed for volumic mesh
    if (!_isVolume)
      mparams.quad = static_cast<const NETGENPlugin_Hypothesis_2D*>
        (hyp)->GetQuadAllowed() ? 1 : 0;
    _optimize = hyp->GetOptimize();
    _simpleHyp = NULL;
  }
}

//=============================================================================
/*!
 * Pass simple parameters to NETGEN
 */
//=============================================================================

void NETGENPlugin_Mesher::SetParameters(const NETGENPlugin_SimpleHypothesis_2D* hyp)
{
  _simpleHyp = hyp;
  if ( _simpleHyp )
    defaultParameters();
}

//=============================================================================
/*!
 *  Link - a pair of integer numbers
 */
//=============================================================================
struct Link
{
  int n1, n2;
  Link(int _n1, int _n2) : n1(_n1), n2(_n2) {}
  Link() : n1(0), n2(0) {}
};

int HashCode(const Link& aLink, int aLimit)
{
  return HashCode(aLink.n1 + aLink.n2, aLimit);
}

Standard_Boolean IsEqual(const Link& aLink1, const Link& aLink2)
{
  return (aLink1.n1 == aLink2.n1 && aLink1.n2 == aLink2.n2 ||
          aLink1.n1 == aLink2.n2 && aLink1.n2 == aLink2.n1);
}

//================================================================================
/*!
 * \brief Initialize netgen::OCCGeometry with OCCT shape
 */
//================================================================================

void NETGENPlugin_Mesher::PrepareOCCgeometry(netgen::OCCGeometry&     occgeo,
                                             const TopoDS_Shape&      shape,
                                             SMESH_Mesh&              mesh,
                                             list< SMESH_subMesh* > * meshedSM,
                                             NETGENPlugin_Internals*  intern)
{
  BRepTools::Clean (shape);
  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh (shape, 0.01, true);
  } catch (Standard_Failure) {
  }
  Bnd_Box bb;
  BRepBndLib::Add (shape, bb);
  double x1,y1,z1,x2,y2,z2;
  bb.Get (x1,y1,z1,x2,y2,z2);
  MESSAGE("shape bounding box:\n" <<
          "(" << x1 << " " << y1 << " " << z1 << ") " <<
          "(" << x2 << " " << y2 << " " << z2 << ")");
  netgen::Point<3> p1 = netgen::Point<3> (x1,y1,z1);
  netgen::Point<3> p2 = netgen::Point<3> (x2,y2,z2);
  occgeo.boundingbox = netgen::Box<3> (p1,p2);

  occgeo.shape = shape;
  occgeo.changed = 1;

  // fill maps of shapes of occgeo with not yet meshed subshapes

  // get root submeshes
  list< SMESH_subMesh* > rootSM;
  if ( SMESH_subMesh* sm = mesh.GetSubMeshContaining( shape )) {
    rootSM.push_back( sm );
  }
  else {
    for ( TopoDS_Iterator it( shape ); it.More(); it.Next() )
      rootSM.push_back( mesh.GetSubMesh( it.Value() ));
  }

  // add subshapes of empty submeshes
  list< SMESH_subMesh* >::iterator rootIt = rootSM.begin(), rootEnd = rootSM.end();
  for ( ; rootIt != rootEnd; ++rootIt ) {
    SMESH_subMesh * root = *rootIt;
    SMESH_subMeshIteratorPtr smIt = root->getDependsOnIterator(/*includeSelf=*/true,
                                                               /*complexShapeFirst=*/true);
    // to find a right orientation of subshapes (PAL20462)
    TopTools_IndexedMapOfShape subShapes;
    TopExp::MapShapes(root->GetSubShape(), subShapes);
    while ( smIt->more() )
    {
      SMESH_subMesh* sm = smIt->next();
      TopoDS_Shape shape = sm->GetSubShape();
      if ( intern && intern->isShapeToPrecompute( shape ))
        continue;
      if ( !meshedSM || sm->IsEmpty() )
      {
        if ( shape.ShapeType() != TopAbs_VERTEX )
          shape = subShapes( subShapes.FindIndex( shape ));// shape -> index -> oriented shape
        if ( shape.Orientation() >= TopAbs_INTERNAL )
          shape.Orientation( TopAbs_FORWARD ); // isuue 0020676
        switch ( shape.ShapeType() ) {
        case TopAbs_FACE  : occgeo.fmap.Add( shape ); break;
        case TopAbs_EDGE  : occgeo.emap.Add( shape ); break;
        case TopAbs_VERTEX: occgeo.vmap.Add( shape ); break;
        case TopAbs_SOLID :occgeo.somap.Add( shape ); break;
        default:;
        }
      }
      // collect submeshes of meshed shapes
      else if (meshedSM)
      {
        meshedSM->push_back( sm );
      }
    }
  }
  occgeo.facemeshstatus.SetSize (occgeo.fmap.Extent());
  occgeo.facemeshstatus = 0;
#ifdef NETGEN_NEW
  occgeo.face_maxh.SetSize(occgeo.fmap.Extent());
  occgeo.face_maxh = netgen::mparam.maxh;
#endif

}

//================================================================================
/*!
 * \brief return id of netgen point corresponding to SMDS node
 */
//================================================================================
typedef map< const SMDS_MeshNode*, int > TNode2IdMap;

static int ngNodeId( const SMDS_MeshNode* node,
                     netgen::Mesh&        ngMesh,
                     TNode2IdMap*         nodeNgIdMap,
                     int                  isDoubledNode=0)
{
  int newNgId = ngMesh.GetNP() + 1;

  TNode2IdMap::iterator node_id =
    nodeNgIdMap[isDoubledNode].insert( make_pair( node, newNgId )).first;

  if ( node_id->second == newNgId)
  {
#if defined(DUMP_SEGMENTS) || defined(DUMP_TRIANGLES)
    cout << "Ng " << newNgId << " - " << node;
#endif
    netgen::MeshPoint p( netgen::Point<3> (node->X(), node->Y(), node->Z()) );
    ngMesh.AddPoint( p );
  }
  return node_id->second;
}

//================================================================================
/*!
 * \brief fill ngMesh with nodes and elements of computed submeshes
 */
//================================================================================

bool NETGENPlugin_Mesher::fillNgMesh(netgen::OCCGeometry&           occgeom,
                                     netgen::Mesh&                  ngMesh,
                                     vector<const SMDS_MeshNode*>&  nodeVec,
                                     const list< SMESH_subMesh* > & meshedSM,
                                     NETGENPlugin_Internals*        internalShapes)
{
  TNode2IdMap nodeNgIdMap[2]; // the second map stores nodes doubled to make the crack
  if ( !nodeVec.empty() )
    for ( int i = 1; i < nodeVec.size(); ++i )
      nodeNgIdMap[0].insert( make_pair( nodeVec[i], i ));

  TIDSortedElemSet borderElems;
  if ( internalShapes )
    internalShapes->findBorderElements(borderElems);

  TopTools_MapOfShape visitedShapes;

  SMESH_MesherHelper helper (*_mesh);

  int faceID = occgeom.fmap.Extent();

  list< SMESH_subMesh* >::const_iterator smIt, smEnd = meshedSM.end();
  for ( smIt = meshedSM.begin(); smIt != smEnd; ++smIt )
  {
    SMESH_subMesh* sm = *smIt;
    if ( !visitedShapes.Add( sm->GetSubShape() ))
      continue;

    SMESHDS_SubMesh * smDS = sm->GetSubMeshDS();
    if ( !smDS ) continue;

    switch ( sm->GetSubShape().ShapeType() )
    {
    case TopAbs_EDGE: { // EDGE
      // ----------------------
      TopoDS_Edge geomEdge  = TopoDS::Edge( sm->GetSubShape() );
      if ( geomEdge.Orientation() >= TopAbs_INTERNAL )
        geomEdge.Orientation( TopAbs_FORWARD ); // isuue 0020676

      // Add ng segments for each not meshed face the edge bounds
      TopTools_MapOfShape visitedAncestors;
      PShapeIteratorPtr fIt = helper.GetAncestors( geomEdge, *sm->GetFather(), TopAbs_FACE );
      while ( const TopoDS_Shape * anc = fIt->next() )
      {
        if ( !visitedAncestors.Add( *anc )) continue;
        TopoDS_Face face = TopoDS::Face( *anc );
        if ( face.Orientation() >= TopAbs_INTERNAL )
          face.Orientation( TopAbs_FORWARD ); // isuue 0020676

        int faceID = occgeom.fmap.FindIndex( face );
        if ( faceID < 1 )
          continue; // meshed face

        // find out orientation of geomEdge within face
        TopAbs_Orientation fOri = helper.GetSubShapeOri( face, geomEdge );

        // get all nodes from geomEdge
        bool isForwad = ( fOri == geomEdge.Orientation() );
        bool isQuad   = smDS->NbElements() ? smDS->GetElements()->next()->IsQuadratic() : false;
        StdMeshers_FaceSide fSide( face, geomEdge, _mesh, isForwad, isQuad );
        const vector<UVPtStruct>& points = fSide.GetUVPtStruct();
        int i, nbSeg = fSide.NbSegments();

        double otherSeamParam = 0;
        helper.SetSubShape( face );
        bool isSeam = helper.IsRealSeam( geomEdge );
        if ( isSeam )
          otherSeamParam =
            helper.GetOtherParam( helper.GetPeriodicIndex() == 1 ? points[0].u : points[0].v );

        // add segments

        int prevNgId = ngNodeId( points[0].node, ngMesh, nodeNgIdMap );

        for ( i = 0; i < nbSeg; ++i )
        {
          const UVPtStruct& p1 = points[ i ];
          const UVPtStruct& p2 = points[ i+1 ];

          netgen::Segment seg;
          // ng node ids
#ifdef NETGEN_NEW
          seg.pnums[0] = prevNgId;
          seg.pnums[1] = prevNgId = ngNodeId( p2.node, ngMesh, nodeNgIdMap );
#else
          seg.p1 = prevNgId;
          seg.p2 = prevNgId = ngNodeId( p2.node, ngMesh, nodeNgIdMap );
#endif
          // node param on curve
          seg.epgeominfo[ 0 ].dist = p1.param;
          seg.epgeominfo[ 1 ].dist = p2.param;
          // uv on face
          seg.epgeominfo[ 0 ].u = p1.u;
          seg.epgeominfo[ 0 ].v = p1.v;
          seg.epgeominfo[ 1 ].u = p2.u;
          seg.epgeominfo[ 1 ].v = p2.v;

          //seg.epgeominfo[ iEnd ].edgenr = edgeID; //  = geom.emap.FindIndex(edge);
          seg.si = faceID;                   // = geom.fmap.FindIndex (face);
          seg.edgenr = ngMesh.GetNSeg() + 1; // segment id
          ngMesh.AddSegment (seg);
#ifdef DUMP_SEGMENTS
          cout << "Segment: " << seg.edgenr << " on SMESH face " << helper.GetMeshDS()->ShapeToIndex( face ) << endl
               << "\tface index: " << seg.si << endl
               << "\tp1: " << seg.p1 << endl
               << "\tp2: " << seg.p2 << endl
               << "\tp0 param: " << seg.epgeominfo[ 0 ].dist << endl
               << "\tp0 uv: " << seg.epgeominfo[ 0 ].u <<", "<< seg.epgeominfo[ 0 ].v << endl
               << "\tp0 edge: " << seg.epgeominfo[ 0 ].edgenr << endl
               << "\tp1 param: " << seg.epgeominfo[ 1 ].dist << endl
               << "\tp1 uv: " << seg.epgeominfo[ 1 ].u <<", "<< seg.epgeominfo[ 1 ].v << endl
               << "\tp1 edge: " << seg.epgeominfo[ 1 ].edgenr << endl;
#endif
          if ( isSeam )
          {
            if ( helper.GetPeriodicIndex() == 1 ) {
              seg.epgeominfo[ 0 ].u = otherSeamParam;
              seg.epgeominfo[ 1 ].u = otherSeamParam;
              swap (seg.epgeominfo[0].v, seg.epgeominfo[1].v);
            } else {
              seg.epgeominfo[ 0 ].v = otherSeamParam;
              seg.epgeominfo[ 1 ].v = otherSeamParam;
              swap (seg.epgeominfo[0].u, seg.epgeominfo[1].u);
            }
#ifdef NETGEN_NEW
            swap (seg.pnums[0], seg.pnums[1]);
#else
            swap (seg.p1, seg.p2);
#endif
            swap (seg.epgeominfo[0].dist, seg.epgeominfo[1].dist);
            seg.edgenr = ngMesh.GetNSeg() + 1; // segment id
            ngMesh.AddSegment (seg);
          }
          else if ( fOri == TopAbs_INTERNAL )
          {
#ifdef NETGEN_NEW
            swap (seg.pnums[0], seg.pnums[1]);
#else
            swap (seg.p1, seg.p2);
#endif
            swap( seg.epgeominfo[0], seg.epgeominfo[1] );
            seg.edgenr = ngMesh.GetNSeg() + 1; // segment id
            ngMesh.AddSegment (seg);
#ifdef DUMP_SEGMENTS
            cout << "Segment: " << seg.edgenr << endl << "\t is REVERSE of the previous" << endl;
#endif
          }
        }
      } // loop on geomEdge ancestors

      break;
    } // case TopAbs_EDGE

    case TopAbs_FACE: { // FACE
      // ----------------------
      const TopoDS_Face& geomFace  = TopoDS::Face( sm->GetSubShape() );
      helper.SetSubShape( geomFace );

      // Find solids the geomFace bounds
      int solidID1 = 0, solidID2 = 0;
      PShapeIteratorPtr solidIt = helper.GetAncestors( geomFace, *sm->GetFather(), TopAbs_SOLID);
      while ( const TopoDS_Shape * solid = solidIt->next() )
      {
        int id = occgeom.somap.FindIndex ( *solid );
        if ( solidID1 && id != solidID1 ) solidID2 = id;
        else                              solidID1 = id;
      }
      faceID++;
      _faceDescriptors[ faceID ].first  = solidID1;
      _faceDescriptors[ faceID ].second = solidID2;

      // Orient the face correctly in solidID1 (issue 0020206)
      bool reverse = false;
      if ( solidID1 ) {
        TopoDS_Shape solid = occgeom.somap( solidID1 );
        TopAbs_Orientation faceOriInSolid = helper.GetSubShapeOri( solid, geomFace );
        if ( faceOriInSolid >= 0 )
          reverse = SMESH_Algo::IsReversedSubMesh
            ( TopoDS::Face( geomFace.Oriented( faceOriInSolid )), helper.GetMeshDS() );
      }

      // Add surface elements

      netgen::Element2d tri(3);
      tri.SetIndex ( faceID );

      // triangle on internal or "border" face having doubled nodes
      netgen::Element2d triDbl(3);
      triDbl.SetIndex ( faceID );
      bool isInternalFace = ( internalShapes && geomFace.Orientation() == TopAbs_INTERNAL );
      bool isBorderFace   = ( internalShapes && internalShapes->isBorderFace( sm->GetId() ));

#ifdef DUMP_TRIANGLES
      cout << "SMESH face " << helper.GetMeshDS()->ShapeToIndex( geomFace )
           << " internal="<<isInternalFace<< " border="<<isBorderFace << endl;
#endif
      SMDS_ElemIteratorPtr faces = smDS->GetElements();
      while ( faces->more() )
      {
        const SMDS_MeshElement* f = faces->next();
        if ( f->NbNodes() % 3 != 0 ) // not triangle
        {
          PShapeIteratorPtr solidIt=helper.GetAncestors(geomFace,*sm->GetFather(),TopAbs_SOLID);
          if ( const TopoDS_Shape * solid = solidIt->next() )
            sm = _mesh->GetSubMesh( *solid );
          SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
          smError.reset( new SMESH_ComputeError(COMPERR_BAD_INPUT_MESH,"Not triangle submesh"));
          smError->myBadElements.push_back( f );
          return false;
        }
        bool makeDbl = ( isInternalFace || ( isBorderFace && borderElems.count( f )));

        for ( int i = 0; i < 3; ++i )
        {
          const SMDS_MeshNode* node = f->GetNode( i ), * inFaceNode=0;

          // get node UV on face
          int shapeID = node->GetPosition()->GetShapeId();
          if ( helper.IsSeamShape( shapeID ))
            if ( helper.IsSeamShape( f->GetNodeWrap( i+1 )->GetPosition()->GetShapeId() ))
              inFaceNode = f->GetNodeWrap( i-1 );
            else 
              inFaceNode = f->GetNodeWrap( i+1 );
          gp_XY uv = helper.GetNodeUV( geomFace, node, inFaceNode );

          int ind = reverse ? 3-i : i+1;
          tri.GeomInfoPi(ind).u = uv.X();
          tri.GeomInfoPi(ind).v = uv.Y();
          tri.PNum      (ind) = ngNodeId( node, ngMesh, nodeNgIdMap );

          if ( makeDbl )
          {
            int ngID = internalShapes->isInternalShape( shapeID ) ? ngNodeId( node, ngMesh, nodeNgIdMap, makeDbl ) : int ( tri.PNum( ind ));
            if ( isBorderFace )
            {
              tri.PNum( ind ) = ngID;
            }
            else
            {
              triDbl.GeomInfoPi(4-ind) = tri.GeomInfoPi(ind);
              triDbl.PNum      (4-ind) = ngID;
            }
          }
        }

        ngMesh.AddSurfaceElement (tri);

        if ( isInternalFace )
          ngMesh.AddSurfaceElement (triDbl);
#ifdef DUMP_TRIANGLES
        cout << tri << endl;
        if ( isInternalFace )
          cout << triDbl << endl;
#endif
      }
      break;
    } // case TopAbs_FACE

    case TopAbs_VERTEX: { // VERTEX
      // --------------------------
      SMDS_NodeIteratorPtr nodeIt = smDS->GetNodes();
      if ( nodeIt->more() )
        ngNodeId( nodeIt->next(), ngMesh, nodeNgIdMap );
      break;
    }
    default:;
    } // switch
  } // loop on submeshes

  // fill nodeVec
  nodeVec.resize( ngMesh.GetNP() + 1 );
  for ( int isDbl = 0; isDbl < 2; ++isDbl )
  {
    TNode2IdMap::iterator node_NgId, nodeNgIdEnd = nodeNgIdMap[isDbl].end();
    for ( node_NgId = nodeNgIdMap[isDbl].begin(); node_NgId != nodeNgIdEnd; ++node_NgId)
      nodeVec[ node_NgId->second ] = (SMDS_MeshNode*) node_NgId->first;
  }
  return true;
}

//================================================================================
/*!
 * \brief Fill SMESH mesh according to contents of netgen mesh
 *  \param occgeo - container of OCCT geometry to mesh
 *  \param ngMesh - netgen mesh
 *  \param initState - bn of entities in netgen mesh before computing
 *  \param sMesh - SMESH mesh to fill in
 *  \param nodeVec - vector of nodes in which node index == netgen ID
 *  \retval int - error
 */
//================================================================================

int NETGENPlugin_Mesher::FillSMesh(const netgen::OCCGeometry&          occgeo,
                                   const netgen::Mesh&                 ngMesh,
                                   const NETGENPlugin_ngMeshInfo&      initState,
                                   SMESH_Mesh&                         sMesh,
                                   std::vector<const SMDS_MeshNode*>&  nodeVec,
                                   SMESH_Comment&                      comment)
{
  int nbNod = ngMesh.GetNP();
  int nbSeg = ngMesh.GetNSeg();
  int nbFac = ngMesh.GetNSE();
  int nbVol = ngMesh.GetNE();

  SMESHDS_Mesh* meshDS = sMesh.GetMeshDS();

  // map of nodes assigned to submeshes
  NCollection_Map<int> pindMap;
  // create and insert nodes into nodeVec
  nodeVec.resize( nbNod + 1 );
  int i, nbInitNod = initState._nbNodes;
  for (i = nbInitNod+1; i <= nbNod; ++i )
  {
    const netgen::MeshPoint& ngPoint = ngMesh.Point(i);
    SMDS_MeshNode* node = NULL;
    TopoDS_Vertex aVert;
    // First, netgen creates nodes on vertices in occgeo.vmap,
    // so node index corresponds to vertex index
    // but (isuue 0020776) netgen does not create nodes with equal coordinates
    if ( i-nbInitNod <= occgeo.vmap.Extent() )
    {
#ifdef NETGEN_NEW
      gp_Pnt p (ngPoint(0), ngPoint(1), ngPoint(2));
#else
      gp_Pnt p (ngPoint.X(), ngPoint.Y(), ngPoint.Z());
#endif
      for (int iV = i-nbInitNod; aVert.IsNull() && iV <= occgeo.vmap.Extent(); ++iV)
      {
        aVert = TopoDS::Vertex( occgeo.vmap( iV ) );
        gp_Pnt pV = BRep_Tool::Pnt( aVert );
        if ( p.SquareDistance( pV ) > 1e-20 )
          aVert.Nullify();
        else
          node = const_cast<SMDS_MeshNode*>( SMESH_Algo::VertexNode( aVert, meshDS ));
      }
    }
    if (node) // node found on vertex
      pindMap.Add(i);
    else
    {
#ifdef NETGEN_NEW
      node = meshDS->AddNode(ngPoint(0), ngPoint(1), ngPoint(2));
#else
      node = meshDS->AddNode(ngPoint.X(), ngPoint.Y(), ngPoint.Z());
#endif
      if (!aVert.IsNull())
      {
        // point on vertex
        meshDS->SetNodeOnVertex(node, aVert);
        pindMap.Add(i);
      }
    }
    nodeVec[i] = node;
  }

  // create mesh segments along geometric edges
  NCollection_Map<Link> linkMap;
  int nbInitSeg = initState._nbSegments;
  for (i = nbInitSeg+1; i <= nbSeg; ++i )
  {
    const netgen::Segment& seg = ngMesh.LineSegment(i);
#ifdef NETGEN_NEW
    Link link(seg.pnums[0], seg.pnums[1]);
#else
    Link link(seg.p1, seg.p2);
#endif
    if (!linkMap.Add(link))
      continue;
    TopoDS_Edge aEdge;
#ifdef NETGEN_NEW
    int pinds[3] = { seg.pnums[0], seg.pnums[1], seg.pnums[2] };
#else
    int pinds[3] = { seg.p1, seg.p2, seg.pmid };
#endif
    int nbp = 0;
    double param2 = 0;
    for (int j=0; j < 3; ++j)
    {
      int pind = pinds[j];
      if (pind <= 0) continue;
      ++nbp;
      double param;
      if (j < 2)
      {
        if (aEdge.IsNull())
        {
          int aGeomEdgeInd = seg.epgeominfo[j].edgenr;
          if (aGeomEdgeInd > 0 && aGeomEdgeInd <= occgeo.emap.Extent())
            aEdge = TopoDS::Edge(occgeo.emap(aGeomEdgeInd));
        }
        param = seg.epgeominfo[j].dist;
        param2 += param;
      }
      else
        param = param2 * 0.5;
      if (pind <= nbInitNod || pindMap.Contains(pind))
        continue;
      if (!aEdge.IsNull())
      {
        meshDS->SetNodeOnEdge(nodeVec_ACCESS(pind), aEdge, param);
        pindMap.Add(pind);
      }
    }
    SMDS_MeshEdge* edge;
    if (nbp < 3) // second order ?
      edge = meshDS->AddEdge(nodeVec_ACCESS(pinds[0]), nodeVec_ACCESS(pinds[1]));
    else
      edge = meshDS->AddEdge(nodeVec_ACCESS(pinds[0]), nodeVec_ACCESS(pinds[1]),
                             nodeVec_ACCESS(pinds[2]));
    if (!edge)
    {
      if ( !comment.size() ) comment << "Cannot create a mesh edge";
      MESSAGE("Cannot create a mesh edge");
      nbSeg = nbFac = nbVol = 0;
      break;
    }
    if (!aEdge.IsNull())
      meshDS->SetMeshElementOnShape(edge, aEdge);
  }

  // create mesh faces along geometric faces
  int nbInitFac = initState._nbFaces;
  for (i = nbInitFac+1; i <= nbFac; ++i )
  {
    const netgen::Element2d& elem = ngMesh.SurfaceElement(i);
    int aGeomFaceInd = elem.GetIndex();
    TopoDS_Face aFace;
    if (aGeomFaceInd > 0 && aGeomFaceInd <= occgeo.fmap.Extent())
      aFace = TopoDS::Face(occgeo.fmap(aGeomFaceInd));
    vector<SMDS_MeshNode*> nodes;
    for (int j=1; j <= elem.GetNP(); ++j)
    {
      int pind = elem.PNum(j);
      SMDS_MeshNode* node = nodeVec_ACCESS(pind);
      nodes.push_back(node);
      if (pind <= nbInitNod || pindMap.Contains(pind))
        continue;
      if (!aFace.IsNull())
      {
        const netgen::PointGeomInfo& pgi = elem.GeomInfoPi(j);
        meshDS->SetNodeOnFace(node, aFace, pgi.u, pgi.v);
        pindMap.Add(pind);
      }
    }
    SMDS_MeshFace* face = NULL;
    switch (elem.GetType())
    {
    case netgen::TRIG:
      face = meshDS->AddFace(nodes[0],nodes[1],nodes[2]);
      break;
    case netgen::QUAD:
      face = meshDS->AddFace(nodes[0],nodes[1],nodes[2],nodes[3]);
      break;
    case netgen::TRIG6:
      face = meshDS->AddFace(nodes[0],nodes[1],nodes[2],nodes[5],nodes[3],nodes[4]);
      break;
    case netgen::QUAD8:
      face = meshDS->AddFace(nodes[0],nodes[1],nodes[2],nodes[3],
                             nodes[4],nodes[7],nodes[5],nodes[6]);
      break;
    default:
      MESSAGE("NETGEN created a face of unexpected type, ignoring");
      continue;
    }
    if (!face)
    {
      if ( !comment.size() ) comment << "Cannot create a mesh face";
      MESSAGE("Cannot create a mesh face");
      nbSeg = nbFac = nbVol = 0;
      break;
    }
    if (!aFace.IsNull())
      meshDS->SetMeshElementOnShape(face, aFace);
  }

  // create tetrahedra
  for (i = 1; i <= nbVol/* && isOK*/; ++i)
  {
    const netgen::Element& elem = ngMesh.VolumeElement(i);      
    int aSolidInd = elem.GetIndex();
    TopoDS_Solid aSolid;
    if (aSolidInd > 0 && aSolidInd <= occgeo.somap.Extent())
      aSolid = TopoDS::Solid(occgeo.somap(aSolidInd));
    vector<SMDS_MeshNode*> nodes;
    for (int j=1; j <= elem.GetNP(); ++j)
    {
      int pind = elem.PNum(j);
      SMDS_MeshNode* node = nodeVec_ACCESS(pind);
      nodes.push_back(node);
      if (pind <= nbInitNod || pindMap.Contains(pind))
        continue;
      if (!aSolid.IsNull())
      {
        // point in solid
        meshDS->SetNodeInVolume(node, aSolid);
        pindMap.Add(pind);
      }
    }
    SMDS_MeshVolume* vol = NULL;
    switch (elem.GetType())
    {
    case netgen::TET:
      vol = meshDS->AddVolume(nodes[0],nodes[1],nodes[2],nodes[3]);
      break;
    case netgen::TET10:
      vol = meshDS->AddVolume(nodes[0],nodes[1],nodes[2],nodes[3],
                              nodes[4],nodes[7],nodes[5],nodes[6],nodes[8],nodes[9]);
      break;
    default:
      MESSAGE("NETGEN created a volume of unexpected type, ignoring");
      continue;
    }
    if (!vol)
    {
      if ( !comment.size() ) comment << "Cannot create a mesh volume";
      MESSAGE("Cannot create a mesh volume");
      nbSeg = nbFac = nbVol = 0;
      break;
    }
    if (!aSolid.IsNull())
      meshDS->SetMeshElementOnShape(vol, aSolid);
  }
  return comment.empty() ? 0 : 1;
}

//=============================================================================
/*!
 * Here we are going to use the NETGEN mesher
 */
//=============================================================================

bool NETGENPlugin_Mesher::Compute()
{
  NETGENPlugin_NetgenLibWrapper ngLib;

  netgen::MeshingParameters& mparams = netgen::mparam;
  MESSAGE("Compute with:\n"
          " max size = " << mparams.maxh << "\n"
          " segments per edge = " << mparams.segmentsperedge);
  MESSAGE("\n"
          " growth rate = " << mparams.grading << "\n"
          " elements per radius = " << mparams.curvaturesafety << "\n"
          " second order = " << mparams.secondorder << "\n"
          " quad allowed = " << mparams.quad);

  SMESH_ComputeErrorPtr error = SMESH_ComputeError::New();
  

  // -------------------------
  // Prepare OCC geometry
  // -------------------------

  netgen::OCCGeometry occgeo;
  list< SMESH_subMesh* > meshedSM;
  NETGENPlugin_Internals internals( *_mesh, _shape, _isVolume );
  PrepareOCCgeometry( occgeo, _shape, *_mesh, &meshedSM, &internals );

  // -------------------------
  // Generate the mesh
  // -------------------------

  netgen::Mesh *ngMesh = NULL;
  NETGENPlugin_ngMeshInfo initState;

  SMESH_Comment comment;
  int err = 0;

  // vector of nodes in which node index == netgen ID
  vector< const SMDS_MeshNode* > nodeVec;
  try
  {
    // ----------------
    // compute 1D mesh
    // ----------------
    // Pass 1D simple parameters to NETGEN
    if ( _simpleHyp ) {
      if ( int nbSeg = _simpleHyp->GetNumberOfSegments() ) {
        // nb of segments
        mparams.segmentsperedge = nbSeg + 0.1;
        mparams.maxh = occgeo.boundingbox.Diam();
        mparams.grading = 0.01;
      }
      else {
        // segment length
        mparams.segmentsperedge = 1;
        mparams.maxh = _simpleHyp->GetLocalLength();
      }
    }
    // Let netgen create ngMesh and calculate element size on not meshed shapes
    char *optstr = 0;
    int startWith = netgen::MESHCONST_ANALYSE;
    int endWith   = netgen::MESHCONST_ANALYSE;
    err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
    if (err) comment << "Error in netgen::OCCGenerateMesh() at MESHCONST_ANALYSE step";
    ngLib.setMesh(( Ng_Mesh*) ngMesh );

    // Precompute internal edges (issue 0020676) in order to
    // add mesh on them correctly (twice) to netgen mesh
    if ( !err && internals.hasInternalEdges() )
    {
      // load internal shapes into OCCGeometry
      netgen::OCCGeometry intOccgeo;
      internals.getInternalEdges( intOccgeo.fmap, intOccgeo.emap, intOccgeo.vmap, meshedSM);
      intOccgeo.boundingbox = occgeo.boundingbox;
      intOccgeo.shape = occgeo.shape;

      // let netgen compute element size by the main geometry in temporary mesh
      netgen::Mesh *tmpNgMesh = NULL;
      netgen::OCCGenerateMesh(occgeo, tmpNgMesh, startWith, endWith, optstr);
      // compute mesh on internal edges
      endWith = netgen::MESHCONST_MESHEDGES;
      err = netgen::OCCGenerateMesh(intOccgeo, tmpNgMesh, startWith, endWith, optstr);
      if (err) comment << "Error in netgen::OCCGenerateMesh() at meshing internal edges";

      // fill SMESH by netgen mesh
      vector< const SMDS_MeshNode* > tmpNodeVec;
      FillSMesh( intOccgeo, *tmpNgMesh, initState, *_mesh, tmpNodeVec, comment );
      err = ( !comment.empty() );

      nglib::Ng_DeleteMesh((nglib::Ng_Mesh*)tmpNgMesh);
    }

    // Fill ngMesh with nodes and elements of computed submeshes
    if ( !err )
    {
      _faceDescriptors.clear();
      err = ! fillNgMesh(occgeo, *ngMesh, nodeVec, meshedSM);
    }
    initState = NETGENPlugin_ngMeshInfo(ngMesh);

    // Compute 1d mesh
    if (!err)
    {
      startWith = endWith = netgen::MESHCONST_MESHEDGES;
      err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
      if (err) comment << "Error in netgen::OCCGenerateMesh() at 1D mesh generation";
    }
    // ---------------------
    // compute surface mesh
    // ---------------------
    if (!err)
    {
      // Pass 2D simple parameters to NETGEN
      if ( _simpleHyp ) {
        if ( double area = _simpleHyp->GetMaxElementArea() ) {
          // face area
          mparams.maxh = sqrt(2. * area/sqrt(3.0));
          mparams.grading = 0.4; // moderate size growth
        }
        else {
          // length from edges
          if ( ngMesh->GetNSeg() ) {
            double edgeLength = 0;
            TopTools_MapOfShape visitedEdges;
            for ( TopExp_Explorer exp( _shape, TopAbs_EDGE ); exp.More(); exp.Next() )
              if( visitedEdges.Add(exp.Current()) )
                edgeLength += SMESH_Algo::EdgeLength( TopoDS::Edge( exp.Current() ));
            // we have to multiply length by 2 since for each TopoDS_Edge there
            // are double set of NETGEN edges or, in other words, we have to
            // divide ngMesh->GetNSeg() by 2.
            mparams.maxh = 2*edgeLength / ngMesh->GetNSeg();
          }
          else {
            mparams.maxh = 1000;
          }
          mparams.grading = 0.2; // slow size growth
        }
        mparams.maxh = min( mparams.maxh, occgeo.boundingbox.Diam()/2 );
        ngMesh->SetGlobalH (mparams.maxh);
        netgen::Box<3> bb = occgeo.GetBoundingBox();
        bb.Increase (bb.Diam()/20);
        ngMesh->SetLocalH (bb.PMin(), bb.PMax(), mparams.grading);
      }

      // Precompute internal faces (issue 0020676) in order to
      // add mesh on them correctly (twice to emulate the crack) to netgen mesh
      if ( internals.hasInternalFaces() )
      {
        // fill SMESH with generated segments
        FillSMesh( occgeo, *ngMesh, initState, *_mesh, nodeVec, comment );

        // load internal shapes into OCCGeometry
        netgen::OCCGeometry intOccgeo;
        list< SMESH_subMesh* > boundarySM;
        internals.getInternalFaces( intOccgeo.fmap, intOccgeo.emap, meshedSM, boundarySM);
        intOccgeo.boundingbox = occgeo.boundingbox;
        intOccgeo.shape = occgeo.shape;
        intOccgeo.facemeshstatus.SetSize (intOccgeo.fmap.Extent());
        intOccgeo.facemeshstatus = 0;

        // let netgen compute element size by the main geometry in temporary mesh
        int start = netgen::MESHCONST_ANALYSE, end = netgen::MESHCONST_ANALYSE;
        netgen::Mesh *tmpNgMesh = NULL;
        netgen::OCCGenerateMesh(occgeo, tmpNgMesh, start, end, optstr);
        // add already computed elements from submeshes of internal faces to tmpNgMesh
        vector< const SMDS_MeshNode* > tmpNodeVec;
        fillNgMesh(intOccgeo, *tmpNgMesh, tmpNodeVec, boundarySM);
        // compute mesh on internal faces
        NETGENPlugin_ngMeshInfo prevState(tmpNgMesh);
        start = netgen::MESHCONST_MESHEDGES;
        end = netgen::MESHCONST_MESHSURFACE;
        err = netgen::OCCGenerateMesh(intOccgeo, tmpNgMesh, start, end, optstr);
        if (err) comment << "Error in netgen::OCCGenerateMesh() at meshing internal faces";

        // fill SMESH with computed elements
        FillSMesh( intOccgeo, *tmpNgMesh, prevState, *_mesh, tmpNodeVec, comment );
        err = ( !comment.empty() );

        // add elements on internal faces to netgen mesh
//         occgeo.facemeshstatus.SetSize (occgeo.fmap.Extent() + intOccgeo.fmap.Extent());
//         occgeo.facemeshstatus = 0;
//         for ( int i = 1; i <= intOccgeo.fmap.Extent(); ++i )
//         {
//           occgeo.fmap.Add(intOccgeo.fmap(i));
//           occgeo.facemeshstatus[ occgeo.fmap.Extent()-1 ] = intOccgeo.facemeshstatus[i-1];
//         }
        err = ! fillNgMesh(occgeo, *ngMesh, nodeVec, meshedSM, &internals);
        initState = NETGENPlugin_ngMeshInfo(ngMesh);

        nglib::Ng_DeleteMesh((nglib::Ng_Mesh*)tmpNgMesh);
      }

      // Let netgen compute 2D mesh
      startWith = netgen::MESHCONST_MESHSURFACE;
      endWith = _optimize ? netgen::MESHCONST_OPTSURFACE : netgen::MESHCONST_MESHSURFACE;
      err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
      if (err) comment << "Error in netgen::OCCGenerateMesh() at surface mesh generation";
    }
    // ---------------------
    // generate volume mesh
    // ---------------------
    if (!err && _isVolume)
    {
      // Add ng face descriptors of meshed faces
      map< int, pair<int,int> >::iterator fId_soIds = _faceDescriptors.begin();
      for ( ; fId_soIds != _faceDescriptors.end(); ++fId_soIds ) {
        int faceID   = fId_soIds->first;
        int solidID1 = fId_soIds->second.first;
        int solidID2 = fId_soIds->second.second;
        ngMesh->AddFaceDescriptor (netgen::FaceDescriptor(faceID, solidID1, solidID2, 0));
      }
      // Pass 3D simple parameters to NETGEN
      const NETGENPlugin_SimpleHypothesis_3D* simple3d =
        dynamic_cast< const NETGENPlugin_SimpleHypothesis_3D* > ( _simpleHyp );
      if ( simple3d ) {
        if ( double vol = simple3d->GetMaxElementVolume() ) {
          // max volume
          mparams.maxh = pow( 72, 1/6. ) * pow( vol, 1/3. );
          mparams.maxh = min( mparams.maxh, occgeo.boundingbox.Diam()/2 );
        }
        else {
          // length from faces
          mparams.maxh = ngMesh->AverageH();
        }
//      netgen::ARRAY<double> maxhdom;
//      maxhdom.SetSize (occgeo.NrSolids());
//      maxhdom = mparams.maxh;
//      ngMesh->SetMaxHDomain (maxhdom);
        ngMesh->SetGlobalH (mparams.maxh);
        mparams.grading = 0.4;
        ngMesh->CalcLocalH();
      }
      // Let netgen compute 3D mesh
      startWith = netgen::MESHCONST_MESHVOLUME;
      endWith = _optimize ? netgen::MESHCONST_OPTVOLUME : netgen::MESHCONST_MESHVOLUME;
      err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
      if (err) comment << "Error in netgen::OCCGenerateMesh()";
    }
    if (!err && mparams.secondorder > 0)
    {
      netgen::OCCRefinementSurfaces ref (occgeo);
      ref.MakeSecondOrder (*ngMesh);
    }
  }
  catch (netgen::NgException exc)
  {
    error->myName = err = COMPERR_ALGO_FAILED;
    comment << exc.What();
  }

  int nbNod = ngMesh->GetNP();
  int nbSeg = ngMesh->GetNSeg();
  int nbFac = ngMesh->GetNSE();
  int nbVol = ngMesh->GetNE();
  bool isOK = ( !err && (_isVolume ? (nbVol > 0) : (nbFac > 0)) );

  MESSAGE((err ? "Mesh Generation failure" : "End of Mesh Generation") <<
          ", nb nodes: " << nbNod <<
          ", nb segments: " << nbSeg <<
          ", nb faces: " << nbFac <<
          ", nb volumes: " << nbVol);

  // ------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Elements
  // ------------------------------------------------------------

  if ( true /*isOK*/ ) // get whatever built
    FillSMesh( occgeo, *ngMesh, initState, *_mesh, nodeVec, comment );

  SMESH_ComputeErrorPtr readErr = readErrors(nodeVec);
  if ( readErr && !readErr->myBadElements.empty() )
    error = readErr;

  if ( error->IsOK() && ( !isOK || comment.size() > 0 ))
    error->myName = COMPERR_ALGO_FAILED;
  if ( !comment.empty() )
    error->myComment = comment;

  // set bad compute error to subshapes of all failed subshapes shapes
  if ( !error->IsOK() && err )
  {
    bool pb2D = false;
    for (int i = 1; i <= occgeo.fmap.Extent(); i++) {
      int status = occgeo.facemeshstatus[i-1];
      if (status == 1 ) continue;
      pb2D = true;
      if ( SMESH_subMesh* sm = _mesh->GetSubMeshContaining( occgeo.fmap( i ))) {
        SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
        if ( !smError || smError->IsOK() ) {
          if ( status == -1 )
            smError.reset( new SMESH_ComputeError( *error ));
          else
            smError.reset( new SMESH_ComputeError( COMPERR_ALGO_FAILED, "Ignored" ));
        }
      }
    }
    if ( !pb2D ) // all faces are OK
      for (int i = 1; i <= occgeo.somap.Extent(); i++)
        if ( SMESH_subMesh* sm = _mesh->GetSubMeshContaining( occgeo.somap( i )))
        {
          SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
          if ( sm->IsEmpty() && ( !smError || smError->IsOK() ))
            smError.reset( new SMESH_ComputeError( *error ));
        }
  }

  return error->IsOK();
}

//=============================================================================
/*!
 * Evaluate
 */
//=============================================================================
bool NETGENPlugin_Mesher::Evaluate(MapShapeNbElems& aResMap)
{
  netgen::MeshingParameters& mparams = netgen::mparam;


  // -------------------------
  // Prepare OCC geometry
  // -------------------------
  netgen::OCCGeometry occgeo;
  PrepareOCCgeometry( occgeo, _shape, *_mesh );

  bool tooManyElems = false;
  const int hugeNb = std::numeric_limits<int>::max() / 100;

  // ----------------
  // evaluate 1D 
  // ----------------
  // pass 1D simple parameters to NETGEN
  if ( _simpleHyp ) {
    if ( int nbSeg = _simpleHyp->GetNumberOfSegments() ) {
      // nb of segments
      mparams.segmentsperedge = nbSeg + 0.1;
      mparams.maxh = occgeo.boundingbox.Diam();
      mparams.grading = 0.01;
    }
    else {
      // segment length
      mparams.segmentsperedge = 1;
      mparams.maxh = _simpleHyp->GetLocalLength();
    }
  }
  // let netgen create ngMesh and calculate element size on not meshed shapes
  NETGENPlugin_NetgenLibWrapper ngLib;
  netgen::Mesh *ngMesh = NULL;
  char *optstr = 0;
  int startWith = netgen::MESHCONST_ANALYSE;
  int endWith   = netgen::MESHCONST_MESHEDGES;
  int err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
  ngLib.setMesh(( Ng_Mesh*) ngMesh );
  if (err) {
    if ( SMESH_subMesh* sm = _mesh->GetSubMeshContaining( _shape ))
      sm->GetComputeError().reset( new SMESH_ComputeError( COMPERR_ALGO_FAILED ));
    return false;
  }

  // calculate total nb of segments and length of edges
  double fullLen = 0.0;
  int fullNbSeg = 0;
  int entity = mparams.secondorder > 0 ? SMDSEntity_Quad_Edge : SMDSEntity_Edge;
  TopTools_DataMapOfShapeInteger Edge2NbSeg;
  for (TopExp_Explorer exp(_shape, TopAbs_EDGE); exp.More(); exp.Next())
  {
    TopoDS_Edge E = TopoDS::Edge( exp.Current() );
    if( !Edge2NbSeg.Bind(E,0) )
      continue;

    double aLen = SMESH_Algo::EdgeLength(E);
    fullLen += aLen;

    vector<int>& aVec = aResMap[_mesh->GetSubMesh(E)];
    if ( aVec.empty() )
      aVec.resize( SMDSEntity_Last, 0);
    else
      fullNbSeg += aVec[ entity ];
  }

  // store nb of segments computed by Netgen
  NCollection_Map<Link> linkMap;
  for (int i = 1; i <= ngMesh->GetNSeg(); ++i )
  {
    const netgen::Segment& seg = ngMesh->LineSegment(i);
#ifdef NETGEN_NEW
    Link link(seg.pnums[0], seg.pnums[1]);
#else
    Link link(seg.p1, seg.p2);
#endif
    if ( !linkMap.Add( link )) continue;
    int aGeomEdgeInd = seg.epgeominfo[0].edgenr;
    if (aGeomEdgeInd > 0 && aGeomEdgeInd <= occgeo.emap.Extent())
    {
      vector<int>& aVec = aResMap[_mesh->GetSubMesh(occgeo.emap(aGeomEdgeInd))];
      aVec[ entity ]++;
    }
  }
  // store nb of nodes on edges computed by Netgen
  TopTools_DataMapIteratorOfDataMapOfShapeInteger Edge2NbSegIt(Edge2NbSeg);
  for (; Edge2NbSegIt.More(); Edge2NbSegIt.Next())
  {
    vector<int>& aVec = aResMap[_mesh->GetSubMesh(Edge2NbSegIt.Key())];
    if ( aVec[ entity ] > 1 && aVec[ SMDSEntity_Node ] == 0 )
      aVec[SMDSEntity_Node] = mparams.secondorder > 0  ? 2*aVec[ entity ]-1 : aVec[ entity ]-1;

    fullNbSeg += aVec[ entity ];
    Edge2NbSeg( Edge2NbSegIt.Key() ) = aVec[ entity ];
  }

  // ----------------
  // evaluate 2D 
  // ----------------
  if ( _simpleHyp ) {
    if ( double area = _simpleHyp->GetMaxElementArea() ) {
      // face area
      mparams.maxh = sqrt(2. * area/sqrt(3.0));
      mparams.grading = 0.4; // moderate size growth
    }
    else {
      // length from edges
      mparams.maxh = fullLen/fullNbSeg;
      mparams.grading = 0.2; // slow size growth
    }
  }
  mparams.maxh = min( mparams.maxh, occgeo.boundingbox.Diam()/2 );
  mparams.maxh = min( mparams.maxh, fullLen/fullNbSeg * (1. + mparams.grading));

  for (TopExp_Explorer exp(_shape, TopAbs_FACE); exp.More(); exp.Next())
  {
    TopoDS_Face F = TopoDS::Face( exp.Current() );
    SMESH_subMesh *sm = _mesh->GetSubMesh(F);
    GProp_GProps G;
    BRepGProp::SurfaceProperties(F,G);
    double anArea = G.Mass();
    tooManyElems = tooManyElems || ( anArea/hugeNb > mparams.maxh*mparams.maxh );
    int nb1d = 0;
    if ( !tooManyElems )
    {
      TopTools_MapOfShape egdes;
      for (TopExp_Explorer exp1(F,TopAbs_EDGE); exp1.More(); exp1.Next())
        if ( egdes.Add( exp1.Current() ))
          nb1d += Edge2NbSeg.Find(exp1.Current());
    }
    int nbFaces = tooManyElems ? hugeNb : int( 4*anArea / (mparams.maxh*mparams.maxh*sqrt(3.)));
    int nbNodes = tooManyElems ? hugeNb : (( nbFaces*3 - (nb1d-1)*2 ) / 6 + 1 );

    vector<int> aVec(SMDSEntity_Last, 0);
    if( mparams.secondorder > 0 ) {
      int nb1d_in = (nbFaces*3 - nb1d) / 2;
      aVec[SMDSEntity_Node] = nbNodes + nb1d_in;
      aVec[SMDSEntity_Quad_Triangle] = nbFaces;
    }
    else {
      aVec[SMDSEntity_Node] = nbNodes;
      aVec[SMDSEntity_Triangle] = nbFaces;
    }
    aResMap[sm].swap(aVec);
  }

  // ----------------
  // evaluate 3D
  // ----------------
  if(_isVolume) {
    // pass 3D simple parameters to NETGEN
    const NETGENPlugin_SimpleHypothesis_3D* simple3d =
      dynamic_cast< const NETGENPlugin_SimpleHypothesis_3D* > ( _simpleHyp );
    if ( simple3d ) {
      if ( double vol = simple3d->GetMaxElementVolume() ) {
        // max volume
        mparams.maxh = pow( 72, 1/6. ) * pow( vol, 1/3. );
        mparams.maxh = min( mparams.maxh, occgeo.boundingbox.Diam()/2 );
      }
      else {
        // using previous length from faces
      }
      mparams.grading = 0.4;
      mparams.maxh = min( mparams.maxh, fullLen/fullNbSeg * (1. + mparams.grading));
    }
    GProp_GProps G;
    BRepGProp::VolumeProperties(_shape,G);
    double aVolume = G.Mass();
    double tetrVol = 0.1179*mparams.maxh*mparams.maxh*mparams.maxh;
    tooManyElems = tooManyElems || ( aVolume/hugeNb > tetrVol );
    int nbVols = tooManyElems ? hugeNb : int(aVolume/tetrVol);
    int nb1d_in = int(( nbVols*6 - fullNbSeg ) / 6 );
    vector<int> aVec(SMDSEntity_Last, 0 );
    if ( tooManyElems ) // avoid FPE
    {
      aVec[SMDSEntity_Node] = hugeNb;
      aVec[ mparams.secondorder > 0 ? SMDSEntity_Quad_Tetra : SMDSEntity_Tetra] = hugeNb;
    }
    else
    {
      if( mparams.secondorder > 0 ) {
        aVec[SMDSEntity_Node] = nb1d_in/3 + 1 + nb1d_in;
        aVec[SMDSEntity_Quad_Tetra] = nbVols;
      }
      else {
        aVec[SMDSEntity_Node] = nb1d_in/3 + 1;
        aVec[SMDSEntity_Tetra] = nbVols;
      }
    }
    SMESH_subMesh *sm = _mesh->GetSubMesh(_shape);
    aResMap[sm].swap(aVec);
  }

  return true;
}

//================================================================================
/*!
 * \brief Remove "test.out" and "problemfaces" files in current directory
 */
//================================================================================

void NETGENPlugin_Mesher::RemoveTmpFiles()
{
  SMESH_File("test.out").remove();
  SMESH_File("problemfaces").remove();
  SMESH_File("occmesh.rep").remove();
}

//================================================================================
/*!
 * \brief Read mesh entities preventing successful computation from "test.out" file
 */
//================================================================================

SMESH_ComputeErrorPtr
NETGENPlugin_Mesher::readErrors(const vector<const SMDS_MeshNode* >& nodeVec)
{
  SMESH_ComputeErrorPtr err = SMESH_ComputeError::New
    (COMPERR_BAD_INPUT_MESH, "Some edges multiple times in surface mesh");
  SMESH_File file("test.out");
  vector<int> edge(2);
  const char* badEdgeStr = " multiple times in surface mesh";
  const int   badEdgeStrLen = strlen( badEdgeStr );
  while( !file.eof() )
  {
    if ( strncmp( file, "Edge ", 5 ) == 0 &&
         file.getInts( edge ) &&
         strncmp( file, badEdgeStr, badEdgeStrLen ) == 0 &&
         edge[0] < nodeVec.size() && edge[1] < nodeVec.size())
    {
      err->myBadElements.push_back( new SMDS_MeshEdge( nodeVec[ edge[0]], nodeVec[ edge[1]] ));
      file += badEdgeStrLen;
    }
    else
    {
      ++file;
    }
  }
  return err;
}

//================================================================================
/*!
 * \brief Constructor of NETGENPlugin_ngMeshInfo
 */
//================================================================================

NETGENPlugin_ngMeshInfo::NETGENPlugin_ngMeshInfo( netgen::Mesh* ngMesh)
{
  if ( ngMesh )
  {
    _nbNodes    = ngMesh->GetNP();
    _nbSegments = ngMesh->GetNSeg();
    _nbFaces    = ngMesh->GetNSE();
    _nbVolumes  = ngMesh->GetNE();
  }
  else
  {
    _nbNodes = _nbSegments = _nbFaces = _nbVolumes = 0;
  }
}

//================================================================================
/*!
 * \brief Find "internal" sub-shapes
 */
//================================================================================

NETGENPlugin_Internals::NETGENPlugin_Internals( SMESH_Mesh&         mesh,
                                                const TopoDS_Shape& shape,
                                                bool                is3D )
  : _mesh( mesh ), _is3D( is3D )
{
  SMESHDS_Mesh* meshDS = mesh.GetMeshDS();

  TopExp_Explorer f,e;
  for ( f.Init( shape, TopAbs_FACE ); f.More(); f.Next() )
  {
    // find not computed internal edges

    for ( e.Init( f.Current().Oriented(TopAbs_FORWARD), TopAbs_EDGE ); e.More(); e.Next() )
      if ( e.Current().Orientation() == TopAbs_INTERNAL )
      {
        SMESH_subMesh* eSM = mesh.GetSubMesh( e.Current() );
        if ( eSM->IsEmpty() )
        {
          int faceID = meshDS->ShapeToIndex( f.Current() );
          _ev2face.insert( make_pair( eSM->GetId(), faceID ));
          for ( TopoDS_Iterator v(e.Current()); v.More(); v.Next() )
            _ev2face.insert( make_pair( meshDS->ShapeToIndex( v.Value() ), faceID ));
        }
      }
    if ( is3D )
    {
      // find internal faces and their subshapes where nodes are to be doubled

      if ( f.Current().Orientation() == TopAbs_INTERNAL )
      {
        _intShapes.insert( meshDS->ShapeToIndex( f.Current() ));

        // egdes
        list< TopoDS_Shape > edges;
        for ( e.Init( f.Current(), TopAbs_EDGE ); e.More(); e.Next())
          if ( SMESH_MesherHelper::NbAncestors( e.Current(), mesh, TopAbs_FACE ) > 1 )
          {
            _intShapes.insert( meshDS->ShapeToIndex( e.Current() ));
            edges.push_back( e.Current() );
            // find border faces
            PShapeIteratorPtr fIt =
              SMESH_MesherHelper::GetAncestors( edges.back(),mesh,TopAbs_FACE );
            while ( const TopoDS_Shape* pFace = fIt->next() )
              if ( !pFace->IsSame( f.Current() ))
                _borderFaces.insert( meshDS->ShapeToIndex( *pFace ));
          }
        // vertices
        // we consider vertex internal if it is shared by more than one internal edge
        list< TopoDS_Shape >::iterator edge = edges.begin();
        for ( ; edge != edges.end(); ++edge )
          for ( TopoDS_Iterator v( *edge ); v.More(); v.Next() )
          {
            set<int> internalEdges;
            PShapeIteratorPtr eIt =
              SMESH_MesherHelper::GetAncestors( v.Value(),mesh,TopAbs_EDGE );
            while ( const TopoDS_Shape* pEdge = eIt->next() )
            {
              int edgeID = meshDS->ShapeToIndex( *pEdge );
              if ( isInternalShape( edgeID ))
                internalEdges.insert( edgeID );
            }
            if ( internalEdges.size() > 1 )
              _intShapes.insert( meshDS->ShapeToIndex( v.Value() ));
          }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Find mesh faces on non-internal geom faces sharing internal edge
 * some nodes of which are to be doubled to make the second border of the "crack"
 */
//================================================================================

void NETGENPlugin_Internals::findBorderElements( TIDSortedElemSet & borderElems )
{
  if ( _intShapes.empty() ) return;

  SMESH_Mesh& mesh = const_cast<SMESH_Mesh&>(_mesh);
  SMESHDS_Mesh* meshDS = mesh.GetMeshDS();

  // loop on internal geom edges
  set<int>::const_iterator intShapeId = _intShapes.begin();
  for ( ; intShapeId != _intShapes.end(); ++intShapeId )
  {
    const TopoDS_Shape& s = meshDS->IndexToShape( *intShapeId );
    if ( s.ShapeType() != TopAbs_EDGE ) continue;

    // get internal and non-internal geom faces sharing the internal edge <s>
    int intFace = 0;
    set<int>::iterator bordFace = _borderFaces.end();
    PShapeIteratorPtr faces = SMESH_MesherHelper::GetAncestors( s, _mesh, TopAbs_FACE );
    while ( const TopoDS_Shape* pFace = faces->next() )
    {
      int faceID = meshDS->ShapeToIndex( *pFace );
      if ( isInternalShape( faceID ))
        intFace = faceID;
      else
        bordFace = _borderFaces.insert( faceID ).first;
    }
    if ( bordFace == _borderFaces.end() || !intFace ) continue;

    // get all links of mesh faces on internal geom face sharing nodes on edge <s>
    set< SMESH_OrientedLink > links; //!< links of faces on internal geom face
    list<const SMDS_MeshElement*> suspectFaces[2]; //!< mesh faces on border geom faces
    int nbSuspectFaces = 0;
    SMESHDS_SubMesh* intFaceSM = meshDS->MeshElements( intFace );
    if ( !intFaceSM || intFaceSM->NbElements() == 0 ) continue;
    SMESH_subMeshIteratorPtr smIt = mesh.GetSubMesh( s )->getDependsOnIterator(true,true);
    while ( smIt->more() )
    {
      SMESHDS_SubMesh* sm = smIt->next()->GetSubMeshDS();
      if ( !sm ) continue;
      SMDS_NodeIteratorPtr nIt = sm->GetNodes();
      while ( nIt->more() )
      {
        const SMDS_MeshNode* nOnEdge = nIt->next();
        SMDS_ElemIteratorPtr fIt = nOnEdge->GetInverseElementIterator(SMDSAbs_Face);
        while ( fIt->more() )
        {
          const SMDS_MeshElement* f = fIt->next();
          int nbNodes = f->NbNodes() / ( f->IsQuadratic() ? 2 : 1 );
          if ( intFaceSM->Contains( f ))
          {
            for ( int i = 0; i < nbNodes; ++i )
              links.insert( SMESH_OrientedLink( f->GetNode(i), f->GetNode((i+1)%nbNodes)));
          }
          else
          {
            int nbDblNodes = 0;
            for ( int i = 0; i < nbNodes; ++i )
              nbDblNodes += isInternalShape( f->GetNode(i)->GetPosition()->GetShapeId() );
            if ( nbDblNodes )
              suspectFaces[ nbDblNodes < 2 ].push_back( f );
            nbSuspectFaces++;
          }
        }
      }
    }
    // suspectFaces[0] having link with same orientation as mesh faces on
    // the internal geom face are <borderElems>. suspectFaces[1] have
    // only one node on edge s, we decide on them later (at the 2nd loop)
    // by links of <borderElems> found at the 1st and 2nd loops
    set< SMESH_OrientedLink > borderLinks;
    for ( int isPostponed = 0; isPostponed < 2; ++isPostponed )
    {
      list<const SMDS_MeshElement*>::iterator fIt = suspectFaces[isPostponed].begin();
      for ( int nbF = 0; fIt != suspectFaces[isPostponed].end(); ++fIt, ++nbF )
      {
        const SMDS_MeshElement* f = *fIt;
        bool isBorder = false, linkFound = false, borderLinkFound = false;
        list< SMESH_OrientedLink > faceLinks;
        int nbNodes = f->NbNodes() / ( f->IsQuadratic() ? 2 : 1 );
        for ( int i = 0; i < nbNodes; ++i )
        {
          SMESH_OrientedLink link( f->GetNode(i), f->GetNode((i+1)%nbNodes));
          faceLinks.push_back( link );
          if ( !linkFound )
          {
            set< SMESH_OrientedLink >::iterator foundLink = links.find( link );
            if ( foundLink != links.end() )
            {
              linkFound= true;
              isBorder = ( foundLink->_reversed == link._reversed );
              if ( !isBorder && !isPostponed ) break;
              faceLinks.pop_back();
            }
            else if ( isPostponed && !borderLinkFound )
            {
              foundLink = borderLinks.find( link );
              if ( foundLink != borderLinks.end() )
              {
                borderLinkFound = true;
                isBorder = ( foundLink->_reversed != link._reversed );
              }
            }
          }
        }
        if ( isBorder )
        {
          borderElems.insert( f );
          borderLinks.insert( faceLinks.begin(), faceLinks.end() );
        }
        else if ( !linkFound && !borderLinkFound )
        {
          suspectFaces[1].push_back( f );
          if ( nbF > 2 * nbSuspectFaces )
            break; // dead loop protection
        }
      }
    }
//     TIDSortedElemSet posponedFaces;
//     set< SMESH_OrientedLink > borderLinks;
//     TIDSortedElemSet::iterator fIt = suspectFaces.begin();
//     for ( ; fIt != suspectFaces.end(); ++fIt )
//     {
//       const SMDS_MeshElement* f = *fIt;
//       bool linkFound = false, isBorder = false;
//       list< SMESH_OrientedLink > faceLinks;
//       int nbNodes = f->NbNodes() / ( f->IsQuadratic() ? 2 : 1 );
//       for ( int i = 0; i < nbNodes; ++i )
//       {
//         SMESH_OrientedLink link( f->GetNode(i), f->GetNode((i+1)%nbNodes));
//         faceLinks.push_back( link );
//         if ( !linkFound )
//         {
//           set< SMESH_OrientedLink >::iterator foundLink = links.find( link );
//           if ( foundLink != links.end() )
//           {
//             linkFound= true;
//             isBorder = ( foundLink->_reversed == link._reversed );
//             if ( !isBorder ) break;
//           }
//         }
//       }
//       if ( isBorder )
//       {
//         borderElems.insert( f );
//         borderLinks.insert( faceLinks.begin(), faceLinks.end() );
//       }
//       else if ( !linkFound )
//       {
//         posponedFaces.insert( f );
//       }
//     }
//     // decide on posponedFaces
//     for ( fIt = posponedFaces.begin(); fIt != posponedFaces.end(); ++fIt )
//     {
//       const SMDS_MeshElement* f = *fIt;
//       int nbNodes = f->NbNodes() / ( f->IsQuadratic() ? 2 : 1 );
//       for ( int i = 0; i < nbNodes; ++i )
//       {
//         SMESH_OrientedLink link( f->GetNode(i), f->GetNode((i+1)%nbNodes));
//         set< SMESH_OrientedLink >::iterator foundLink = borderLinks.find( link );
//         if ( foundLink != borderLinks.end() )
//         {
//           if ( foundLink->_reversed != link._reversed )
//             borderElems.insert( f );
//           break;
//         }
//       }
//     }
  }
}

//================================================================================
/*!
 * \brief put internal shapes in maps and fill in submeshes to precompute
 */
//================================================================================

void NETGENPlugin_Internals::getInternalEdges( TopTools_IndexedMapOfShape& fmap,
                                               TopTools_IndexedMapOfShape& emap,
                                               TopTools_IndexedMapOfShape& vmap,
                                               list< SMESH_subMesh* >& smToPrecompute)
{
  if ( !hasInternalEdges() ) return;
  map<int,int>::const_iterator ev_face = _ev2face.begin();
  for ( ; ev_face != _ev2face.end(); ++ev_face )
  {
    const TopoDS_Shape& ev   = _mesh.GetMeshDS()->IndexToShape( ev_face->first );
    const TopoDS_Shape& face = _mesh.GetMeshDS()->IndexToShape( ev_face->second );

    ( ev.ShapeType() == TopAbs_EDGE ? emap : vmap ).Add( ev );
    fmap.Add( face );
    //cout<<"INTERNAL EDGE or VERTEX "<<ev_face->first<<" on face "<<ev_face->second<<endl;

    smToPrecompute.push_back( _mesh.GetSubMeshContaining( ev_face->first ));
  }
}

//================================================================================
/*!
 * \brief return shapes and submeshes to be meshed and already meshed boundary submeshes
 */
//================================================================================

void NETGENPlugin_Internals::getInternalFaces( TopTools_IndexedMapOfShape& fmap,
                                               TopTools_IndexedMapOfShape& emap,
                                               list< SMESH_subMesh* >&     intFaceSM,
                                               list< SMESH_subMesh* >&     boundarySM)
{
  if ( !hasInternalFaces() ) return;

  // <fmap> and <emap> are for not yet meshed shapes
  // <intFaceSM> is for submeshes of faces
  // <boundarySM> is for meshed edges and vertices

  intFaceSM.clear();
  boundarySM.clear();

  set<int> shapeIDs ( _intShapes );
  if ( !_borderFaces.empty() )
    shapeIDs.insert( _borderFaces.begin(), _borderFaces.end() );

  set<int>::const_iterator intS = shapeIDs.begin();
  for ( ; intS != shapeIDs.end(); ++intS )
  {
    SMESH_subMesh* sm = _mesh.GetSubMeshContaining( *intS );

    if ( sm->GetSubShape().ShapeType() != TopAbs_FACE ) continue;

    intFaceSM.push_back( sm );

    // add submeshes of not computed internal faces
    if ( !sm->IsEmpty() ) continue;

    SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(true,true);
    while ( smIt->more() )
    {
      sm = smIt->next();
      const TopoDS_Shape& s = sm->GetSubShape();

      if ( sm->IsEmpty() )
      {
        // not yet meshed
        switch ( s.ShapeType() ) {
        case TopAbs_FACE: fmap.Add ( s ); break;
        case TopAbs_EDGE: emap.Add ( s ); break;
        default:;
        }
      }
      else
      {
        if ( s.ShapeType() != TopAbs_FACE )
          boundarySM.push_back( sm );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Return true if given shape is to be precomputed in order to be correctly
 * added to netgen mesh
 */
//================================================================================

bool NETGENPlugin_Internals::isShapeToPrecompute(const TopoDS_Shape& s)
{
  int shapeID = _mesh.GetMeshDS()->ShapeToIndex( s );
  switch ( s.ShapeType() ) {
  case TopAbs_FACE  : return isInternalShape( shapeID ) || isBorderFace( shapeID );
  case TopAbs_EDGE  : return isInternalEdge( shapeID );
  case TopAbs_VERTEX: return false; //isInternalVertex( shapeID );
  default:;
  }
  return false;
}

//================================================================================
/*!
 * \brief Initialize netgen library
 */
//================================================================================

NETGENPlugin_NetgenLibWrapper::NETGENPlugin_NetgenLibWrapper()
{
  Ng_Init();
  _ngMesh = Ng_NewMesh();
}

//================================================================================
/*!
 * \brief Finish using netgen library
 */
//================================================================================

NETGENPlugin_NetgenLibWrapper::~NETGENPlugin_NetgenLibWrapper()
{
  Ng_DeleteMesh( _ngMesh );
  Ng_Exit();
  NETGENPlugin_Mesher::RemoveTmpFiles();
}

//================================================================================
/*!
 * \brief Set netgen mesh to delete at destruction
 */
//================================================================================

void NETGENPlugin_NetgenLibWrapper::setMesh( Ng_Mesh* mesh )
{
  if ( _ngMesh )
    Ng_DeleteMesh( _ngMesh );
  _ngMesh = mesh;
}
