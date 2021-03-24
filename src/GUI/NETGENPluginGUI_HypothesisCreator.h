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

//  NETGENPlugin GUI: GUI for plugged-in mesher NETGENPlugin
//  File   : NETGENPluginGUI_HypothesisCreator.h
//  Author : Michael Zorin
//  Module : NETGENPlugin
//  $Header: 
//
#ifndef NETGENPLUGINGUI_HypothesisCreator_HeaderFile
#define NETGENPLUGINGUI_HypothesisCreator_HeaderFile

#include "NETGENPluginGUI.h"

#include <SMESHGUI_Hypotheses.h>

#include <TopAbs_ShapeEnum.hxx>

class GeomSelectionTools;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QTableWidget;
class SMESHGUI_SpinBox;
class SalomeApp_IntSpinBox;

typedef struct
{
  double  myMaxSize, myMinSize, myGrowthRate, myNbSegPerEdge, myNbSegPerRadius, myRidgeAngle, myChordalError, myElemSizeWeight, myEdgeCornerAngle, myChartAngle, myOuterChartAngle, myRestHChartDistFactor, myRestHLineLengthFactor, myRestHCloseEdgeFactor, myRestHSurfCurvFactor, myRestHEdgeAngleFactor, myRestHSurfMeshCurvFactor;
  int     myFineness, myNbSurfOptSteps, myNbVolOptSteps, myWorstElemMeasure;
  bool    mySecondOrder, myAllowQuadrangles, myOptimize, mySurfaceCurvature, myFuseEdges, myChordalErrorEnabled, myUseDelauney, myCheckOverlapping, myCheckChartBoundary, myRestHChartDistEnable, myRestHLineLengthEnable, myRestHCloseEdgeEnable, myRestHSurfCurvEnable, myRestHEdgeAngleEnable, myRestHSurfMeshCurvEnable, myKeepExistingEdges, myMakeGroupsOfSurfaces;
  QString myName, myMeshSizeFile;
  QString myMaxSizeVar, myMinSizeVar, myGrowthRateVar, myNbSegPerEdgeVar, myNbSegPerRadiusVar, myRidgeAngleVar, myChordalErrorVar, myNbSurfOptStepsVar, myNbVolOptStepsVar, myElemSizeWeightVar, myWorstElemMeasureVar, myEdgeCornerAngleVar, myChartAngleVar, myOuterChartAngleVar, myRestHChartDistFactorVar, myRestHLineLengthFactorVar, myRestHCloseEdgeFactorVar, myRestHSurfCurvFactorVar, myRestHEdgeAngleFactorVar, myRestHSurfMeshCurvFactorVar;
} NetgenHypothesisData;

/*!
 * \brief Class for creation of NETGEN2D and NETGEN3D hypotheses
*/
class NETGENPLUGIN_GUI_EXPORT NETGENPluginGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  NETGENPluginGUI_HypothesisCreator( const QString& );
  virtual ~NETGENPluginGUI_HypothesisCreator();

  virtual bool     checkParams(QString&) const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual QString  storeParams   () const;
  
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

protected slots:
  virtual void     onFinenessChanged();
  virtual void     onChordalErrorEnabled();
  virtual void     onSurfaceCurvatureChanged();
  virtual void     onAddLocalSizeOnVertex();
  virtual void     onAddLocalSizeOnEdge();
  virtual void     onAddLocalSizeOnFace();
  virtual void     onAddLocalSizeOnSolid();
  virtual void     onRemoveLocalSizeOnShape();
  virtual void     onSetLocalSize(int,int);
  virtual void     onSetSizeFile();
  virtual void     onSTLEnable();

private:
  bool readParamsFromHypo( NetgenHypothesisData& ) const;
  bool readParamsFromWidgets( NetgenHypothesisData& ) const;
  bool storeParamsToHypo( const NetgenHypothesisData& ) const;
  GeomSelectionTools* getGeomSelectionTools();
  void addLocalSizeOnShape(TopAbs_ShapeEnum);

private:
 QLineEdit*            myName;
 SMESHGUI_SpinBox*     myMaxSize;
 SMESHGUI_SpinBox*     myMinSize;
 QCheckBox*            mySecondOrder;
 QCheckBox*            myOptimize;
 QComboBox*            myFineness;
 SMESHGUI_SpinBox*     myGrowthRate;
 SMESHGUI_SpinBox*     myNbSegPerEdge;
 SMESHGUI_SpinBox*     myNbSegPerRadius;
 QCheckBox*            myChordalErrorEnabled;
 SMESHGUI_SpinBox*     myChordalError;
 QCheckBox*            myAllowQuadrangles;
 QCheckBox*            mySurfaceCurvature;
 // optimizer
 SMESHGUI_SpinBox*     myElemSizeWeight;
 SalomeApp_IntSpinBox* myNbSurfOptSteps;
 SalomeApp_IntSpinBox* myNbVolOptSteps;
 // insider
 QCheckBox*            myFuseEdges;
 SalomeApp_IntSpinBox* myWorstElemMeasure;
 QCheckBox*            myUseDelauney;
 QCheckBox*            myCheckOverlapping;
 QCheckBox*            myCheckChartBoundary;
 // stl options
 QCheckBox*            myKeepExistingEdges;
 QCheckBox*            myMakeGroupsOfSurfaces;
 // stl charts
 SMESHGUI_SpinBox*     myRidgeAngle;
 SMESHGUI_SpinBox*     myEdgeCornerAngle;
 SMESHGUI_SpinBox*     myChartAngle;
 SMESHGUI_SpinBox*     myOuterChartAngle;
 // stl size
 SMESHGUI_SpinBox*     myRestHChartDistFactor;
 SMESHGUI_SpinBox*     myRestHLineLengthFactor;
 SMESHGUI_SpinBox*     myRestHCloseEdgeFactor;
 SMESHGUI_SpinBox*     myRestHSurfCurvFactor;
 SMESHGUI_SpinBox*     myRestHEdgeAngleFactor;
 SMESHGUI_SpinBox*     myRestHSurfMeshCurvFactor;
 QCheckBox*            myRestHChartDistEnable;
 QCheckBox*            myRestHLineLengthEnable;
 QCheckBox*            myRestHCloseEdgeEnable;
 QCheckBox*            myRestHSurfCurvEnable;
 QCheckBox*            myRestHEdgeAngleEnable;
 QCheckBox*            myRestHSurfMeshCurvEnable;

 bool myIs2D;   // 2D or 3D
 bool myIsONLY; // one dim or several

 QLineEdit*             myMeshSizeFile;
 QTableWidget*          myLocalSizeTable;
 GeomSelectionTools*    myGeomSelectionTools;
 QMap<QString, QString> myLocalSizeMap;
};

#endif
