//=============================================================================
// File      : NETGENPlugin_NETGEN_3D.cxx
//             Moved here from SMESH_NETGEN_3D.cxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================
using namespace std;

#include "NETGENPlugin_NETGEN_3D.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include <TopExp.hxx>
#include <BRepTools.hxx>

#include "utilities.h"

#include <list>
#include <vector>
#include <map>

/*
  Netgen include files
*/

#include "nglib.h"

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
//   _shapeType = TopAbs_SOLID;
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
//   MESSAGE("_shapeType octal " << oct << _shapeType);
  _compatibleHypothesis.push_back("MaxElementVolume");

  _maxElementVolume = 0.;

  _hypMaxElementVolume = NULL;
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

bool NETGENPlugin_NETGEN_3D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::CheckHypothesis");

  _hypMaxElementVolume = NULL;

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (!nbHyp)
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work with no hypothesis
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

  // get a shell from aShape
  TopoDS_Shell aShell;
  if ( aShape.ShapeType() == TopAbs_SOLID ) {
    aShell = BRepTools::OuterShell( TopoDS::Solid( aShape ));
  }
  else {
    TopExp_Explorer exp(aShape,TopAbs_SHELL);
    if ( exp.More() )
      aShell = TopoDS::Shell(exp.Current());
  }

  if ( aShell.IsNull() || !aMesh.GetSubMesh( aShell )) {
    INFOS( "NETGENPlugin_NETGEN_3D::Compute(), bad shape");
    return false;
  }
  // -------------------------------------------------------------------
  // get triangles on aShell and make a map of nodes to Netgen node IDs
  // -------------------------------------------------------------------

  typedef map< const SMDS_MeshNode*, int> TNodeToIDMap;
  TNodeToIDMap nodeToNetgenID;
  list< const SMDS_MeshElement* > triangles;
  list< bool >                    isReversed; // orientation of triangles

  SMESH::Controls::Area areaControl;
  SMESH::Controls::TSequenceOfXYZ nodesCoords;

  for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next())
  {
    TopoDS_Shape aShapeFace = exp.Current();
    int faceID = meshDS->ShapeToIndex( aShapeFace );
    TopoDS_Shape aMeshedFace = meshDS->IndexToShape( faceID );
    const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements( faceID );
    if ( aSubMeshDSFace )
    {
      bool isRev = ( aShapeFace.Orientation() != aMeshedFace.Orientation() );
      SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
      while ( iteratorElem->more() ) // loop on elements on a face
      {
        // check element
        const SMDS_MeshElement* elem = iteratorElem->next();
        if ( !elem || elem->NbNodes() != 3 ) {
          INFOS( "NETGENPlugin_NETGEN_3D::Compute(), bad mesh");
          return false;
        }
        // keep a triangle
        triangles.push_back( elem );
        isReversed.push_back( isRev );
        // put elem nodes to nodeToNetgenID map
        SMDS_ElemIteratorPtr triangleNodesIt = elem->nodesIterator();
        while ( triangleNodesIt->more() ) {
	  const SMDS_MeshNode * node =
            static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
          nodeToNetgenID.insert( make_pair( node, 0 ));
        }
#ifdef _DEBUG_
        // check if a trainge is degenerated
        areaControl.GetPoints( elem, nodesCoords );
        double area = areaControl.GetValue( nodesCoords );
        if ( area <= DBL_MIN ) {
          MESSAGE( "Warning: Degenerated " << elem );
        }
#endif
      }
    }
  }
  // ---------------------------------
  // Feed the Netgen with surface mesh
  // ---------------------------------

  int Netgen_NbOfNodes = nodeToNetgenID.size();
  int Netgen_param2ndOrder = 0;
  double Netgen_paramFine = 1.;
  double Netgen_paramSize = _maxElementVolume;

  double Netgen_point[3];
  int Netgen_triangle[3];
  int Netgen_tetrahedron[4];

  Ng_Init();

  Ng_Mesh * Netgen_mesh = Ng_NewMesh();

  // set nodes and remember thier netgen IDs
  TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
  for ( int id = 0; n_id != nodeToNetgenID.end(); ++n_id )
  {
    const SMDS_MeshNode* node = n_id->first;
    Netgen_point [ 0 ] = node->X();
    Netgen_point [ 1 ] = node->Y();
    Netgen_point [ 2 ] = node->Z();
    n_id->second = ++id;
    Ng_AddPoint(Netgen_mesh, Netgen_point);
  }
  // set triangles
  list< const SMDS_MeshElement* >::iterator tria = triangles.begin();
  list< bool >::iterator                 reverse = isReversed.begin();
  for ( ; tria != triangles.end(); ++tria, ++reverse )
  {
    int i = 0;
    SMDS_ElemIteratorPtr triangleNodesIt = (*tria)->nodesIterator();
    while ( triangleNodesIt->more() ) {
      const SMDS_MeshNode * node =
        static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
      Netgen_triangle[ *reverse ? 2 - i : i ] = nodeToNetgenID[ node ];
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
    status = Ng_GenerateVolumeMesh(Netgen_mesh, &Netgen_param);
  } catch (...) {
    MESSAGE("An exception has been caught during the Volume Mesh Generation ...");
    status = NG_VOLUME_FAILURE;
  }

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);

  int Netgen_NbOfTetra = Ng_GetNE(Netgen_mesh);

  MESSAGE("End of Volume Mesh Generation. status=" << status <<
          ", nb new nodes: " << Netgen_NbOfNodesNew - Netgen_NbOfNodes <<
          ", nb tetra: " << Netgen_NbOfTetra);

  // -------------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Volume Elements
  // -------------------------------------------------------------------

  bool isOK = ( status == NG_OK && Netgen_NbOfTetra > 0 );
  if ( isOK )
  {
    // vector of nodes in which node index == netgen ID
    vector< const SMDS_MeshNode* > nodeVec ( Netgen_NbOfNodesNew + 1 );
    // insert old nodes into nodeVec
    for ( n_id = nodeToNetgenID.begin(); n_id != nodeToNetgenID.end(); ++n_id )
      nodeVec[ n_id->second ] = n_id->first;
    // create and insert new nodes into nodeVec
    int nodeIndex = Netgen_NbOfNodes + 1;
    for ( ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint( Netgen_mesh, nodeIndex, Netgen_point );
      SMDS_MeshNode * node = meshDS->AddNode(Netgen_point[0],
                                             Netgen_point[1],
                                             Netgen_point[2]);
      meshDS->SetNodeInVolume(node, aShell);
      nodeVec[nodeIndex] = node;
    }

    // create tetrahedrons
    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      Ng_GetVolumeElement(Netgen_mesh, elemIndex, Netgen_tetrahedron);
      SMDS_MeshVolume * elt = meshDS->AddVolume (nodeVec[ Netgen_tetrahedron[0] ],
                                                 nodeVec[ Netgen_tetrahedron[1] ],
                                                 nodeVec[ Netgen_tetrahedron[2] ],
                                                 nodeVec[ Netgen_tetrahedron[3] ]);
      meshDS->SetMeshElementOnShape(elt, aShell);
    }
  }

  Ng_DeleteMesh(Netgen_mesh);
  Ng_Exit();

  return isOK;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & NETGENPlugin_NETGEN_3D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & NETGENPlugin_NETGEN_3D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, NETGENPlugin_NETGEN_3D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, NETGENPlugin_NETGEN_3D & hyp)
{
  return hyp.LoadFrom( load );
}
