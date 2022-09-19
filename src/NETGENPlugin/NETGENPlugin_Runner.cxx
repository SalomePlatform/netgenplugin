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

//  File   : NETGENPlugin_Runner.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "NETGENPlugin_Runner.hxx"


#include "NETGENPlugin_DriverParam.hxx"

#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <chrono>

// SMESH include
#include <SMESH_Mesh.hxx>
#include <SMESH_subMesh.hxx>
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
#include <StdMeshers_ViscousLayers2D.hxx>
#include <SMESH_DriverShape.hxx>
#include <SMESH_DriverMesh.hxx>


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
#include <meshing.hpp>

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

  NETGENPLUGIN_DLL_HEADER
  extern bool merge_solids;

#ifdef NETGEN_V5
  extern void OCCSetLocalMeshSize(OCCGeometry & geom, Mesh & mesh);
#endif
}
using namespace nglib;

int error(int error_type, std::string msg)
{
 std::cerr << msg << std::endl;
  return error_type;
};

int error(const SMESH_Comment& comment)
{
  return error(1, "SMESH_Comment error: "+comment);
};

/**
 * @brief Set the netgen parameters
 *
 * @param aParams Internal structure of parameters
 * @param mparams Netgen strcuture of parameters
 */
void set_netgen_parameters(netgen_params& aParams)
{

  // Default parameters
#ifdef NETGEN_V6

  //netgen::mparam.nthreads = std::thread::hardware_concurrency();
  netgen::mparam.nthreads = aParams.nbThreads;
  netgen::mparam.parallel_meshing = aParams.nbThreads > 1;


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
  netgen::mparam.meshsizefilename= aParams.meshsizefilename.empty() ? 0 : aParams.meshsizefilename.c_str();
#endif
}

/**
 * @brief compute mesh with netgen3d
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
             const std::string output_mesh_file,
             int nbThreads)
{
  auto time0 = std::chrono::high_resolution_clock::now();
  // Importing mesh
  SMESH_Gen gen;

  std::unique_ptr<SMESH_Mesh> myMesh(gen.CreateMesh(false));
  //TODO: To define
  std::string mesh_name = "Maillage_1";

  importMesh(input_mesh_file, *myMesh, mesh_name);
  auto time1 = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time1-time0);
  std::cout << "Time for importMesh: " << elapsed.count() * 1e-9 << std::endl;

  // Importing shape
  TopoDS_Shape myShape;
  importShape(shape_file, myShape);
  auto time2 = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1);
  std::cout << "Time for importShape: " << elapsed.count() * 1e-9 << std::endl;

  // Importing hypothesis
  netgen_params myParams;

  importNetgenParams(hypo_file, myParams, &gen);
  auto time3 = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2);
  std::cout << "Time for import_netgen_param: " << elapsed.count() * 1e-9 << std::endl;
  // Setting number of threads for netgen
  myParams.nbThreads = nbThreads;

  std::cout << "Meshing with netgen3d" << std::endl;
  int ret = netgen3dInternal(myShape, *myMesh, myParams,
                              new_element_file, element_orientation_file,
                              !output_mesh_file.empty());


  if(!ret){
    std::cout << "Meshing failed" << std::endl;
    return ret;
  }

  if(!output_mesh_file.empty()){
    auto time4 = std::chrono::high_resolution_clock::now();
    exportMesh(output_mesh_file, *myMesh, mesh_name);
    auto time5 = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time5-time4);
    std::cout << "Time for exportMesh: " << elapsed.count() * 1e-9 << std::endl;
  }

  return ret;
}

/**
 * @brief Compute aShape within aMesh using netgen3d
 *
 * @param aShape the shape
 * @param aMesh the mesh
 * @param aParams the netgen parameters
 * @param new_element_file file containing data on the new point/tetra added by netgen
 * @param element_orientation_file file containing data on the orientation of each element to add to netgen
 * @param output_mesh if true add element created by netgen into aMesh
 *
 * @return error code
 */
int netgen3dInternal(TopoDS_Shape &aShape, SMESH_Mesh& aMesh, netgen_params& aParams,
                      std::string new_element_file, std::string element_orientation_file,
                      bool output_mesh)
{

  auto time0 = std::chrono::high_resolution_clock::now();

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
    std::map<vtkIdType, bool> elemOrientation;
    {
      // Setting all element orientation to false if there no element orientation file
      if(element_orientation_file.empty()){
        SMDS_ElemIteratorPtr iteratorElem = meshDS->elementsIterator(SMDSAbs_Face);
        while ( iteratorElem->more() ) // loop on elements on a geom face
          {
            // check mesh face
            const SMDS_MeshElement* elem = iteratorElem->next();
            elemOrientation[elem->GetID()] = false;
          }
      } else {
        std::ifstream df(element_orientation_file, ios::binary|ios::in);
        int nbElement;
        bool orient;

        // Warning of the use of vtkIdType (I had issue when run_mesher was compiled with internal vtk) and salome not
        // Sizeof was the same but how he othered the type was different
        // Maybe using another type (uint64_t) instead would be better
        vtkIdType id;
        df.read((char*)&nbElement, sizeof(int));

        for(int ielem=0;ielem<nbElement;++ielem){
          df.read((char*) &id, sizeof(vtkIdType));
          df.read((char*) &orient, sizeof(bool));
          elemOrientation[id] = orient;
        }
      }
    }

    // Adding elements from Mesh
    SMDS_ElemIteratorPtr iteratorElem = meshDS->elementsIterator(SMDSAbs_Face);
    bool isRev;
    bool isInternalFace = false;

    bool isIn;

    while ( iteratorElem->more() ) // loop on elements on a geom face
      {
        // check mesh face
        const SMDS_MeshElement* elem = iteratorElem->next();
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
  auto time1 = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time1-time0);
  std::cout << "Time for fill_in_ngmesh: " << elapsed.count() * 1e-9 << std::endl;

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
    err = ngLib.GenerateMesh(occgeo, startWith, endWith, ngMesh);

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
  auto time2 = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1);
  std::cout << "Time for netgen_compute: " << elapsed.count() * 1e-9 << std::endl;

  bool isOK = ( /*status == NG_OK &&*/ Netgen_NbOfTetra > 0 );// get whatever built
  if ( isOK && !new_element_file.empty() )
  {
    std::ofstream df(new_element_file, ios::out|ios::binary);

    double Netgen_point[3];
    int    Netgen_tetrahedron[4];

    // Writing nodevec (correspondence netgen numbering mesh numbering)
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
  }
  auto time3 = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2);
  std::cout << "Time for write_new_elem: " << elapsed.count() * 1e-9 << std::endl;


  // Adding new elements in aMesh as well
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
    auto time4 = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time4-time3);
    std::cout << "Time for add_element_to_smesh: " << elapsed.count() * 1e-9 << std::endl;

  }

  return !err;
}

/**
 * @brief compute mesh with netgen2d
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
int netgen2d(const std::string input_mesh_file,
             const std::string shape_file,
             const std::string hypo_file,
             const std::string element_orientation_file,
             const std::string new_element_file,
             const std::string output_mesh_file)
{

  // Importing mesh
  SMESH_Gen gen;

  std::unique_ptr<SMESH_Mesh> myMesh(gen.CreateMesh(false));

  //TODO: To define
  std::string mesh_name = "Maillage_1";

  importMesh(input_mesh_file, *myMesh, mesh_name);

  // Importing shape
  TopoDS_Shape myShape;
  importShape(shape_file, myShape);

  // Importing hypothesis
  netgen_params myParams;

  importNetgenParams(hypo_file, myParams, &gen);

  std::cout << "Meshing with netgen3d" << std::endl;
  int ret = netgen2dInternal(myShape, *myMesh, myParams,
                              new_element_file, element_orientation_file,
                              !output_mesh_file.empty());

  if(!ret){
    std::cout << "Meshing failed" << std::endl;
    return ret;
  }

  if(!output_mesh_file.empty())
    exportMesh(output_mesh_file, *myMesh, mesh_name);

  return ret;
}


// TODO: Not working properly
/**
 * @brief Compute aShape within aMesh using netgen2d
 *
 * @param aShape the shape
 * @param aMesh the mesh
 * @param aParams the netgen parameters
 * @param new_element_file file containing data on the new point/tetra added by netgen
 *
 * @return error code
 */
int netgen2dInternal(TopoDS_Shape &aShape, SMESH_Mesh& aMesh, netgen_params& aParams,
                      std::string new_element_file, std::string element_orientation_file,
                      bool output_mesh)
{
  netgen::multithread.terminate = 0;
  netgen::multithread.task = "Surface meshing";

  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  SMESH_MesherHelper helper(aMesh);
  helper.SetElementsOnShape( true );

  NETGENPlugin_NetgenLibWrapper ngLib;
  ngLib._isComputeOk = false;

  netgen::Mesh   ngMeshNoLocSize;
  netgen::Mesh * ngMeshes[2] = { (netgen::Mesh*) ngLib._ngMesh,  & ngMeshNoLocSize };
  netgen::OCCGeometry occgeoComm;

  std::map<vtkIdType, bool> elemOrientation;

  typedef map< const SMDS_MeshNode*, int, TIDCompare > TNodeToIDMap;
  typedef TNodeToIDMap::value_type                     TN2ID;
  const int invalid_ID = -1;
  int Netgen_NbOfNodes=0;
  double Netgen_point[3];
  int Netgen_segment[2];
  int Netgen_triangle[3];

  // min / max sizes are set as follows:
  // if ( _hypParameters )
  //    min and max are defined by the user
  // else if ( aParams.has_LengthFromEdges_hyp )
  //    min = aMesher.GetDefaultMinSize()
  //    max = average segment len of a FACE
  // else if ( _hypMaxElementArea )
  //    min = aMesher.GetDefaultMinSize()
  //    max = f( _hypMaxElementArea )
  // else
  //    min = aMesher.GetDefaultMinSize()
  //    max = max segment len of a FACE
  NETGENPlugin_Mesher aMesher( &aMesh, aShape, /*isVolume=*/false);
  set_netgen_parameters( aParams );
  const bool toOptimize = aParams.optimize;
  if ( aParams.has_maxelementvolume_hyp )
  {
    netgen::mparam.maxh = sqrt( 2. * aParams.maxElementVolume / sqrt(3.0) );
  }
  netgen::mparam.quad = aParams.quad;

  // local size is common for all FACEs in aShape?
  const bool isCommonLocalSize = ( !aParams.has_LengthFromEdges_hyp && !aParams.has_maxelementvolume_hyp && netgen::mparam.uselocalh );
  const bool isDefaultHyp = ( !aParams.has_LengthFromEdges_hyp && !aParams.has_maxelementvolume_hyp && !aParams.has_netgen_param );


  if ( isCommonLocalSize ) // compute common local size in ngMeshes[0]
  {
    //list< SMESH_subMesh* > meshedSM[4]; --> all sub-shapes are added to occgeoComm
    aMesher.PrepareOCCgeometry( occgeoComm, aShape, aMesh );//, meshedSM );

    // local size set at MESHCONST_ANALYSE step depends on
    // minh, face_maxh, grading and curvaturesafety; find minh if not set by the user
    if ( !aParams.has_netgen_param || netgen::mparam.minh < DBL_MIN )
    {
      if ( !aParams.has_netgen_param )
        netgen::mparam.maxh = occgeoComm.GetBoundingBox().Diam() / 3.;
      netgen::mparam.minh = aMesher.GetDefaultMinSize( aShape, netgen::mparam.maxh );
    }
    // set local size depending on curvature and NOT closeness of EDGEs
#ifdef NETGEN_V6
    const double factor = 2; //netgen::occparam.resthcloseedgefac;
#else
    const double factor = netgen::occparam.resthcloseedgefac;
    netgen::occparam.resthcloseedgeenable = false;
    netgen::occparam.resthcloseedgefac = 1.0 + netgen::mparam.grading;
#endif
    occgeoComm.face_maxh = netgen::mparam.maxh;
#ifdef NETGEN_V6
    netgen::OCCParameters occparam;
    netgen::OCCSetLocalMeshSize( occgeoComm, *ngMeshes[0], netgen::mparam, occparam );
#else
    netgen::OCCSetLocalMeshSize( occgeoComm, *ngMeshes[0] );
#endif
    occgeoComm.emap.Clear();
    occgeoComm.vmap.Clear();

    // Reading list of element to integrate into netgen mesh
    {
      std::ifstream df(element_orientation_file, ios::in|ios::binary);
      int nbElement;
      vtkIdType id;
      bool orient;
      df.read((char*)&nbElement, sizeof(int));

      for(int ielem=0;ielem<nbElement;++ielem){
        df.read((char*) &id, sizeof(vtkIdType));
        df.read((char*) &orient, sizeof(bool));
        elemOrientation[id] = orient;
      }
    }

    bool isIn;
    // set local size according to size of existing segments
    SMDS_ElemIteratorPtr iteratorElem = meshDS->elementsIterator(SMDSAbs_Edge);
    while ( iteratorElem->more() ) // loop on elements on a geom face
    {
      const SMDS_MeshElement* seg = iteratorElem->next();
      // Keeping only element that are in the element orientation file
      isIn = elemOrientation.count(seg->GetID())==1;

      if(!isIn)
        continue;

      SMESH_TNodeXYZ n1 = seg->GetNode(0);
      SMESH_TNodeXYZ n2 = seg->GetNode(1);
      gp_XYZ p = 0.5 * ( n1 + n2 );
      netgen::Point3d pi(p.X(), p.Y(), p.Z());
      ngMeshes[0]->RestrictLocalH( pi, factor * ( n1 - n2 ).Modulus() );
    }

    // set local size defined on shapes
    aMesher.SetLocalSize( occgeoComm, *ngMeshes[0] );
    aMesher.SetLocalSizeForChordalError( occgeoComm, *ngMeshes[0] );
    try {
      ngMeshes[0]->LoadLocalMeshSize( netgen::mparam.meshsizefilename );
    } catch (netgen::NgException & ex) {
      return error( COMPERR_BAD_PARMETERS, ex.What() );
    }
  }
  netgen::mparam.uselocalh = toOptimize; // restore as it is used at surface optimization
  // ==================
  // Loop on all FACEs
  // ==================

  vector< const SMDS_MeshNode* > nodeVec;

    // prepare occgeom
    netgen::OCCGeometry occgeom;
    occgeom.shape = aShape;
    occgeom.fmap.Add( aShape );
    occgeom.CalcBoundingBox();
    occgeom.facemeshstatus.SetSize(1);
    occgeom.facemeshstatus = 0;
    occgeom.face_maxh_modified.SetSize(1);
    occgeom.face_maxh_modified = 0;
    occgeom.face_maxh.SetSize(1);
    occgeom.face_maxh = netgen::mparam.maxh;

    // -------------------------
    // Fill netgen mesh
    // -------------------------
    // maps nodes to ng ID


    // MESHCONST_ANALYSE step may lead to a failure, so we make an attempt
    // w/o MESHCONST_ANALYSE at the second loop
    int err = 0;
    enum { LOC_SIZE, NO_LOC_SIZE };
    int iLoop = isCommonLocalSize ? 0 : 1;
    int faceID = occgeom.fmap.FindIndex(aShape);
    int solidID = 0;
    for ( ; iLoop < 2; iLoop++ )
    {
      //bool isMESHCONST_ANALYSE = false;
      //TODO: check how to replace that
      //InitComputeError();

      netgen::Mesh * ngMesh = ngMeshes[ iLoop ];
      ngMesh->DeleteMesh();

      if ( iLoop == NO_LOC_SIZE )
      {
        ngMesh->SetGlobalH ( netgen::mparam.maxh );
        ngMesh->SetMinimalH( netgen::mparam.minh );
        netgen::Box<3> bb = occgeom.GetBoundingBox();
        bb.Increase (bb.Diam()/10);
        ngMesh->SetLocalH (bb.PMin(), bb.PMax(), netgen::mparam.grading);
        aMesher.SetLocalSize( occgeom, *ngMesh );
        aMesher.SetLocalSizeForChordalError( occgeoComm, *ngMesh );
        try {
          ngMesh->LoadLocalMeshSize( netgen::mparam.meshsizefilename );
        } catch (netgen::NgException & ex) {
          return error( COMPERR_BAD_PARMETERS, ex.What() );
        }
      }

      TNodeToIDMap nodeToNetgenID;

      nodeVec.clear();
      ngMesh->AddFaceDescriptor( netgen::FaceDescriptor( faceID, solidID, solidID, 0 ));
          // set local size according to size of existing segments
      SMDS_ElemIteratorPtr iteratorElem = meshDS->elementsIterator(SMDSAbs_Edge);
      while ( iteratorElem->more() ) // loop on elements on a geom face
      {
        const SMDS_MeshElement* elem = iteratorElem->next();
        // Keeping only element that are in the element orientation file
        bool isIn = elemOrientation.count(elem->GetID())==1;

        if(!isIn)
          continue;

        bool isRev = elemOrientation[elem->GetID()];
        std::cerr << isRev;



        for ( int iN = 0; iN < 2; ++iN )
        {
          const SMDS_MeshNode* node = elem->GetNode( iN );
          const int shapeID = node->getshapeId();
          int& ngID = nodeToNetgenID.insert(TN2ID( node, invalid_ID )).first->second;
          if ( ngID == invalid_ID )
          {
            ngID = ++Netgen_NbOfNodes;
            Netgen_point [ 0 ] = node->X();
            Netgen_point [ 1 ] = node->Y();
            Netgen_point [ 2 ] = node->Z();
            netgen::MeshPoint mp( netgen::Point<3> (node->X(), node->Y(), node->Z()) );
            ngMesh->AddPoint ( mp, 1, netgen::EDGEPOINT );
          }
          Netgen_segment[ isRev ? 1-iN : iN ] = ngID;
        }
        // add segment

        netgen::Segment seg;
        seg[0] = Netgen_segment[0];
        seg[1] = Netgen_segment[1];
        seg.edgenr = ngMesh->GetNSeg() +1;
        seg.si = faceID;

        ngMesh->AddSegment(seg);
      }
      int nbNodes2 = ngMesh->GetNP();
      int nseg = ngMesh->GetNSeg();

      // insert old nodes into nodeVec
      nodeVec.resize( nodeToNetgenID.size() + 1, 0 );
      TNodeToIDMap::iterator n_id = nodeToNetgenID.begin();
      for ( ; n_id != nodeToNetgenID.end(); ++n_id )
        nodeVec[ n_id->second ] = n_id->first;
      nodeToNetgenID.clear();


      //if ( !isCommonLocalSize )
      //limitSize( ngMesh, mparam.maxh * 0.8);

      // -------------------------
      // Generate surface mesh
      // -------------------------

      const int startWith = netgen::MESHCONST_MESHSURFACE;
      const int endWith   = toOptimize ? netgen::MESHCONST_OPTSURFACE : netgen::MESHCONST_MESHSURFACE;

      SMESH_Comment str;
      try {
        OCC_CATCH_SIGNALS;
        err = ngLib.GenerateMesh(occgeom, startWith, endWith, ngMesh);
        if ( netgen::multithread.terminate )
          return false;
        if ( err )
          str << "Error in netgen::OCCGenerateMesh() at " << netgen::multithread.task;
      }
      catch (Standard_Failure& ex)
      {
        err = 1;
        str << "Exception in  netgen::OCCGenerateMesh()"
            << " at " << netgen::multithread.task
            << ": " << ex.DynamicType()->Name();
        if ( ex.GetMessageString() && strlen( ex.GetMessageString() ))
          str << ": " << ex.GetMessageString();
      }
      catch (...) {
        err = 1;
        str << "Exception in  netgen::OCCGenerateMesh()"
            << " at " << netgen::multithread.task;
      }
      if ( err )
      {
        if ( iLoop == LOC_SIZE )
        {
          std::cout << "Need second run" << std::endl;
          /*netgen::mparam.minh = netgen::mparam.maxh;
          netgen::mparam.maxh = 0;
          for ( size_t iW = 0; iW < wires.size(); ++iW )
          {
            StdMeshers_FaceSidePtr wire = wires[ iW ];
            const vector<UVPtStruct>& uvPtVec = wire->GetUVPtStruct();
            for ( size_t iP = 1; iP < uvPtVec.size(); ++iP )
            {
              SMESH_TNodeXYZ   p( uvPtVec[ iP ].node );
              netgen::Point3d np( p.X(),p.Y(),p.Z());
              double segLen = p.Distance( uvPtVec[ iP-1 ].node );
              double   size = ngMesh->GetH( np );
              netgen::mparam.minh = Min( netgen::mparam.minh, size );
              netgen::mparam.maxh = Max( netgen::mparam.maxh, segLen );
            }
          }
          //cerr << "min " << mparam.minh << " max " << mparam.maxh << endl;
          netgen::mparam.minh *= 0.9;
          netgen::mparam.maxh *= 1.1;
          */
          continue;
        }
        else
        {
          //faceErr.reset( new SMESH_ComputeError( COMPERR_ALGO_FAILED, str ));
        }
      }

      // ----------------------------------------------------
      // Fill the SMESHDS with the generated nodes and faces
      // ----------------------------------------------------

      if(output_mesh)
      {
        int nbNodes = ngMesh->GetNP();
        int nbFaces = ngMesh->GetNSE();
        std::cout << nbFaces << " " << nbNodes << std::endl;

        int nbInputNodes = (int) nodeVec.size()-1;
        nodeVec.resize( nbNodes+1, 0 );

        // add nodes
        for ( int ngID = nbInputNodes + 1; ngID <= nbNodes; ++ngID )
        {
          const netgen::MeshPoint& ngPoint = ngMesh->Point( ngID );
          SMDS_MeshNode * node = meshDS->AddNode(ngPoint(0), ngPoint(1), ngPoint(2));
          nodeVec[ ngID ] = node;
        }

        // create faces
        int i,j;
        for ( i = 1; i <= nbFaces ; ++i )
        {
          Ng_GetVolumeElement(ngLib.ngMesh(), i, Netgen_triangle);

          helper.AddFace (nodeVec.at( Netgen_triangle[0] ),
                          nodeVec.at( Netgen_triangle[1] ),
                          nodeVec.at( Netgen_triangle[2] ));

        }
      } // output_mesh

      break;
    } // two attempts
  //} // loop on FACEs

  return true;

}
