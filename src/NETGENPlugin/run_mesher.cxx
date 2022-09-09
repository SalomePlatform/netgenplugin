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

//  File   : run_mesher.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "DriverStep.hxx"
#include "DriverMesh.hxx"
#include "netgen_param.hxx"
#include "netgen_mesher.hxx"

#include <SMESH_Mesh.hxx>
#include <SMESH_Gen.hxx>

#include <TopoDS_Shape.hxx>
#include <iostream>

#include <chrono>

/**
 * @brief Test of shape Import/Export
 *
 * @param shape_file
 */
void test_shape(){

  std::cout << "Testing Shape Import/Export" << std::endl;
  std::string shape_file = "box.step";

  TopoDS_Shape myShape;

  // Test 1 import -> export cmp files
  std::string shape_file2 = "/tmp/shape.step";

  import_shape(shape_file, myShape);
  export_shape(shape_file2, myShape);

  assert(diff_step_file(shape_file, shape_file2));

  // Test 2 import->export->import cmp TopoDS_Shape
  std::string shape_file3 = "/tmp/shape2.step";
  TopoDS_Shape myShape1, myShape2;

  import_shape(shape_file, myShape1);

  export_shape(shape_file3, myShape1);

  import_shape(shape_file3, myShape2);

  // TODO: See why this does not work
  // TShape seems to be different
  //assert(myShape1.IsSame(myShape2));

}

/**
 * @brief test of mesh import/export
 *
 * @param mesh_file
 */
void test_mesh(){

  std::cout << "Testing Mesh Import/Export" << std::endl;
  std::string mesh_file = "box.med";
  SMESH_Gen gen;

  SMESH_Mesh *myMesh = gen.CreateMesh(false);
  std::string mesh_name = "Maillage_1";

  // Test 1 import -> export cmp files
  std::string mesh_file2 = "/tmp/mesh.med";

  import_mesh(mesh_file, *myMesh, mesh_name);
  export_mesh(mesh_file2, *myMesh, mesh_name);

  assert(diff_med_file(mesh_file, mesh_file2, mesh_name));

  // TODO: Compare the two med files via dump ?

  // Test 2 import->export->import cmp TopoDS_Shape
  std::string mesh_file3 = "/tmp/mesh2.med";
  SMESH_Mesh *myMesh1 = gen.CreateMesh(false);
  SMESH_Mesh *myMesh2 = gen.CreateMesh(false);

  import_mesh(mesh_file, *myMesh1, mesh_name);

  export_mesh(mesh_file3, *myMesh1, mesh_name);

  import_mesh(mesh_file3, *myMesh2, mesh_name);

  // TODO: Compare SMESH_Mesh
  //assert(myMesh1==myMesh2);
}

/**
 * @brief Test of import/export of netgen param
 *
 */
void test_netgen_params(){

  std::string param_file = "/tmp/netgen_param.txt";
  netgen_params myParams, myParams2;
  myParams.has_netgen_param = true;
  myParams.maxh = 34.64;
  myParams.minh = 0.14;
  myParams.segmentsperedge = 15;
  myParams.grading = 0.2;
  myParams.curvaturesafety = 1.5;
  myParams.secondorder = false;
  myParams.quad = false;
  myParams.optimize = true;
  myParams.fineness = 5;
  myParams.uselocalh = true;
  myParams.merge_solids = true;
  myParams.chordalError = -1;
  myParams.optsteps2d = 3;
  myParams.optsteps3d = 3;
  myParams.elsizeweight = 0.2;
  myParams.opterrpow = 2;
  myParams.delaunay = true;
  myParams.checkoverlap = true;
  myParams.checkchartboundary = false;
  myParams.closeedgefac = 2;
  myParams.has_local_size = false;
  myParams.meshsizefilename = "";
  myParams.has_maxelementvolume_hyp = false;
  myParams.maxElementVolume = 0.0;

  export_netgen_params(param_file, myParams);
  import_netgen_params(param_file, myParams2);

  assert(diff_netgen_params(myParams, myParams2));

};

void test_netgen3d(){

  std::cout << "Testing NETGEN 3D mesher" << std::endl;
  netgen3d("box_partial2D1D-2.med",
           "box-2.step",
           "box_param.txt",
           "element_orient.dat",
           "new_element.dat",
           true,
           "box_with3D.med");

  // TODO: Check result
}

/**
 * @brief Main function
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return error code
 */
int main(int argc, char *argv[]){

  if(argc!=9||(argc==2 && (argv[1] == "-h" || argv[1]=="--help"))){
    std::cout << "Error in number of argument"<<std::endl;
    std::cout << "Syntax:"<<std::endl;
    std::cout << "run_mesher MESHER INPUT_MESH_FILE SHAPE_FILE HYPO_FILE" << std::endl;
    std::cout << "           ELEM_ORIENT_FILE NEW_ELEMENT_FILE OUTPUT_MESH_FILE" << std::endl;
    std::cout << std::endl;
    std::cout << "Args:" << std::endl;
    std::cout << "  MESHER: mesher to use from (NETGEN3D, NETGEN2D)" << std::endl;
    std::cout << "  INPUT_MESH_FILE: MED File containing lower-dimension-elements already meshed" << std::endl;
    std::cout << "  SHAPE_FILE: STEP file containing the shape to mesh" << std::endl;
    std::cout << "  HYPO_FILE: Ascii file containint the list of parameters" << std::endl;
    std::cout << "  ELEM_ORIENT_FILE: binary file containing the list of element from INPUT_MESH_FILE associated to the shape and their orientation" << std::endl;
    std::cout << "  NEW_ELEMENT_FILE: (out) contains elements and nodes added by the meshing" << std::endl;
    std::cout << "  OUTPUT_MESH: If !=0 will export mesh into OUTPUT_MESH_FILE " << std::endl;
    std::cout << "  OUTPUT_MESH_FILE: MED File containing the mesh after the run of the mesher" << std::endl;
    return 0;
  }
  std::string mesher=argv[1];
  std::string input_mesh_file=argv[2];
  std::string shape_file=argv[3];
  std::string hypo_file=argv[4];
  std::string element_orientation_file=argv[5];
  std::string new_element_file=argv[6];
  bool output_mesh = std::stoi(argv[7]) != 0;
  std::string output_mesh_file=argv[8];

  if (mesher=="test"){
    std::cout << "Running tests" << std::endl;
    test_shape();
    test_mesh();
    test_netgen_params();
    test_netgen3d();
  } else if (mesher=="NETGEN3D"){
    auto begin = std::chrono::high_resolution_clock::now();
    netgen3d(input_mesh_file,
             shape_file,
             hypo_file,
             element_orientation_file,
             new_element_file,
             output_mesh,
             output_mesh_file);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    std::cout << "Time elapsed: " << elapsed.count()*1e-9 << std::endl;
  } else if (mesher=="NETGEN2D"){
    netgen2d(input_mesh_file,
             shape_file,
             hypo_file,
             element_orientation_file,
             new_element_file,
             output_mesh,
             output_mesh_file);
  } else {
    std::cerr << "Unknown mesher:" << mesher << std::endl;
  }
  return 0;
}
