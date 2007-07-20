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
// File      : NETGENPlugin_Mesher.cxx
// Author    : Michael Sazonov (OCN)
// Date      : 31/03/2006
// Project   : SALOME
// $Header$
//=============================================================================
using namespace std;

#include "NETGENPlugin_Mesher.hxx"
#include "NETGENPlugin_Hypothesis_2D.hxx"

#include <SMESH_Mesh.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_ComputeError.hxx>
#include <SMESH_subMesh.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMDS_MeshElement.hxx>
#include <SMDS_MeshNode.hxx>
#include <utilities.h>

#include <vector>

#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <NCollection_Map.hxx>
#include <OSD_Path.hxx>
#include <OSD_File.hxx>

// Netgen include files
namespace nglib {
#include <nglib.h>
}
#define OCCGEOMETRY
#include <occgeom.hpp>
#include <meshing.hpp>
//#include <ngexception.hpp>
namespace netgen {
  extern int OCCGenerateMesh (OCCGeometry&, Mesh*&, int, int, char*);
  extern MeshingParameters mparam;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_Mesher::NETGENPlugin_Mesher (SMESH_Mesh* mesh,
                                          const TopoDS_Shape& aShape,
                                          const bool isVolume)
  : _mesh  (mesh),
    _shape   (aShape),
    _isVolume(isVolume),
    _optimize(true)
{
  // Initialize global NETGEN parameters by default values:
  // maximal mesh edge size
  netgen::mparam.maxh = NETGENPlugin_Hypothesis::GetDefaultMaxSize();
  // minimal number of segments per edge
  netgen::mparam.segmentsperedge = NETGENPlugin_Hypothesis::GetDefaultNbSegPerEdge();
  // rate of growth of size between elements
  netgen::mparam.grading = NETGENPlugin_Hypothesis::GetDefaultGrowthRate();
  // safety factor for curvatures (elements per radius)
  netgen::mparam.curvaturesafety = NETGENPlugin_Hypothesis::GetDefaultNbSegPerRadius();
  // create elements of second order
  netgen::mparam.secondorder = NETGENPlugin_Hypothesis::GetDefaultSecondOrder() ? 1 : 0;
  // quad-dominated surface meshing
  if (_isVolume)
    netgen::mparam.quad = 0;
  else
    netgen::mparam.quad = NETGENPlugin_Hypothesis_2D::GetDefaultQuadAllowed() ? 1 : 0;
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
    // Initialize global NETGEN parameters:
    // maximal mesh segment size
    netgen::mparam.maxh = hyp->GetMaxSize();
    // minimal number of segments per edge
    netgen::mparam.segmentsperedge = hyp->GetNbSegPerEdge();
    // rate of growth of size between elements
    netgen::mparam.grading = hyp->GetGrowthRate();
    // safety factor for curvatures (elements per radius)
    netgen::mparam.curvaturesafety = hyp->GetNbSegPerRadius();
    // create elements of second order
    netgen::mparam.secondorder = hyp->GetSecondOrder() ? 1 : 0;
    // quad-dominated surface meshing
    // only triangles are allowed for volumic mesh
    if (!_isVolume)
      netgen::mparam.quad = static_cast<const NETGENPlugin_Hypothesis_2D*>
        (hyp)->GetQuadAllowed() ? 1 : 0;
    _optimize = hyp->GetOptimize();
  }
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

void NETGENPlugin_Mesher::PrepareOCCgeometry(netgen::OCCGeometry& occgeo,
                                             const TopoDS_Shape&  shape)
{
  occgeo.shape = shape;
  occgeo.changed = 1;
  occgeo.BuildFMap();
  BRepTools::Clean (shape);
  BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh (shape, 0.01, true);
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
}

//=============================================================================
/*!
 * Here we are going to use the NETGEN mesher
 */
//=============================================================================
bool NETGENPlugin_Mesher::Compute()
{
  MESSAGE("Compute with:\n"
          " max size = " << netgen::mparam.maxh << "\n"
          " segments per edge = " << netgen::mparam.segmentsperedge);
  MESSAGE("\n"
          " growth rate = " << netgen::mparam.grading << "\n"
          " elements per radius = " << netgen::mparam.curvaturesafety << "\n"
          " second order = " << netgen::mparam.secondorder << "\n"
          " quad allowed = " << netgen::mparam.quad);

  SMESH_ComputeErrorPtr error = SMESH_ComputeError::New();
  nglib::Ng_Init();

  // -------------------------
  // Prepare OCC geometry
  // -------------------------

  netgen::OCCGeometry occgeo;
  PrepareOCCgeometry( occgeo, _shape );

  // -------------------------
  // Generate the mesh
  // -------------------------

  netgen::Mesh *ngMesh = NULL;
  // we start always with ANALYSE,
  // but end depending on _optimize and _isVolume
  int startWith = netgen::MESHCONST_ANALYSE;
  int endWith = (_optimize
                 ? (_isVolume ? netgen::MESHCONST_OPTVOLUME : netgen::MESHCONST_OPTSURFACE)
                 : netgen::MESHCONST_MESHSURFACE);
  char *optstr;

  int err = 0;
  SMESH_Comment comment;
  try
  {
    err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
    if (err) comment << "Error in netgen::OCCGenerateMesh()";
    if (!err && !_optimize)
    {
      // we have got surface mesh only, so generate volume mesh
      startWith = endWith = netgen::MESHCONST_MESHVOLUME;
      err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
      if (err) comment << "Error in netgen::OCCGenerateMesh()";
    }
    if (!err && netgen::mparam.secondorder > 0)
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

  MESSAGE((err ? "Mesh Generation failure" : "End of Mesh Generation") <<
          ", nb nodes: " << nbNod <<
          ", nb segments: " << nbSeg <<
          ", nb faces: " << nbFac <<
          ", nb volumes: " << nbVol);

  // -----------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Elements
  // -----------------------------------------------------------

  SMESHDS_Mesh* meshDS = _mesh->GetMeshDS();
  bool isOK = ( !err && (_isVolume ? (nbVol > 0) : (nbFac > 0)) );
  if ( true /*isOK*/ ) // get whatever built
  {
    // vector of nodes in which node index == netgen ID
    vector< SMDS_MeshNode* > nodeVec ( nbNod + 1 );
    // map of nodes assigned to submeshes
    NCollection_Map<int> pindMap;
    // create and insert nodes into nodeVec
    int i;
    for (i = 1; i <= nbNod /*&& isOK*/; ++i )
    {
      const netgen::MeshPoint& ngPoint = ngMesh->Point(i);
      SMDS_MeshNode* node = NULL;
      bool newNodeOnVertex = false;
      TopoDS_Vertex aVert;
      if (i <= occgeo.vmap.Extent())
      {
        // point on vertex
        aVert = TopoDS::Vertex(occgeo.vmap(i));
        SMESHDS_SubMesh * submesh = meshDS->MeshElements(aVert);
        if (submesh)
        {
          SMDS_NodeIteratorPtr it = submesh->GetNodes();
          if (it->more())
          {
            node = const_cast<SMDS_MeshNode*> (it->next());
            pindMap.Add(i);
          }
        }
        if (!node)
          newNodeOnVertex = true;
      }
      if (!node)
        node = meshDS->AddNode(ngPoint.X(), ngPoint.Y(), ngPoint.Z());
      if (!node)
      {
        MESSAGE("Cannot create a mesh node");
        if ( !comment.size() ) comment << "Cannot create a mesh node";
        nbSeg = nbFac = nbVol = isOK = 0;
        break;
      }
      nodeVec.at(i) = node;
      if (newNodeOnVertex)
      {
        // point on vertex
        meshDS->SetNodeOnVertex(node, aVert);
        pindMap.Add(i);
      }
    }

    // create mesh segments along geometric edges
    NCollection_Map<Link> linkMap;
    for (i = 1; i <= nbSeg/* && isOK*/; ++i )
    {
      const netgen::Segment& seg = ngMesh->LineSegment(i);
      Link link(seg.p1, seg.p2);
      if (linkMap.Contains(link))
        continue;
      linkMap.Add(link);
      TopoDS_Edge aEdge;
      int pinds[3] = { seg.p1, seg.p2, seg.pmid };
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
        if (pindMap.Contains(pind))
          continue;
        if (!aEdge.IsNull())
        {
          meshDS->SetNodeOnEdge(nodeVec.at(pind), aEdge, param);
          pindMap.Add(pind);
        }
      }
      SMDS_MeshEdge* edge;
      if (nbp < 3) // second order ?
        edge = meshDS->AddEdge(nodeVec.at(pinds[0]), nodeVec.at(pinds[1]));
      else
        edge = meshDS->AddEdge(nodeVec.at(pinds[0]), nodeVec.at(pinds[1]),
                                nodeVec.at(pinds[2]));
      if (!edge)
      {
        if ( !comment.size() ) comment << "Cannot create a mesh edge";
        MESSAGE("Cannot create a mesh edge");
        nbSeg = nbFac = nbVol = isOK = 0;
        break;
      }
      if (!aEdge.IsNull())
        meshDS->SetMeshElementOnShape(edge, aEdge);
    }

    // create mesh faces along geometric faces
    for (i = 1; i <= nbFac/* && isOK*/; ++i )
    {
      const netgen::Element2d& elem = ngMesh->SurfaceElement(i);
      int aGeomFaceInd = elem.GetIndex();
      TopoDS_Face aFace;
      if (aGeomFaceInd > 0 && aGeomFaceInd <= occgeo.fmap.Extent())
        aFace = TopoDS::Face(occgeo.fmap(aGeomFaceInd));
      vector<SMDS_MeshNode*> nodes;
      for (int j=1; j <= elem.GetNP(); ++j)
      {
        int pind = elem.PNum(j);
        SMDS_MeshNode* node = nodeVec.at(pind);
        nodes.push_back(node);
        if (pindMap.Contains(pind))
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
        nbSeg = nbFac = nbVol = isOK = 0;
        break;
      }
      if (!aFace.IsNull())
        meshDS->SetMeshElementOnShape(face, aFace);
    }

    // create tetrahedra
    for (i = 1; i <= nbVol/* && isOK*/; ++i)
    {
      const netgen::Element& elem = ngMesh->VolumeElement(i);
      int aSolidInd = elem.GetIndex();
      TopoDS_Solid aSolid;
      if (aSolidInd > 0 && aSolidInd <= occgeo.somap.Extent())
        aSolid = TopoDS::Solid(occgeo.somap(aSolidInd));
      vector<SMDS_MeshNode*> nodes;
      for (int j=1; j <= elem.GetNP(); ++j)
      {
        int pind = elem.PNum(j);
        SMDS_MeshNode* node = nodeVec.at(pind);
        nodes.push_back(node);
        if (pindMap.Contains(pind))
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
        nbSeg = nbFac = nbVol = isOK = 0;
        break;
      }
      if (!aSolid.IsNull())
        meshDS->SetMeshElementOnShape(vol, aSolid);
    }
  }

  if ( error->IsOK() && ( !isOK || comment.size() > 0 ))
    error->myName = COMPERR_ALGO_FAILED;
  if ( !comment.empty() )
    error->myComment = comment;

  // set bad compute error to subshapes of all failed subshapes shapes
  if ( !error->IsOK() && err )
  {
    for (int i = 1; i <= occgeo.fmap.Extent(); i++) {
      int status = occgeo.facemeshstatus[i-1];
      if (status == 1 ) continue;
      if ( SMESH_subMesh* sm = _mesh->GetSubMeshContaining( occgeo.fmap( i ))) {
        SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
        if ( !smError || smError->IsOK() ) {
          if ( status == -1 )
            smError.reset( new SMESH_ComputeError( error->myName, error->myComment ));
          else
            smError.reset( new SMESH_ComputeError( COMPERR_ALGO_FAILED, "Ignored" ));
        }
      }
    }
  }

  nglib::Ng_DeleteMesh((nglib::Ng_Mesh*)ngMesh);
  nglib::Ng_Exit();

  RemoveTmpFiles();

  return error->IsOK();
}

//================================================================================
/*!
 * \brief Remove "test.out" and "problemfaces" files in current directory
 */
//================================================================================

void NETGENPlugin_Mesher::RemoveTmpFiles()
{
  OSD_File( OSD_Path("test.out") ).Remove();
  OSD_File( OSD_Path("problemfaces") ).Remove();
}
