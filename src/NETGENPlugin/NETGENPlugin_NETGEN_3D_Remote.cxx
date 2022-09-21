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

//=============================================================================
// File      : NETGENPlugin_NETGEN_3D_Remote.cxx
// Created   : lundi 19 Septembre 2022
// Author    : Yoann AUDOUIN (CEA)
// Project   : SALOME
//=============================================================================
//
//
#include "NETGENPlugin_NETGEN_3D_Remote.hxx"

#include "NETGENPlugin_NETGEN_3D.hxx"

#include "NETGENPlugin_DriverParam.hxx"
#include "NETGENPlugin_Hypothesis.hxx"

#include <SMESH_Gen.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_DriverShape.hxx>
#include <SMESH_DriverMesh.hxx>
#include <SMESHDS_Mesh.hxx>

#include <QString>
#include <QProcess>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_NETGEN_3D_Remote::NETGENPlugin_NETGEN_3D_Remote(int hypId, SMESH_Gen * gen)
  : NETGENPlugin_NETGEN_3D(hypId, gen)
{
  _name = "NETGEN_3D_Remote";
}

//=============================================================================
/*!
 *
 */
//=============================================================================

NETGENPlugin_NETGEN_3D_Remote::~NETGENPlugin_NETGEN_3D_Remote()
{
}


void NETGENPlugin_NETGEN_3D_Remote::fillParameters(const NETGENPlugin_Hypothesis* hyp, netgen_params &aParams)
{
  aParams.maxh               = hyp->GetMaxSize();
  aParams.minh               = hyp->GetMinSize();
  aParams.segmentsperedge    = hyp->GetNbSegPerEdge();
  aParams.grading            = hyp->GetGrowthRate();
  aParams.curvaturesafety    = hyp->GetNbSegPerRadius();
  aParams.secondorder        = hyp->GetSecondOrder() ? 1 : 0;
  aParams.quad               = hyp->GetQuadAllowed() ? 1 : 0;
  aParams.optimize           = hyp->GetOptimize();
  aParams.fineness           = hyp->GetFineness();
  aParams.uselocalh          = hyp->GetSurfaceCurvature();
  aParams.merge_solids       = hyp->GetFuseEdges();
  aParams.chordalError       = hyp->GetChordalErrorEnabled() ? hyp->GetChordalError() : -1.;
  aParams.optsteps2d         = aParams.optimize ? hyp->GetNbSurfOptSteps() : 0;
  aParams.optsteps3d         = aParams.optimize ? hyp->GetNbVolOptSteps()  : 0;
  aParams.elsizeweight       = hyp->GetElemSizeWeight();
  aParams.opterrpow          = hyp->GetWorstElemMeasure();
  aParams.delaunay           = hyp->GetUseDelauney();
  aParams.checkoverlap       = hyp->GetCheckOverlapping();
  aParams.checkchartboundary = hyp->GetCheckChartBoundary();
#ifdef NETGEN_V6
  // std::string
  aParams.meshsizefilename = hyp->GetMeshSizeFile();
#else
  // const char*
  aParams.meshsizefilename = hyp->GetMeshSizeFile();
#endif
#ifdef NETGEN_V6
  aParams.closeedgefac = 2;
#else
  aParams.closeedgefac = 0;
#endif
}

// write in a binary file the orientation for each 2D element of the mesh
void NETGENPlugin_NETGEN_3D_Remote::exportElementOrientation(SMESH_Mesh& aMesh,
                                                      const TopoDS_Shape& aShape,
                                                      netgen_params& aParams,
                                                      const std::string output_file)
{
  SMESH_MesherHelper helper(aMesh);
  NETGENPlugin_Internals internals( aMesh, aShape, /*is3D=*/true );
  SMESH_ProxyMesh::Ptr proxyMesh( new SMESH_ProxyMesh( aMesh ));
  std::map<vtkIdType, bool> elemOrientation;

  for ( TopExp_Explorer exFa( aShape, TopAbs_FACE ); exFa.More(); exFa.Next())
  {
    const TopoDS_Shape& aShapeFace = exFa.Current();
    int faceID = aMesh.GetMeshDS()->ShapeToIndex( aShapeFace );
    bool isInternalFace = internals.isInternalShape( faceID );
    bool isRev = false;
    if ( !isInternalFace &&
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
    while ( iteratorElem->more() ) // loop on elements on a geom face
    {
      // check mesh face
      const SMDS_MeshElement* elem = iteratorElem->next();
      if ( !elem )
        error( COMPERR_BAD_INPUT_MESH, "Null element encounters");
      if ( elem->NbCornerNodes() != 3 )
        error( COMPERR_BAD_INPUT_MESH, "Not triangle element encounters");
      elemOrientation[elem->GetID()] = isRev;
    } // loop on elements on a face
  } // loop on faces of a SOLID or SHELL

  {
    std::ofstream df(output_file, ios::out|ios::binary);
    int size=elemOrientation.size();

    df.write((char*)&size, sizeof(int));
    for(auto const& [id, orient]:elemOrientation){
      df.write((char*)&id, sizeof(vtkIdType));
      df.write((char*)&orient, sizeof(bool));
    }
  }
}


bool NETGENPlugin_NETGEN_3D_Remote::Compute(SMESH_Mesh&         aMesh,
                                           const TopoDS_Shape& aShape)
{
  aMesh.Lock();
  SMESH_Hypothesis::Hypothesis_Status hypStatus;
  NETGENPlugin_NETGEN_3D::CheckHypothesis(aMesh, aShape, hypStatus);


  // Temporary folder for run
  fs::path tmp_folder = aMesh.tmp_folder / fs::unique_path(fs::path("Volume-%%%%-%%%%"));
  fs::create_directories(tmp_folder);
  // Using MESH2D generated after all triangles where created.
  fs::path mesh_file=aMesh.tmp_folder / fs::path("Mesh2D.med");
  fs::path element_orientation_file=tmp_folder / fs::path("element_orientation.dat");
  fs::path new_element_file=tmp_folder / fs::path("new_elements.dat");
  fs::path tmp_mesh_file=tmp_folder / fs::path("tmp_mesh.med");
  // Not used kept for debug
  //fs::path output_mesh_file=tmp_folder / fs::path("output_mesh.med");
  fs::path shape_file=tmp_folder / fs::path("shape.brep");
  fs::path param_file=tmp_folder / fs::path("netgen3d_param.txt");
  fs::path log_file=tmp_folder / fs::path("run.log");
  fs::path cmd_file=tmp_folder / fs::path("cmd.log");
  //TODO: Handle variable mesh_name
  std::string mesh_name = "Maillage_1";

  //Writing Shape
  exportShape(shape_file.string(), aShape);

  //Writing hypo
  netgen_params aParams;
  std::cout << _hypParameters << std::endl;
  fillParameters(_hypParameters, aParams);

  exportNetgenParams(param_file.string(), aParams);

  // Exporting element orientation
  exportElementOrientation(aMesh, aShape, aParams, element_orientation_file.string());

  aMesh.Unlock();
  // Calling run_mesher
  // TODO: check if we need to handle the .exe for windows
  std::string cmd;
  fs::path run_mesher_exe =
    fs::path(std::getenv("NETGENPLUGIN_ROOT_DIR"))/
    fs::path("bin")/
    fs::path("salome")/
    fs::path("NETGENPlugin_Runner");

  cmd = run_mesher_exe.string() +
                  " NETGEN3D " + mesh_file.string() + " "
                               + shape_file.string() + " "
                               + param_file.string() + " "
                               + element_orientation_file.string() + " "
                               + std::to_string(aMesh.GetMesherNbThreads()) + " "
                               + new_element_file.string() + " "
                               + "NONE";
  // Writing command in log
  {
    std::ofstream flog(cmd_file.string());
    flog << cmd << endl;
    flog << endl;
  }
  //std::cout << "Running command: " << std::endl;
  //std::cout << cmd << std::endl;


  // Building arguments for QProcess
  QString program = run_mesher_exe.c_str();
  QStringList arguments;
  arguments << "NETGEN3D";
  arguments << mesh_file.c_str();
  arguments << shape_file.c_str();
  arguments << param_file.c_str();
  arguments << element_orientation_file.c_str();
  arguments << std::to_string(aMesh.GetMesherNbThreads()).c_str();
  arguments << new_element_file.c_str();
  arguments << "NONE";
  QString out_file = log_file.c_str();
  QProcess myProcess;
  myProcess.setStandardOutputFile(out_file);

  myProcess.start(program, arguments);
  myProcess.waitForFinished();
  int ret = myProcess.exitStatus();

  if(ret != 0){
    // Run crahed
    std::cout << "Issue with command: " << std::endl;
    std::cout << "See log for more details: " << log_file.string() << std::endl;
    std::cout << cmd << std::endl;
    return false;
  }

  aMesh.Lock();
  {
    std::ifstream df(new_element_file.string(), ios::binary);

    int Netgen_NbOfNodes;
    int Netgen_NbOfNodesNew;
    int Netgen_NbOfTetra;
    double Netgen_point[3];
    int    Netgen_tetrahedron[4];
    int nodeID;

    SMESH_MesherHelper helper(aMesh);
    // This function is mandatory for setElementsOnShape to work
    helper.IsQuadraticSubMesh(aShape);
    helper.SetElementsOnShape( true );

    // Number of nodes in intial mesh
    df.read((char*) &Netgen_NbOfNodes, sizeof(int));
    // Number of nodes added by netgen
    df.read((char*) &Netgen_NbOfNodesNew, sizeof(int));

    // Filling nodevec (correspondence netgen numbering mesh numbering)
    vector< const SMDS_MeshNode* > nodeVec ( Netgen_NbOfNodesNew + 1 );
    //vector<int> nodeTmpVec ( Netgen_NbOfNodesNew + 1 );
    SMESHDS_Mesh * meshDS = helper.GetMeshDS();
    for (int nodeIndex = 1 ; nodeIndex <= Netgen_NbOfNodes; ++nodeIndex )
    {
      //Id of the point
      df.read((char*) &nodeID, sizeof(int));
      nodeVec.at(nodeIndex) = meshDS->FindNode(nodeID);
    }

    // Add new points and update nodeVec
    for (int nodeIndex = Netgen_NbOfNodes +1 ; nodeIndex <= Netgen_NbOfNodesNew; ++nodeIndex )
    {
      df.read((char *) &Netgen_point, sizeof(double)*3);

      nodeVec.at(nodeIndex) = helper.AddNode(Netgen_point[0],
                                 Netgen_point[1],
                                 Netgen_point[2]);
    }

    // Add tetrahedrons
    df.read((char*) &Netgen_NbOfTetra, sizeof(int));

    for ( int elemIndex = 1; elemIndex <= Netgen_NbOfTetra; ++elemIndex )
    {
      df.read((char*) &Netgen_tetrahedron, sizeof(int)*4);
      helper.AddVolume(
                    nodeVec.at( Netgen_tetrahedron[0] ),
                    nodeVec.at( Netgen_tetrahedron[1] ),
                    nodeVec.at( Netgen_tetrahedron[2] ),
                    nodeVec.at( Netgen_tetrahedron[3] ));
    }
  }

  aMesh.Unlock();

  return true;
}