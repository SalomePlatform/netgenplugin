// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File      : NETGENPlugin_NETGEN_2D_ONLY.cxx
// Author    : Edward AGAPOV (OCC)
// Project   : SALOME
//
#include "NETGENPlugin_NETGEN_2D_ONLY.hxx"

#include "NETGENPlugin_Mesher.hxx"
#include "NETGENPlugin_Hypothesis_2D.hxx"

#include <SMDS_MeshElement.hxx>
#include <SMDS_MeshNode.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_Gen.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_subMesh.hxx>
#include <StdMeshers_FaceSide.hxx>
#include <StdMeshers_LengthFromEdges.hxx>
#include <StdMeshers_MaxElementArea.hxx>
#include <StdMeshers_QuadranglePreference.hxx>
#include <StdMeshers_ViscousLayers2D.hxx>

#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>

#include <utilities.h>

#include <list>
#include <vector>
#include <limits>

/*
  Netgen include files
*/
namespace nglib {
#include <nglib.h>
}
#ifndef OCCGEOMETRY
#define OCCGEOMETRY
#endif
#include <occgeom.hpp>
#include <meshing.hpp>
//#include <meshtype.hpp>
namespace netgen {
  extern int OCCGenerateMesh (OCCGeometry&, Mesh*&, int, int, char*);
  extern MeshingParameters mparam;
}

using namespace std;
using namespace netgen;
using namespace nglib;

//#define DUMP_SEGMENTS

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_2D_ONLY::NETGENPlugin_NETGEN_2D_ONLY(int hypId, int studyId,
                                                         SMESH_Gen* gen)
  : SMESH_2D_Algo(hypId, studyId, gen)
{
  MESSAGE("NETGENPlugin_NETGEN_2D_ONLY::NETGENPlugin_NETGEN_2D_ONLY");
  _name = "NETGEN_2D_ONLY";
  
  _shapeType = (1 << TopAbs_FACE);// 1 bit /shape type

  _compatibleHypothesis.push_back("MaxElementArea");
  _compatibleHypothesis.push_back("LengthFromEdges");
  _compatibleHypothesis.push_back("QuadranglePreference");
  _compatibleHypothesis.push_back("NETGEN_Parameters_2D");
  _compatibleHypothesis.push_back("ViscousLayers2D");

  _hypMaxElementArea = 0;
  _hypLengthFromEdges = 0;
  _hypQuadranglePreference = 0;
  _hypParameters = 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_2D_ONLY::~NETGENPlugin_NETGEN_2D_ONLY()
{
  MESSAGE("NETGENPlugin_NETGEN_2D_ONLY::~NETGENPlugin_NETGEN_2D_ONLY");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool NETGENPlugin_NETGEN_2D_ONLY::CheckHypothesis (SMESH_Mesh&         aMesh,
                                                   const TopoDS_Shape& aShape,
                                                   Hypothesis_Status&  aStatus)
{
  _hypMaxElementArea = 0;
  _hypLengthFromEdges = 0;
  _hypQuadranglePreference = 0;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape, false);

  if (hyps.empty())
  {
    aStatus = HYP_OK; //SMESH_Hypothesis::HYP_MISSING;
    return true;  // (PAL13464) can work with no hypothesis, LengthFromEdges is default one
  }

  aStatus = HYP_MISSING;

  list<const SMESHDS_Hypothesis*>::const_iterator ith;
  for (ith = hyps.begin(); ith != hyps.end(); ++ith )
  {
    const SMESHDS_Hypothesis* hyp = (*ith);

    string hypName = hyp->GetName();

    if      ( hypName == "MaxElementArea")
      _hypMaxElementArea = static_cast<const StdMeshers_MaxElementArea*> (hyp);
    else if ( hypName == "LengthFromEdges" )
      _hypLengthFromEdges = static_cast<const StdMeshers_LengthFromEdges*> (hyp);
    else if ( hypName == "QuadranglePreference" )
      _hypQuadranglePreference = static_cast<const StdMeshers_QuadranglePreference*>(hyp);
    else if ( hypName == "NETGEN_Parameters_2D" )
      _hypParameters = static_cast<const NETGENPlugin_Hypothesis_2D*>(hyp);
    else if ( hypName == StdMeshers_ViscousLayers2D::GetHypType() )
      continue;
    else {
      aStatus = HYP_INCOMPATIBLE;
      return false;
    }
  }

  int nbHyps = bool(_hypMaxElementArea) + bool(_hypLengthFromEdges) + bool(_hypParameters );
  if ( nbHyps > 1 )
    aStatus = HYP_CONCURENT;
  else
    aStatus = HYP_OK;

  return ( aStatus == HYP_OK );
}

//=============================================================================
/*!
 *Here we are going to use the NETGEN mesher
 */
//=============================================================================

bool NETGENPlugin_NETGEN_2D_ONLY::Compute(SMESH_Mesh&         aMesh,
                                          const TopoDS_Shape& aShape)
{
#ifdef WITH_SMESH_CANCEL_COMPUTE
  netgen::multithread.terminate = 0;
#endif
  MESSAGE("NETGENPlugin_NETGEN_2D_ONLY::Compute()");

  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  int faceID = meshDS->ShapeToIndex( aShape );

  SMESH_MesherHelper helper(aMesh);
  _quadraticMesh = helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );
  const bool ignoreMediumNodes = _quadraticMesh;
  
  // build viscous layers if required
  const TopoDS_Face F = TopoDS::Face( aShape.Oriented( TopAbs_FORWARD ));
  SMESH_ProxyMesh::Ptr proxyMesh = StdMeshers_ViscousLayers2D::Compute( aMesh, F );
  if ( !proxyMesh )
    return false;

  // ------------------------
  // get all edges of a face
  // ------------------------
  TError problem;
  TSideVector wires =
    StdMeshers_FaceSide::GetFaceWires( F, aMesh, ignoreMediumNodes, problem, proxyMesh );
  if ( problem && !problem->IsOK() )
    return error( problem );
  int nbWires = wires.size();
  if ( nbWires == 0 )
    return error( "Problem in StdMeshers_FaceSide::GetFaceWires()");
  if ( wires[0]->NbSegments() < 3 ) // ex: a circle with 2 segments
    return error(COMPERR_BAD_INPUT_MESH,
                 SMESH_Comment("Too few segments: ")<<wires[0]->NbSegments());

  // --------------------
  // compute edge length
  // --------------------

  NETGENPlugin_Mesher aMesher( &aMesh, aShape, /*isVolume=*/false);
  netgen::OCCGeometry occgeo;
  aMesher.PrepareOCCgeometry( occgeo, F, aMesh );
  occgeo.fmap.Clear(); // face can be reversed, which is wrong in this case (issue 19978)
  occgeo.fmap.Add( F );

  if ( _hypParameters )
  {
    aMesher.SetParameters(_hypParameters);
  }
  else
  {
    double edgeLength = 0;
    if (_hypLengthFromEdges || (!_hypLengthFromEdges && !_hypMaxElementArea))
    {
      int nbSegments = 0;
      for ( int iW = 0; iW < nbWires; ++iW )
      {
        edgeLength += wires[ iW ]->Length();
        nbSegments += wires[ iW ]->NbSegments();
      }
      if ( nbSegments )
        edgeLength /= nbSegments;
    }
    if ( _hypMaxElementArea )
    {
      double maxArea = _hypMaxElementArea->GetMaxArea();
      edgeLength = sqrt(2. * maxArea/sqrt(3.0));
    }
    if ( edgeLength < DBL_MIN )
      edgeLength = occgeo.GetBoundingBox().Diam();

    netgen::mparam.maxh = edgeLength;
    netgen::mparam.minh = aMesher.GetDefaultMinSize( aShape, netgen::mparam.maxh );
    netgen::mparam.quad = _hypQuadranglePreference ? 1 : 0;
    netgen::mparam.grading = 0.7; // very coarse mesh by default
  }
#ifdef NETGEN_NEW
  occgeo.face_maxh = netgen::mparam.maxh;
#endif

  // -------------------------
  // Make input netgen mesh
  // -------------------------

  NETGENPlugin_NetgenLibWrapper ngLib;
  netgen::Mesh * ngMesh = (netgen::Mesh*) ngLib._ngMesh;

  Box<3> bb = occgeo.GetBoundingBox();
  bb.Increase (bb.Diam()/10);
  ngMesh->SetLocalH (bb.PMin(), bb.PMax(), netgen::mparam.grading);
  ngMesh->SetGlobalH (netgen::mparam.maxh);

  vector< const SMDS_MeshNode* > nodeVec;
  problem = aMesher.AddSegmentsToMesh( *ngMesh, occgeo, wires, helper, nodeVec );
  if ( problem && !problem->IsOK() )
    return error( problem );

  // -------------------------
  // Generate surface mesh
  // -------------------------

  char *optstr = 0;
  int startWith = MESHCONST_MESHSURFACE;
  int endWith   = MESHCONST_OPTSURFACE;
  int err = 1;

  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    err = netgen::OCCGenerateMesh(occgeo, ngMesh, startWith, endWith, optstr);
#ifdef WITH_SMESH_CANCEL_COMPUTE
    if(netgen::multithread.terminate)
      return false;
#endif
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
  catch (...) {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    str << " at " << netgen::multithread.task;
    error(str);
  }

  // ----------------------------------------------------
  // Fill the SMESHDS with the generated nodes and faces
  // ----------------------------------------------------

  int nbNodes = ngMesh->GetNP();
  int nbFaces = ngMesh->GetNSE();

  int nbInputNodes = nodeVec.size()-1;
  nodeVec.resize( nbNodes+1, 0 );

  // add nodes
  for ( int ngID = nbInputNodes + 1; ngID <= nbNodes; ++ngID )
  {
    const MeshPoint& ngPoint = ngMesh->Point( ngID );
#ifdef NETGEN_NEW
    SMDS_MeshNode * node = meshDS->AddNode(ngPoint(0), ngPoint(1), ngPoint(2));
#else
    SMDS_MeshNode * node = meshDS->AddNode(ngPoint.X(), ngPoint.Y(), ngPoint.Z());
#endif
    nodeVec[ ngID ] = node;
  }

  // create faces
  bool reverse = ( aShape.Orientation() == TopAbs_REVERSED );
  int i,j;
  for ( i = 1; i <= nbFaces ; ++i )
  {
    const Element2d& elem = ngMesh->SurfaceElement(i);
    vector<const SMDS_MeshNode*> nodes( elem.GetNP() );
    for (j=1; j <= elem.GetNP(); ++j)
    {
      int pind = elem.PNum(j);
      if ( pind < 1 )
        break;
      const SMDS_MeshNode* node = nodeVec[ pind ];
      if ( reverse )
        nodes[ nodes.size()-j ] = node;
      else
        nodes[ j-1 ] = node;
      if ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_3DSPACE )
      {
        const PointGeomInfo& pgi = elem.GeomInfoPi(j);
        meshDS->SetNodeOnFace((SMDS_MeshNode*)node, faceID, pgi.u, pgi.v);
      }
    }
    if ( j > elem.GetNP() )
    {
      SMDS_MeshFace* face = 0;
      if ( elem.GetType() == TRIG )
        face = helper.AddFace(nodes[0],nodes[1],nodes[2]);
      else
        face = helper.AddFace(nodes[0],nodes[1],nodes[2],nodes[3]);
    }
  }

  return !err;
}

#ifdef WITH_SMESH_CANCEL_COMPUTE
void NETGENPlugin_NETGEN_2D_ONLY::CancelCompute()
{
  SMESH_Algo::CancelCompute();
  netgen::multithread.terminate = 1;
}
#endif

//=============================================================================
/*!
 *
 */
//=============================================================================

bool NETGENPlugin_NETGEN_2D_ONLY::Evaluate(SMESH_Mesh& aMesh,
                                           const TopoDS_Shape& aShape,
                                           MapShapeNbElems& aResMap)
{
  TopoDS_Face F = TopoDS::Face(aShape);
  if(F.IsNull())
    return false;

  // collect info from edges
  int nb0d = 0, nb1d = 0;
  bool IsQuadratic = false;
  bool IsFirst = true;
  double fullLen = 0.0;
  TopTools_MapOfShape tmpMap;
  for (TopExp_Explorer exp(F, TopAbs_EDGE); exp.More(); exp.Next()) {
    TopoDS_Edge E = TopoDS::Edge(exp.Current());
    if( tmpMap.Contains(E) )
      continue;
    tmpMap.Add(E);
    SMESH_subMesh *aSubMesh = aMesh.GetSubMesh(exp.Current());
    MapShapeNbElemsItr anIt = aResMap.find(aSubMesh);
    if( anIt==aResMap.end() ) {
      SMESH_subMesh *sm = aMesh.GetSubMesh(F);
      SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
      smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,"Submesh can not be evaluated",this));
      return false;
    }
    std::vector<int> aVec = (*anIt).second;
    nb0d += aVec[SMDSEntity_Node];
    nb1d += Max(aVec[SMDSEntity_Edge],aVec[SMDSEntity_Quad_Edge]);
    double aLen = SMESH_Algo::EdgeLength(E);
    fullLen += aLen;
    if(IsFirst) {
      IsQuadratic = (aVec[SMDSEntity_Quad_Edge] > aVec[SMDSEntity_Edge]);
      IsFirst = false;
    }
  }
  tmpMap.Clear();

  // compute edge length
  double ELen = 0;
  if (_hypLengthFromEdges || !_hypLengthFromEdges && !_hypMaxElementArea) {
    if ( nb1d > 0 )
      ELen = fullLen / nb1d;
  }
  if ( _hypMaxElementArea ) {
    double maxArea = _hypMaxElementArea->GetMaxArea();
    ELen = sqrt(2. * maxArea/sqrt(3.0));
  }
  GProp_GProps G;
  BRepGProp::SurfaceProperties(F,G);
  double anArea = G.Mass();

  const int hugeNb = numeric_limits<int>::max()/10;
  if ( anArea / hugeNb > ELen*ELen )
  {
    SMESH_subMesh *sm = aMesh.GetSubMesh(F);
    SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
    smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,"Submesh can not be evaluated.\nToo small element length",this));
    return false;
  }
  int nbFaces = (int) ( anArea / ( ELen*ELen*sqrt(3.) / 4 ) );
  int nbNodes = (int) ( ( nbFaces*3 - (nb1d-1)*2 ) / 6 + 1 );
  std::vector<int> aVec(SMDSEntity_Last);
  for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aVec[i]=0;
  if( IsQuadratic ) {
    aVec[SMDSEntity_Node] = nbNodes;
    aVec[SMDSEntity_Quad_Triangle] = nbFaces;
  }
  else {
    aVec[SMDSEntity_Node] = nbNodes;
    aVec[SMDSEntity_Triangle] = nbFaces;
  }
  SMESH_subMesh *sm = aMesh.GetSubMesh(F);
  aResMap.insert(std::make_pair(sm,aVec));

  return true;
}
