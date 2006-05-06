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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
// File      : NETGENPlugin_Hypothesis.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 27/03/2006
// Project   : SALOME
// $Header$
//=============================================================================

#ifndef _NETGENPlugin_Hypothesis_HXX_
#define _NETGENPlugin_Hypothesis_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

//  Parameters for work of NETGEN
//

class NETGENPlugin_Hypothesis: public SMESH_Hypothesis
{
public:

  NETGENPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen * gen);

  void SetMaxSize(double theSize);
  double GetMaxSize() const { return _maxSize; }

  void SetSecondOrder(bool theVal);
  bool GetSecondOrder() const { return _secondOrder; }

  void SetOptimize(bool theVal);
  bool GetOptimize() const { return _optimize; }

  enum Fineness
  {
    VeryCoarse,
    Coarse,
    Moderate,
    Fine,
    VeryFine,
    UserDefined
  };

  void SetFineness(Fineness theFineness);
  Fineness GetFineness() const { return _fineness; }

  // the following parameters are controlled by Fineness

  void SetGrowthRate(double theRate);
  double GetGrowthRate() const { return _growthRate; }

  void SetNbSegPerEdge(double theVal);
  double GetNbSegPerEdge() const { return _nbSegPerEdge; }

  void SetNbSegPerRadius(double theVal);
  double GetNbSegPerRadius() const { return _nbSegPerRadius; }

  // the default values (taken from NETGEN 4.5 sources)

  static double GetDefaultMaxSize();
  static Fineness GetDefaultFineness();
  static double GetDefaultGrowthRate();
  static double GetDefaultNbSegPerEdge();
  static double GetDefaultNbSegPerRadius();
  static bool GetDefaultSecondOrder();
  static bool GetDefaultOptimize();

  // Persistence
  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator <<(ostream & save, NETGENPlugin_Hypothesis & hyp);
  friend istream & operator >>(istream & load, NETGENPlugin_Hypothesis & hyp);

  /*!
   * \brief Does nothing
   * \param theMesh - the built mesh
   * \param theShape - the geometry of interest
   * \retval bool - always false
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

private:
  double        _maxSize;
  double        _growthRate;
  double        _nbSegPerEdge;
  double        _nbSegPerRadius;
  Fineness      _fineness;
  bool          _secondOrder;
  bool          _optimize;
};

#endif
