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

//  File   : netgen_param.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//
#include "netgen_param.hxx"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

/**
 * @brief Print content of a netgen_params
 *
 * @param aParams The object to display
 */
void print_netgen_params(netgen_params& aParams){
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
  std::cout << "has_local_size: " << aParams.has_local_size << std::endl;
  std::cout << "meshsizefilename: " << aParams.meshsizefilename << std::endl;
  std::cout << "has_maxelementvolume_hyp: " << aParams.has_maxelementvolume_hyp << std::endl;
  std::cout << "maxElementVolume: " << aParams.maxElementVolume << std::endl;
  std::cout << "closeedgefac: " << aParams.closeedgefac << std::endl;
}

/**
 * @brief Import a param_file into a netgen_params structure
 *
 * @param param_file Name of the file
 * @param aParams Structure to fill
 */
void import_netgen_params(const std::string param_file, netgen_params& aParams){
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

  myfile.close();
};

/**
 * @brief Writes the content of a netgen_param into a file
 *
 * @param param_file the file
 * @param aParams the object
 */
void export_netgen_params(const std::string param_file, netgen_params& aParams){
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

  myfile.close();
};

/**
 * @brief Compares two netgen_parms object
 *
 * @param params1 Object 1
 * @param params2 Object 2

 * @return true if the two object are identical
 */
bool diff_netgen_params(netgen_params params1, netgen_params params2){
  bool ret = true;
  ret &= params1.maxh == params2.maxh;
  ret &= params1.minh == params2.minh;
  ret &= params1.segmentsperedge == params2.segmentsperedge;
  ret &= params1.grading == params2.grading;
  ret &= params1.curvaturesafety == params2.curvaturesafety;
  ret &= params1.secondorder == params2.secondorder;
  ret &= params1.quad == params2.quad;
  ret &= params1.optimize == params2.optimize;
  ret &= params1.fineness == params2.fineness;
  ret &= params1.uselocalh == params2.uselocalh;
  ret &= params1.merge_solids == params2.merge_solids;
  ret &= params1.chordalError == params2.chordalError;
  ret &= params1.optsteps2d == params2.optsteps2d;
  ret &= params1.optsteps3d == params2.optsteps3d;
  ret &= params1.elsizeweight == params2.elsizeweight;
  ret &= params1.opterrpow == params2.opterrpow;
  ret &= params1.delaunay == params2.delaunay;
  ret &= params1.checkoverlap == params2.checkoverlap;
  ret &= params1.checkchartboundary == params2.checkchartboundary;
  ret &= params1.closeedgefac == params2.closeedgefac;
  ret &= params1.has_local_size == params2.has_local_size;
  ret &= params1.meshsizefilename == params2.meshsizefilename;
  ret &= params1.has_maxelementvolume_hyp == params2.has_maxelementvolume_hyp;
  ret &= params1.maxElementVolume == params2.maxElementVolume;

  return ret;
}
