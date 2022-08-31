// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : netgen_mesher.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "netgen_mesher.hxx"

#include "DriverStep.hxx"
#include "DriverMesh.hxx"
#include "netgen_param.hxx"

#include <fstream>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

// SMESH include
#include <SMESH_Mesh.hxx>
#include <SMESH_Gen.hxx>
#include <SMESH_Algo.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_ControlsDef.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_ComputeError.hxx>
#include <SMESH_MesherHelper.hxx>
#include <StdMeshers_MaxElementVolume.hxx>
#include <StdMeshers_QuadToTriaAdaptor.hxx>
#include <StdMeshers_ViscousLayers.hxx>

// NETGENPlugin
// #include <NETGENPlugin_Mesher.hxx>
// #include <NETGENPlugin_Hypothesis.hxx>
#include "NETGENPlugin_Mesher.hxx"
#include "NETGENPlugin_Hypothesis.hxx"


// OCC include
#include <TopoDS.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
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

  NETGENPLUGIN_DLL_HEADER
  extern volatile multithreadt multithread;

  NETGENPLUGIN_DLL_HEADER
  extern bool merge_solids;
}
using namespace nglib;

int error(int error_type, std::string msg){
 std::cerr << msg << std::endl;
  return error_type;
};

int error(const SMESH_Comment& comment){
  return error(1, "SMESH_Comment error: "+comment);
};

/**
 * @brief Set the netgen parameters
 *
 * @param aParams Internal structure of parameters
 * @param mparams Netgen strcuture of parameters
 */
void set_netgen_parameters(netgen_params& aParams){

  // Default parameters
#ifdef NETGEN_V6

  netgen::mparam.nthreads = std::thread::hardware_concurrency();

  if ( getenv( "SALOME_NETGEN_DISABLE_MULTITHREADING" ))
  {
    netgen::mparam.nthreads = 1;
    netgen::mparam.parallel_meshing = false;
  }

#endif

  // Initialize global NETGEN parameters:
  netgen::mparam.maxh               = aParams.maxh;
  netgen::mparam.minh               = aParams.minh;
  netgen::mparam.segmentsperedge    = aParams.segmentsperedge;
  netgen::mparam.grading            = aParams.grading;
  netgen::mparam.curvaturesafety    = aParams.curvaturesafety;
  netgen::mparam.secondorder        = aParams.secondorder;
  netgen::mparam.quad               = aParams.quad;
  netgen::mparam.uselocalh          = aParams.uselocalh;
  netgen::merge_solids       = aParams.merge_solids;
  netgen::mparam.optsteps2d         = aParams.optsteps2d;
  netgen::mparam.optsteps3d         = aParams.optsteps3d;
  netgen::mparam.elsizeweight       = aParams.elsizeweight;
  netgen::mparam.opterrpow          = aParams.opterrpow;
  netgen::mparam.delaunay           = aParams.delaunay;
  netgen::mparam.checkoverlap       = aParams.checkoverlap;
  netgen::mparam.checkchartboundary = aParams.checkchartboundary;
#ifdef NETGEN_V6
  // std::string
  netgen::mparam.meshsizefilename = aParams.meshsizefilename;
  netgen::mparam.closeedgefac = aParams.closeedgefac;

#else
  // const char*
  netgen::mparam.meshsizefilename= aParams.meshsizefilename ? 0 : aParams.meshsizefilename.c_str();
#endif
}

/**
 * @brief compute mesh with netgen
 *
 * @param input_mesh_file Input Mesh file
 * @param shape_file Shape file
 * @param hypo_file Parameter file
 * @param new_element_file Binary file containing new nodes and new element info
 * @param output_mesh If true will export mesh into output_mesh_file
 * @param output_mesh_file Output Mesh file
 *
 * @return error code
 */
int netgen3d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orientation_file,
             const std::string new_element_file,
             bool output_mesh,
             const std::string output_mesh_file){

  // Importing mesh
  SMESH_Gen gen;

  SMESH_Mesh *myMesh = gen.CreateMesh(false);
  //TODO: To define
  std::string mesh_name = "Maillage_1";

  import_mesh(input_mesh_file, *myMesh, mesh_name);

  // Importing shape
  TopoDS_Shape myShape;
  import_shape(shape_file, myShape);

  // Importing hypothesis
  //TODO: make it
  netgen_params myParams;

  import_netgen_params(hypo_file, myParams);

  std::cout << "Meshing with netgen3d" << std::endl;
  int ret = netgen3d(myShape, *myMesh, myParams,
                     new_element_file, element_orientation_file,
                     output_mesh);

  if(!ret){
    std::cout << "Meshing failed" << std::endl;
    return ret;
  }

  if(output_mesh)
    export_mesh(output_mesh_file, *myMesh, mesh_name);

  return ret;
}

/**
 * @brief Compute aShape within aMesh using netgen
 *
 * @param aShape the shape
 * @param aMesh the mesh
 * @param aParams the netgen parameters
 * @param new_element_file file containing data on the new point/tetra added by netgen
 *
 * @return error code
 */
int netgen3d(TopoDS_Shape &aShape, SMESH_Mesh& aMesh, netgen_params& aParams,
             std::string new_element_file, std::string element_orientation_file,
             bool output_mesh){

  netgen::multithread.terminate = 0;
  netgen::multithread.task = "Volume meshing";

  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();

  SMESH_MesherHelper helper(aMesh);
  aParams._quadraticMesh = helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );

  int Netgen_NbOfNodes = 0;
  double Netgen_point[3];
  int Netgen_triangle[3];

  NETGENPlugin_NetgenLibWrapper ngLib;
  Ng_Mesh * Netgen_mesh = (Ng_Mesh*)ngLib._ngMesh;

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

    // Get list of elements + their orientation from element_orientation file
    std::ifstream df(element_orientation_file, ios::binary|ios::in);
    int nbElement;
    bool orient;

    // Warning of the use of vtkIdType (I had issue when run_mesher was compiled with internal vtk) and salome not
    // Sizeof was the same but how he othered the type was different
    // Maybe using another type (uint64_t) instead would be better
    vtkIdType id;
    std::map<vtkIdType, bool> elemOrientation;
    df.read((char*)&nbElement, sizeof(int));

    for(int ielem=0;ielem<nbElement;++ielem){
      df.read((char*) &id, sizeof(vtkIdType));
      df.read((char*) &orient, sizeof(bool));
      elemOrientation[id] = orient;
    }
    df.close();

    // Adding elements from Mesh
    SMDS_ElemIteratorPtr iteratorElem = meshDS->elementsIterator(SMDSAbs_Face);
    int nbedge = meshDS->NbEdges();
    int nbface = meshDS->NbFaces();
    bool isRev;
    bool isInternalFace = false;

    bool isIn;

    while ( iteratorElem->more() ) // loop on elements on a geom face
      {
        // check mesh face
        const SMDS_MeshElement* elem = iteratorElem->next();
        int tmp = elem->GetShapeID();
        if ( !elem )
          return error( COMPERR_BAD_INPUT_MESH, "Null element encounters");
        if ( elem->NbCornerNodes() != 3 )
          return error( COMPERR_BAD_INPUT_MESH, "Not triangle element encounters");

        // Keeping only element that are in the element orientation file
        isIn = elemOrientation.count(elem->GetID())==1;

        if(!isIn)
          continue;


        // Get orientation
        // Netgen requires that all the triangle point outside
        isRev = elemOrientation[elem->GetID()];

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

        // TODO: Handle that case (quadrangle 2D) (isInternal is set to false)
        if ( isInternalFace && !proxyMesh->IsTemporary( elem ))
        {
          swap( Netgen_triangle[1], Netgen_triangle[2] );
          Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
        }
      }

    // insert old nodes into nodeVec
    nodeVec.resize( nodeToNetgenID.size() + 1, 0 );
    TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
    for ( ; n_id != nodeToNetgenID.end(); ++n_id )
      nodeVec[ n_id->second ] = n_id->first;
    nodeToNetgenID.clear();

    // TODO: Handle internal vertex
    //if ( internals.hasInternalVertexInSolid() )
    //{
    //  netgen::OCCGeometry occgeo;
    //  NETGENPlugin_Mesher::AddIntVerticesInSolids( occgeo,
    //                                               (netgen::Mesh&) *Netgen_mesh,
    //                                               nodeVec,
    //                                               internals);
    //}
  }

  // -------------------------
  // Generate the volume mesh
  // -------------------------
  netgen::multithread.terminate = 0;

  netgen::Mesh* ngMesh = ngLib._ngMesh;
  Netgen_mesh = ngLib.ngMesh();
  Netgen_NbOfNodes = Ng_GetNP( Netgen_mesh );


  int startWith = netgen::MESHCONST_MESHVOLUME;
  int endWith   = netgen::MESHCONST_OPTVOLUME;
  int err = 1;

  NETGENPlugin_Mesher aMesher( &aMesh, helper.GetSubShape(), /*isVolume=*/true );
  netgen::OCCGeometry occgeo;
  set_netgen_parameters(aParams);

  if ( aParams.has_netgen_param )
  {
    if ( aParams.has_local_size)
    {
      if ( ! &ngMesh->LocalHFunction() )
      {
        netgen::Point3d pmin, pmax;
        ngMesh->GetBox( pmin, pmax, 0 );
        ngMesh->SetLocalH( pmin, pmax, aParams.grading );
      }
      aMesher.SetLocalSize( occgeo, *ngMesh );

      try {
        ngMesh->LoadLocalMeshSize( netgen::mparam.meshsizefilename );
      } catch (netgen::NgException & ex) {
        return error( COMPERR_BAD_PARMETERS, ex.What() );
      }
    }
    if ( !aParams.optimize )
      endWith = netgen::MESHCONST_MESHVOLUME;
  }
  else if ( aParams.has_maxelementvolume_hyp )
  {
    netgen::mparam.maxh = pow( 72, 1/6. ) * pow( aParams.maxElementVolume, 1/3. );
    // limitVolumeSize( ngMesh, netgen::mparam.maxh ); // result is unpredictable
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

  if ( !aParams.has_netgen_param && aMesh.HasShapeToMesh() )
  {
    netgen::mparam.minh = aMesher.GetDefaultMinSize( helper.GetSubShape(), netgen::mparam.maxh );
  }

  try
  {
    OCC_CATCH_SIGNALS;

    ngLib.CalcLocalH(ngMesh);
    err = ngLib.GenerateMesh(occgeo, startWith, endWith, ngMesh, netgen::mparam);

    if(netgen::multithread.terminate)
      return false;
    if ( err )
      return error(SMESH_Comment("Error in netgen::OCCGenerateMesh() at ") << netgen::multithread.task);
  }
  catch (Standard_Failure& ex)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    str << " at " << netgen::multithread.task
        << ": " << ex.DynamicType()->Name();
    if ( ex.GetMessageString() && strlen( ex.GetMessageString() ))
      str << ": " << ex.GetMessageString();
    return error(str);
  }
  catch (netgen::NgException& exc)
  {
    SMESH_Comment str("NgException");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    str << ": " << exc.What();
    return error(str);
  }
  catch (...)
  {
    SMESH_Comment str("Exception in  netgen::OCCGenerateMesh()");
    if ( strlen( netgen::multithread.task ) > 0 )
      str << " at " << netgen::multithread.task;
    return error(str);
  }

  int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);
  int Netgen_NbOfTetra    = Ng_GetNE(Netgen_mesh);

  // -------------------------------------------------------------------
  // Feed back the SMESHDS with the generated Nodes and Volume Elements
  // -------------------------------------------------------------------

  if ( err )
  {
    SMESH_ComputeErrorPtr ce = NETGENPlugin_Mesher::ReadErrors(nodeVec);
    if ( ce && ce->HasBadElems() )
      return error( ce );
  }

  bool isOK = ( /*status == NG_OK &&*/ Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK )
  {
    std::ofstream df(new_element_file, ios::out|ios::binary);

    double Netgen_point[3];
    int    Netgen_tetrahedron[4];

    // Writing nodevec (correspondance netgen numbering mesh numbering)
    // Number of nodes
    df.write((char*) &Netgen_NbOfNodes, sizeof(int));
    df.write((char*) &Netgen_NbOfNodesNew, sizeof(int));
    for (int nodeIndex = 1 ; nodeIndex <= Netgen_NbOfNodes; ++nodeIndex )
    {
      //Id of the point
      int id = nodeVec.at(nodeIndex)->GetID();
      df.write((char*) &id, sizeof(int));
    }

    // Writing info on new points
    for (int nodeIndex = Netgen_NbOfNodes +1 ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint(Netgen_mesh, nodeIndex, Netgen_point );
      // Coordinates of the point
      df.write((char *) &Netgen_point, sizeof(double)*3);
    }

    // create tetrahedrons
    df.write((char*) &Netgen_NbOfTetra, sizeof(int));
    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      Ng_GetVolumeElement(Netgen_mesh, elemIndex, Netgen_tetrahedron);
      df.write((char*) &Netgen_tetrahedron, sizeof(int)*4);
    }
    df.close();
  }

  // Adding new files in aMesh as well
  if ( output_mesh )
  {
    double Netgen_point[3];
    int    Netgen_tetrahedron[4];

    // create and insert new nodes into nodeVec
    nodeVec.resize( Netgen_NbOfNodesNew + 1, 0 );
    int nodeIndex = Netgen_NbOfNodes + 1;
    for ( ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      Ng_GetPoint( Netgen_mesh, nodeIndex, Netgen_point );
      nodeVec.at(nodeIndex) = helper.AddNode(Netgen_point[0],
                                             Netgen_point[1],
                                             Netgen_point[2]);
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

  return !err;
}