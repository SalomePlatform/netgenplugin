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

//  File   : NETGENPlugin_DriverParam.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : NETGEN
//
#include "NETGENPlugin_DriverParam.hxx"

#include "NETGENPlugin_Hypothesis.hxx"

#include <SMESH_Gen.hxx>
#include <StdMeshers_MaxElementVolume.hxx>

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

/**
 * @brief Print content of a netgen_params
 *
 * @param aParams The object to display
 */
void printNetgenParams(netgen_params& aParams){
  std::cout << "has_netgen_param: " << aParams.has_netgen_param << std::endl;
  std::cout << "maxh: " << aParams.maxh << std::endl;
  std::cout << "minh: " << aParams.minh << std::endl;
  std::cout << "segmentsperedge: " << aParams.segmentsperedge << std::endl;
  std::cout << "grading: " << aParams.grading << std::endl;
  std::cout << "curvaturesafety: " << aParams.curvaturesafety << std::endl;
  std::cout << "secondorder: " << aParams.secondorder << std::endl;
  std::cout << "quad: " << aParams.quad << std::endl;
  std::cout << "optimize: " << aParams.optimize << std::endl;
  std::cout << "fineness: " << aParams.fineness << std::endl;
  std::cout << "uselocalh: " << aParams.uselocalh << std::endl;
  std::cout << "merge_solids: " << aParams.merge_solids << std::endl;
  std::cout << "chordalError: " << aParams.chordalError << std::endl;
  std::cout << "optsteps2d: " << aParams.optsteps2d << std::endl;
  std::cout << "optsteps3d: " << aParams.optsteps3d << std::endl;
  std::cout << "elsizeweight: " << aParams.elsizeweight << std::endl;
  std::cout << "opterrpow: " << aParams.opterrpow << std::endl;
  std::cout << "delaunay: " << aParams.delaunay << std::endl;
  std::cout << "checkoverlap: " << aParams.checkoverlap << std::endl;
  std::cout << "checkchartboundary: " << aParams.checkchartboundary << std::endl;
  std::cout << "closeedgefac: " << aParams.closeedgefac << std::endl;
  std::cout << "has_local_size: " << aParams.has_local_size << std::endl;
  std::cout << "meshsizefilename: " << aParams.meshsizefilename << std::endl;
  std::cout << "has_maxelementvolume_hyp: " << aParams.has_maxelementvolume_hyp << std::endl;
  std::cout << "maxElementVolume: " << aParams.maxElementVolume << std::endl;
  std::cout << "has_LengthFromEdges_hyp: " << aParams.has_LengthFromEdges_hyp << std::endl;
}

/**
 * @brief Import a param_file into a netgen_params structure
 *
 * @param param_file Name of the file
 * @param aParams Structure to fill
 */
void importNetgenParams(const std::string param_file, netgen_params& aParams, SMESH_Gen *gen){
  std::ifstream myfile(param_file);
  std::string line;

  std::getline(myfile, line);
  aParams.has_netgen_param = std::stoi(line);
  std::getline(myfile, line);
  aParams.maxh = std::stod(line);
  std::getline(myfile, line);
  aParams.minh = std::stod(line);
  std::getline(myfile, line);
  aParams.segmentsperedge = std::stod(line);
  std::getline(myfile, line);
  aParams.grading = std::stod(line);
  std::getline(myfile, line);
  aParams.curvaturesafety = std::stod(line);
  std::getline(myfile, line);
  aParams.secondorder = std::stoi(line);
  std::getline(myfile, line);
  aParams.quad = std::stoi(line);
  std::getline(myfile, line);
  aParams.optimize = std::stoi(line);
  std::getline(myfile, line);
  aParams.fineness = std::stoi(line);
  std::getline(myfile, line);
  aParams.uselocalh = std::stoi(line);
  std::getline(myfile, line);
  aParams.merge_solids = std::stoi(line);
  std::getline(myfile, line);
  aParams.chordalError = std::stod(line);
  std::getline(myfile, line);
  aParams.optsteps2d = std::stoi(line);
  std::getline(myfile, line);
  aParams.optsteps3d = std::stoi(line);
  std::getline(myfile, line);
  aParams.elsizeweight = std::stod(line);
  std::getline(myfile, line);
  aParams.opterrpow = std::stoi(line);
  std::getline(myfile, line);
  aParams.delaunay = std::stoi(line);
  std::getline(myfile, line);
  aParams.checkoverlap = std::stoi(line);
  std::getline(myfile, line);
  aParams.checkchartboundary = std::stoi(line);
  std::getline(myfile, line);
  aParams.closeedgefac = std::stoi(line);
  std::getline(myfile, line);
  aParams.has_local_size = std::stoi(line);
  std::getline(myfile, line);
  aParams.meshsizefilename = line;
  std::getline(myfile, line);
  aParams.has_maxelementvolume_hyp = std::stoi(line);
  std::getline(myfile, line);
  aParams.maxElementVolume = std::stod(line);
  std::getline(myfile, line);
  aParams.maxElementVolume = std::stoi(line);

  if(aParams.has_netgen_param){
    aParams._hypParameters = new NETGENPlugin_Hypothesis(0, gen);

    aParams._hypParameters->SetMaxSize(aParams.maxh);
    aParams._hypParameters->SetMinSize(aParams.minh);
    aParams._hypParameters->SetNbSegPerEdge(aParams.segmentsperedge);
    aParams._hypParameters->SetGrowthRate(aParams.grading);
    aParams._hypParameters->SetNbSegPerRadius(aParams.curvaturesafety);
    aParams._hypParameters->SetSecondOrder(aParams.secondorder);
    aParams._hypParameters->SetQuadAllowed(aParams.quad);
    aParams._hypParameters->SetOptimize(aParams.optimize);
    aParams._hypParameters->SetFineness((NETGENPlugin_Hypothesis::Fineness)aParams.fineness);
    aParams._hypParameters->SetSurfaceCurvature(aParams.uselocalh);
    aParams._hypParameters->SetFuseEdges(aParams.merge_solids);
    aParams._hypParameters->SetChordalErrorEnabled(aParams.chordalError);
    if(aParams.optimize){
      aParams._hypParameters->SetNbSurfOptSteps(aParams.optsteps2d);
      aParams._hypParameters->SetNbVolOptSteps(aParams.optsteps3d);
    }
    aParams._hypParameters->SetElemSizeWeight(aParams.elsizeweight);
    aParams._hypParameters->SetWorstElemMeasure(aParams.opterrpow);
    aParams._hypParameters->SetUseDelauney(aParams.delaunay);
    aParams._hypParameters->SetCheckOverlapping(aParams.checkoverlap);
    aParams._hypParameters->SetCheckChartBoundary(aParams.checkchartboundary);
    aParams._hypParameters->SetMeshSizeFile(aParams.meshsizefilename);
  }
  if(aParams.has_maxelementvolume_hyp){
    aParams._hypMaxElementVolume = new StdMeshers_MaxElementVolume(1, gen);
  }
  // TODO: Handle viscous layer
};

/**
 * @brief Writes the content of a netgen_param into a file
 *
 * @param param_file the file
 * @param aParams the object
 */
void exportNetgenParams(const std::string param_file, netgen_params& aParams){
  std::ofstream myfile(param_file);
  myfile << aParams.has_netgen_param << std::endl;
  myfile << aParams.maxh << std::endl;
  myfile << aParams.minh << std::endl;
  myfile << aParams.segmentsperedge << std::endl;
  myfile << aParams.grading << std::endl;
  myfile << aParams.curvaturesafety << std::endl;
  myfile << aParams.secondorder << std::endl;
  myfile << aParams.quad << std::endl;
  myfile << aParams.optimize << std::endl;
  myfile << aParams.fineness << std::endl;
  myfile << aParams.uselocalh << std::endl;
  myfile << aParams.merge_solids << std::endl;
  myfile << aParams.chordalError << std::endl;
  myfile << aParams.optsteps2d << std::endl;
  myfile << aParams.optsteps3d << std::endl;
  myfile << aParams.elsizeweight << std::endl;
  myfile << aParams.opterrpow << std::endl;
  myfile << aParams.delaunay << std::endl;
  myfile << aParams.checkoverlap << std::endl;
  myfile << aParams.checkchartboundary << std::endl;
  myfile << aParams.closeedgefac << std::endl;
  myfile << aParams.has_local_size << std::endl;
  myfile << aParams.meshsizefilename << std::endl;
  myfile << aParams.has_maxelementvolume_hyp << std::endl;
  myfile << aParams.maxElementVolume << std::endl;
  myfile << aParams.has_LengthFromEdges_hyp << std::endl;
};
