// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

class SMESHGUI_SpinBox;
class GeomSelectionTools;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QTableWidget;

typedef struct
{
  double              myMaxSize, myGrowthRate, myNbSegPerEdge, myNbSegPerRadius;
  int                 myFineness;
  bool                mySecondOrder, myAllowQuadrangles, myOptimize;
  QString             myName;
  QString             myMaxSizeVar, myGrowthRateVar, myNbSegPerEdgeVar, myNbSegPerRadiusVar;
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

  virtual bool     checkParams(QString& msg) const;
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
  virtual void     onAddLocalSizeOnVertex();
  virtual void     onAddLocalSizeOnEdge();
  virtual void     onAddLocalSizeOnFace();
  virtual void     onRemoveLocalSizeOnShape();
  virtual void     onSetLocalSize(int,int);

private:
  bool readParamsFromHypo( NetgenHypothesisData& ) const;
  bool readParamsFromWidgets( NetgenHypothesisData& ) const;
  bool storeParamsToHypo( const NetgenHypothesisData& ) const;
  GeomSelectionTools* getGeomSelectionTools();
  void addLocalSizeOnShape(TopAbs_ShapeEnum);

private:
 QLineEdit*        myName;
 SMESHGUI_SpinBox* myMaxSize;
 QCheckBox*        mySecondOrder;
 QCheckBox*        myOptimize;
 QComboBox*        myFineness;
 SMESHGUI_SpinBox* myGrowthRate;
 SMESHGUI_SpinBox* myNbSegPerEdge;
 SMESHGUI_SpinBox* myNbSegPerRadius;
 QCheckBox*        myAllowQuadrangles;

 bool myIs2D;
 bool myIsONLY;

 QTableWidget* myLocalSizeTable;
 GeomSelectionTools* myGeomSelectionTools;
 QMap<QString, QString> myLocalSizeMap;
};

#endif
