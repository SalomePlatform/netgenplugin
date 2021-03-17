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
//  File   : NETGENPluginGUI_HypothesisCreator.cxx
//  Author : Michael Zorin
//  Module : NETGENPlugin
//
#include "NETGENPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_SpinBox.h>
#include <GeomSelectionTools.h>

#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_FileDlg.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_IntSpinBox.h>
#include <SalomeApp_Tools.h>

#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPixmap>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

enum Fineness {
  VeryCoarse,
  Coarse,
  Moderate,
  Fine,
  VeryFine,
  UserDefined
};

enum {
  STD_TAB = 0,
  STL_TAB,
  LSZ_TAB,
  ADV_TAB
};

enum {
  LSZ_ENTRY_COLUMN = 0,
  LSZ_NAME_COLUMN,
  LSZ_LOCALSIZE_COLUMN,
  LSZ_NB_COLUMNS
};

enum {
  LSZ_BTNS = 0,
  LSZ_VERTEX_BTN,
  LSZ_EDGE_BTN,
  LSZ_FACE_BTN,
  LSZ_SOLID_BTN,
  LSZ_SEPARATOR2,
  LSZ_REMOVE_BTN,
  LSZ_FILE_LE = 9
};

template<class SPINBOX, typename VALUETYPE>
void setTextOrVar( SPINBOX* spin, VALUETYPE value, const QString& variable )
{
  if ( spin )
  {
    if ( variable.isEmpty() )
      spin->setValue( value );
    else
      spin->setText( variable );
  }
}

NETGENPluginGUI_HypothesisCreator::NETGENPluginGUI_HypothesisCreator( const QString& theHypType )
  : SMESHGUI_GenericHypothesisCreator( theHypType )
{
  myGeomSelectionTools = NULL;
  myLocalSizeMap.clear();
  myIs2D   = ( theHypType.startsWith("NETGEN_Parameters_2D") ||
               theHypType == "NETGEN_RemesherParameters_2D");
  myIsONLY = ( theHypType == "NETGEN_Parameters_2D_ONLY" ||
               theHypType == "NETGEN_Parameters_3D" ||
               theHypType == "NETGEN_RemesherParameters_2D");
}

NETGENPluginGUI_HypothesisCreator::~NETGENPluginGUI_HypothesisCreator()
{
}

bool NETGENPluginGUI_HypothesisCreator::checkParams(QString& /*msg*/) const
{
  NetgenHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );

  if ( !res ) //  -- issue 0021364: Dump of netgen parameters has duplicate lines
    storeParamsToHypo( data_old );

  return res;
}

QFrame* NETGENPluginGUI_HypothesisCreator::buildFrame()
{
  const bool isRemesher = ( hypType() == "NETGEN_RemesherParameters_2D" );

  QFrame* fr = new QFrame( 0 );
  fr->setObjectName( "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  QTabWidget* tab = new QTabWidget( fr );
  tab->setTabShape( QTabWidget::Rounded );
  tab->setTabPosition( QTabWidget::North );
  lay->addWidget( tab );

  // ==============
  // Arguments TAB
  // ==============

  QWidget* GroupC1 = new QWidget();
  tab->insertTab( STD_TAB, GroupC1, tr( "SMESH_ARGUMENTS" ) );

  QGridLayout* aGroupLayout = new QGridLayout( GroupC1 );
  aGroupLayout->setMargin( 6 );
  aGroupLayout->setSpacing( 11 );

  int row0 = 0;
  myName = 0;
  if ( isCreation() )
  {
    aGroupLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row0, 0 );
    myName = new QLineEdit( GroupC1 );
    myName->setMinimumWidth(160);
    aGroupLayout->addWidget( myName, row0, 1 );
    row0++;
  }

  // Mesh size group
  // ----------------
  {
    QGroupBox* aSizeBox = new QGroupBox( tr("NETGEN_MESH_SIZE"), GroupC1 );
    aGroupLayout->addWidget( aSizeBox, row0, 0, 1, 2 );
    row0++;

    QGridLayout* aSizeLayout = new QGridLayout( aSizeBox );
    aSizeLayout->setSpacing( 6 );
    aSizeLayout->setMargin( 11 );
    int row = 0;

    aSizeLayout->addWidget( new QLabel( tr( "NETGEN_MAX_SIZE" ), aSizeBox ), row, 0 );
    myMaxSize = new SMESHGUI_SpinBox( aSizeBox );
    myMaxSize->RangeStepAndValidator( 1e-07, 1e+06, 10., "length_precision" );
    aSizeLayout->addWidget( myMaxSize, row, 1 );
    row++;

    aSizeLayout->addWidget( new QLabel( tr( "NETGEN_MIN_SIZE" ), aSizeBox ), row, 0 );
    myMinSize = new SMESHGUI_SpinBox( aSizeBox );
    myMinSize->RangeStepAndValidator( 0.0, 1e+06, 10., "length_precision" );
    aSizeLayout->addWidget( myMinSize, row, 1 );
    row++;

    aSizeLayout->addWidget( new QLabel( tr( "NETGEN_FINENESS" ), aSizeBox ), row, 0 );
    myFineness = new QComboBox( aSizeBox );
    QStringList types;
    types << tr( "NETGEN_VERYCOARSE" ) << tr( "NETGEN_COARSE" )   << tr( "NETGEN_MODERATE" ) <<
      tr( "NETGEN_FINE" )       << tr( "NETGEN_VERYFINE" ) << tr( "NETGEN_CUSTOM" );
    myFineness->addItems( types );
    aSizeLayout->addWidget( myFineness, row, 1 );
    connect( myFineness, SIGNAL( activated( int ) ), this, SLOT( onFinenessChanged() ) );
    row++;

    aSizeLayout->addWidget( new QLabel( tr( "NETGEN_GROWTH_RATE" ), aSizeBox ), row, 0 );
    myGrowthRate = new SMESHGUI_SpinBox( aSizeBox );
    myGrowthRate->RangeStepAndValidator( .0001, 10., .1, "parametric_precision" );
    aSizeLayout->addWidget( myGrowthRate, row, 1 );
    row++;

    myNbSegPerEdge = 0;
    myNbSegPerRadius = 0;
    if ( !myIsONLY )
    {
      const double VALUE_MAX = 1.0e+6;

      aSizeLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_EDGE" ), aSizeBox ), row, 0 );
      myNbSegPerEdge = new SMESHGUI_SpinBox( aSizeBox );
      myNbSegPerEdge->RangeStepAndValidator( .2, VALUE_MAX, .1, "parametric_precision" );
      aSizeLayout->addWidget( myNbSegPerEdge, row, 1 );
      row++;

      aSizeLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_RADIUS" ), aSizeBox ), row, 0 );
      myNbSegPerRadius = new SMESHGUI_SpinBox( aSizeBox );
      myNbSegPerRadius->RangeStepAndValidator( .2, VALUE_MAX, .1, "parametric_precision" );
      aSizeLayout->addWidget( myNbSegPerRadius, row, 1 );
      row++;
    }

    myChordalErrorEnabled = 0;
    myChordalError = 0;
    if (( myIs2D && !isRemesher ) || !myIsONLY )
    {
      myChordalErrorEnabled = new QCheckBox( tr( "NETGEN_CHORDAL_ERROR" ), aSizeBox );
      aSizeLayout->addWidget( myChordalErrorEnabled, row, 0 );
      myChordalError = new SMESHGUI_SpinBox( aSizeBox );
      myChordalError->RangeStepAndValidator( COORD_MIN, COORD_MAX, .1, "length_precision" );
      aSizeLayout->addWidget( myChordalError, row, 1 );
      connect( myChordalErrorEnabled, SIGNAL( stateChanged(int)), SLOT( onChordalErrorEnabled()));
      row++;
    }

    mySurfaceCurvature = 0;
    if (( myIs2D && !isRemesher ) || !myIsONLY )
    {
      mySurfaceCurvature = new QCheckBox( tr( "NETGEN_SURFACE_CURVATURE" ), aSizeBox );
      aSizeLayout->addWidget( mySurfaceCurvature, row, 0, 1, 2 );
      connect( mySurfaceCurvature, SIGNAL( stateChanged( int ) ), this, SLOT( onSurfaceCurvatureChanged() ) );
      row++;
    }
  } // end Mesh Size box

  myAllowQuadrangles = 0;
  if ( myIs2D || !myIsONLY ) // disable only for NETGEN 3D
  {
    myAllowQuadrangles = new QCheckBox( tr( "NETGEN_ALLOW_QUADRANGLES" ), GroupC1 );
    aGroupLayout->addWidget( myAllowQuadrangles, row0, 0, 1, 2 );
    row0++;
  }

  mySecondOrder = 0;
  if ( !myIsONLY )
  {
    mySecondOrder = new QCheckBox( tr( "NETGEN_SECOND_ORDER" ), GroupC1 );
    aGroupLayout->addWidget( mySecondOrder, row0, 0, 1, 2 );
    row0++;
  }

  myOptimize = 0;
  // if ( !isRemesher ) ???
  {
    myOptimize = new QCheckBox( tr( "NETGEN_OPTIMIZE" ), GroupC1 );
    aGroupLayout->addWidget( myOptimize, row0, 0, 1, 2 );
    row0++;
  }

  myKeepExistingEdges = myMakeGroupsOfSurfaces = 0;
  if ( isRemesher )
  {
    myKeepExistingEdges = new QCheckBox( tr( "NETGEN_KEEP_EXISTING_EDGES" ), GroupC1 );
    aGroupLayout->addWidget( myKeepExistingEdges, row0, 0, 1, 2 );
    row0++;

    myMakeGroupsOfSurfaces = new QCheckBox( tr( "NETGEN_MAKE_SURFACE_GROUPS" ), GroupC1 );
    aGroupLayout->addWidget( myMakeGroupsOfSurfaces, row0, 0, 1, 2 );
    row0++;
  }

  aGroupLayout->setRowStretch( row0, 1 );

  // ========
  // STL TAB
  // ========

  QWidget* stlGroup = new QWidget();
  QVBoxLayout* stlLay = new QVBoxLayout( stlGroup );
  stlLay->setMargin( 5 );
  stlLay->setSpacing( 10 );

  // Charts group
  {
    QGroupBox* chaGroup = new QGroupBox( tr("NETGEN_STL_CHARTS"), stlGroup );
    stlLay->addWidget( chaGroup );

    QGridLayout* chaLayout = new QGridLayout( chaGroup );
    chaLayout->setMargin( 6 );
    chaLayout->setSpacing( 6 );

    int row = 0;
    chaLayout->addWidget( new QLabel( tr( "NETGEN_RIDGE_ANGLE" ), chaGroup ), row, 0 );
    myRidgeAngle = new SMESHGUI_SpinBox( chaGroup );
    myRidgeAngle->RangeStepAndValidator( 0, 90, 10, "angle_precision" );
    chaLayout->addWidget( myRidgeAngle, row, 1 );
    row++;

    chaLayout->addWidget( new QLabel( tr( "NETGEN_EDGE_CORNER_ANGLE" ), chaGroup ), row, 0 );
    myEdgeCornerAngle = new SMESHGUI_SpinBox( chaGroup );
    myEdgeCornerAngle->RangeStepAndValidator( 0., 180., 20., "angle_precision" );
    chaLayout->addWidget( myEdgeCornerAngle, row, 1 );
    row++;

    chaLayout->addWidget( new QLabel( tr( "NETGEN_CHART_ANGLE" ), chaGroup ), row, 0 );
    myChartAngle = new SMESHGUI_SpinBox( chaGroup );
    myChartAngle->RangeStepAndValidator( 0., 180., 20., "angle_precision" );
    chaLayout->addWidget( myChartAngle, row, 1 );
    row++;

    chaLayout->addWidget( new QLabel( tr( "NETGEN_OUTER_CHART_ANGLE" ), chaGroup ), row, 0 );
    myOuterChartAngle = new SMESHGUI_SpinBox( chaGroup );
    myOuterChartAngle->RangeStepAndValidator( 0., 180., 20., "angle_precision" );
    chaLayout->addWidget( myOuterChartAngle, row, 1 );
    row++;
  }
  // STL size group
  {
    QGroupBox* sizeGroup = new QGroupBox( tr("NETGEN_STL_SIZE"), stlGroup );
    stlLay->addWidget( sizeGroup );

    QGridLayout* sizeLayout = new QGridLayout( sizeGroup );
    sizeLayout->setMargin( 6 );
    sizeLayout->setSpacing( 6 );

    int row = 0;
    myRestHChartDistEnable = new QCheckBox( tr("NETGEN_RESTH_CHART_DIST"), sizeGroup );
    sizeLayout->addWidget( myRestHChartDistEnable, row, 0 );
    myRestHChartDistFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHChartDistFactor->RangeStepAndValidator( 0.2, 5., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHChartDistFactor, row, 1 );
    row++;

    myRestHLineLengthEnable = new QCheckBox( tr("NETGEN_RESTH_LINE_LENGTH"), sizeGroup );
    sizeLayout->addWidget( myRestHLineLengthEnable, row, 0 );
    myRestHLineLengthFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHLineLengthFactor->RangeStepAndValidator( 0.2, 5., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHLineLengthFactor, row, 1 );
    row++;

    myRestHCloseEdgeEnable = new QCheckBox( tr("NETGEN_RESTH_CLOSE_EDGE"), sizeGroup );
    sizeLayout->addWidget( myRestHCloseEdgeEnable, row, 0 );
    myRestHCloseEdgeFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHCloseEdgeFactor->RangeStepAndValidator( 0.2, 8., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHCloseEdgeFactor, row, 1 );
    row++;

    myRestHSurfCurvEnable = new QCheckBox( tr("NETGEN_RESTH_SURF_CURV"), sizeGroup );
    sizeLayout->addWidget( myRestHSurfCurvEnable, row, 0 );
    myRestHSurfCurvFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHSurfCurvFactor->RangeStepAndValidator( 0.2, 5., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHSurfCurvFactor, row, 1 );
    row++;

    myRestHEdgeAngleEnable = new QCheckBox( tr("NETGEN_RESTH_EDGE_ANGLE"), sizeGroup );
    sizeLayout->addWidget( myRestHEdgeAngleEnable, row, 0 );
    myRestHEdgeAngleFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHEdgeAngleFactor->RangeStepAndValidator( 0.2, 5., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHEdgeAngleFactor, row, 1 );
    row++;

    myRestHSurfMeshCurvEnable = new QCheckBox( tr("NETGEN_RESTH_SURF_MESH_CURV"), sizeGroup );
    sizeLayout->addWidget( myRestHSurfMeshCurvEnable, row, 0 );
    myRestHSurfMeshCurvFactor = new SMESHGUI_SpinBox( sizeGroup );
    myRestHSurfMeshCurvFactor->RangeStepAndValidator( 0.2, 5., 0.1, "length_precision" );
    sizeLayout->addWidget( myRestHSurfMeshCurvFactor, row, 1 );
    row++;
  }
  if ( isRemesher )
  {
    tab->insertTab( STL_TAB, stlGroup, tr( "NETGEN_STL" ));
    connect( myRestHChartDistEnable   , SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
    connect( myRestHLineLengthEnable  , SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
    connect( myRestHCloseEdgeEnable   , SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
    connect( myRestHSurfCurvEnable    , SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
    connect( myRestHEdgeAngleEnable   , SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
    connect( myRestHSurfMeshCurvEnable, SIGNAL( toggled(bool) ), this, SLOT( onSTLEnable() ));
  }
  else
  {
    delete stlGroup;
    myRidgeAngle = 0;
  }

  // ===============
  // Local Size TAB
  // ===============

  myLocalSizeTable = 0;
  //if ( !myIsONLY )
  {
    QWidget* localSizeGroup = new QWidget();
    QGridLayout* localSizeLayout = new QGridLayout(localSizeGroup);

    myLocalSizeTable = new QTableWidget(0, LSZ_NB_COLUMNS, localSizeGroup);
    localSizeLayout->addWidget(myLocalSizeTable, 1, 0, 8, 2);
    QStringList localSizeHeaders;
    localSizeHeaders << tr( "LSZ_ENTRY_COLUMN" )<< tr( "LSZ_NAME_COLUMN" ) << tr( "LSZ_LOCALSIZE_COLUMN" );
    myLocalSizeTable->setHorizontalHeaderLabels(localSizeHeaders);
    myLocalSizeTable->horizontalHeader()->hideSection(LSZ_ENTRY_COLUMN);
    myLocalSizeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    myLocalSizeTable->resizeColumnToContents(LSZ_NAME_COLUMN);
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
    myLocalSizeTable->setAlternatingRowColors(true);
    myLocalSizeTable->verticalHeader()->hide();

    QPushButton* addVertexButton = new QPushButton(tr("NETGEN_LSZ_VERTEX"), localSizeGroup);
    localSizeLayout->addWidget(addVertexButton, LSZ_VERTEX_BTN, 2, 1, 1);
    QPushButton* addEdgeButton = new QPushButton(tr("NETGEN_LSZ_EDGE"), localSizeGroup);
    localSizeLayout->addWidget(addEdgeButton, LSZ_EDGE_BTN, 2, 1, 1);
    QPushButton* addFaceButton = new QPushButton(tr("NETGEN_LSZ_FACE"), localSizeGroup);
    localSizeLayout->addWidget(addFaceButton, LSZ_FACE_BTN, 2, 1, 1);
    QPushButton* addSolidButton = new QPushButton(tr("NETGEN_LSZ_SOLID"), localSizeGroup);
    localSizeLayout->addWidget(addSolidButton, LSZ_SOLID_BTN, 2, 1, 1);

    QFrame *line2 = new QFrame(localSizeGroup);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    localSizeLayout->addWidget(line2, LSZ_SEPARATOR2, 2, 1, 1);

    QPushButton* removeButton = new QPushButton(tr("NETGEN_LSZ_REMOVE"), localSizeGroup);
    localSizeLayout->addWidget(removeButton, LSZ_REMOVE_BTN, 2, 1, 1);

    QPushButton* fileBtn = new QPushButton(tr("NETGEN_LSZ_FILE"), localSizeGroup);
    myMeshSizeFile = new QLineEdit(localSizeGroup);
    myMeshSizeFile->setReadOnly( true );
    localSizeLayout->addWidget( fileBtn, LSZ_FILE_LE, 0, 1, 1);
    localSizeLayout->addWidget( myMeshSizeFile, LSZ_FILE_LE, 1, 1, 2);

    connect( addVertexButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnVertex()));
    connect( addEdgeButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnEdge()));
    connect( addFaceButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnFace()));
    connect( addSolidButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnSolid()));
    connect( removeButton, SIGNAL(clicked()), this, SLOT(onRemoveLocalSizeOnShape()));
    connect( myLocalSizeTable, SIGNAL(cellChanged(int, int)), this, SLOT(onSetLocalSize(int, int)));
    connect( fileBtn, SIGNAL(clicked()), this, SLOT(onSetSizeFile()));

    tab->insertTab(LSZ_TAB, localSizeGroup, tr("NETGEN_LOCAL_SIZE"));
  }

  // =============
  // Advanced TAB
  // =============

  QWidget* advGroup = new QWidget();
  tab->insertTab( ADV_TAB, advGroup, tr( "SMESH_ADVANCED" ));
  QVBoxLayout* advLay = new QVBoxLayout( advGroup );
  advLay->setSpacing( 6 );
  advLay->setMargin( 5 );

  // Optimizer group
  // ----------------
  {
    QGroupBox* optBox = new QGroupBox( tr("NETGEN_OPTIMIZER"), advGroup );
    advLay->addWidget( optBox );

    QGridLayout* optLayout = new QGridLayout( optBox );
    optLayout->setMargin( 6 );
    optLayout->setSpacing( 6 );

    int row = 0;
    myElemSizeWeight = 0;
    myNbSurfOptSteps = 0;
    if ( myIs2D || !myIsONLY ) // 2D options
    {
      optLayout->addWidget( new QLabel( tr( "NETGEN_ELEM_SIZE_WEIGHT" ), optBox ), row, 0 );
      myElemSizeWeight = new SMESHGUI_SpinBox( optBox );
      myElemSizeWeight->RangeStepAndValidator( 0., 1., 0.1, "parametric_precision" );
      optLayout->addWidget( myElemSizeWeight, row, 1 );
      row++;

      optLayout->addWidget( new QLabel( tr( "NETGEN_NB_SURF_OPT_STEPS" ), optBox ), row, 0 );
      myNbSurfOptSteps = new SalomeApp_IntSpinBox( optBox );
      myNbSurfOptSteps->setMinimum( 0 );
      myNbSurfOptSteps->setMaximum( 99 );
      optLayout->addWidget( myNbSurfOptSteps, row, 1 );
      row++;
    }

    myNbVolOptSteps = 0;
    if ( !myIs2D )
    {
      optLayout->addWidget( new QLabel( tr( "NETGEN_NB_VOL_OPT_STEPS" ), optBox ), row, 0 );
      myNbVolOptSteps = new SalomeApp_IntSpinBox( optBox );
      myNbVolOptSteps->setMinimum( 0 );
      myNbVolOptSteps->setMaximum( 99 );
      optLayout->addWidget( myNbVolOptSteps, row, 1 );
    }
  }
  // Insider group
  {
    QGroupBox* insGroup = new QGroupBox( tr("NETGEN_INSIDER"), advGroup );
    advLay->addWidget( insGroup );

    QGridLayout* insLayout = new QGridLayout( insGroup );
    insLayout->setMargin( 6 );
    insLayout->setSpacing( 6 );

    int row = 0;
    myWorstElemMeasure = 0;
    myUseDelauney = 0;
    if ( !myIs2D )
    {
      insLayout->addWidget( new QLabel( tr( "NETGEN_WORST_ELEM_MEASURE" ), insGroup ), row, 0 );
      myWorstElemMeasure = new SalomeApp_IntSpinBox( insGroup );
      myWorstElemMeasure->setMinimum( 1 );
      myWorstElemMeasure->setMaximum( 10 );
      insLayout->addWidget( myWorstElemMeasure, row, 1, 1, 2 );
      row++;

      myUseDelauney = new QCheckBox( tr( "NETGEN_USE_DELAUNEY" ), insGroup );
      insLayout->addWidget( myUseDelauney, row, 0, 1, 2 );
      row++;
    }

    myCheckOverlapping = 0;
    if ( myIs2D || !myIsONLY ) // 2D options
    {
      myCheckOverlapping = new QCheckBox( tr( "NETGEN_CHECK_OVERLAPPING" ), insGroup );
      insLayout->addWidget( myCheckOverlapping, row, 0, 1, 2 );
      row++;
    }

    myCheckChartBoundary = 0;
    if ( isRemesher )
    {
      myCheckChartBoundary = new QCheckBox( tr( "NETGEN_CHECK_CHART_BOUNDARY" ), insGroup );
      insLayout->addWidget( myCheckChartBoundary, row, 0, 1, 2 );
      row++;
    }

    myFuseEdges = 0;
    if ( !myIsONLY && !isRemesher )
    {
      myFuseEdges = new QCheckBox( tr( "NETGEN_FUSE_EDGES" ), insGroup );
      insLayout->addWidget( myFuseEdges, row, 0, 1, 2 );
      row++;
    }
    insLayout->setRowStretch( row, 1 );
  }

  return fr;
}

void NETGENPluginGUI_HypothesisCreator::retrieveParams() const
{
  NetgenHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );

  setTextOrVar( myMaxSize, data.myMaxSize, data.myMaxSizeVar );
  setTextOrVar( myMinSize, data.myMinSize, data.myMinSizeVar );
  if ( mySecondOrder )
    mySecondOrder->setChecked( data.mySecondOrder );
  if ( myOptimize )
    myOptimize->setChecked( data.myOptimize );
  myFineness->setCurrentIndex( data.myFineness );
  setTextOrVar( myGrowthRate, data.myGrowthRate, data.myGrowthRateVar );
  setTextOrVar( myNbSegPerEdge, data.myNbSegPerEdge, data.myNbSegPerEdgeVar );
  setTextOrVar( myNbSegPerRadius, data.myNbSegPerRadius, data.myNbSegPerRadiusVar );

  if ( myChordalError )
  {
    myChordalErrorEnabled->setChecked( data.myChordalErrorEnabled && data.myChordalError > 0 );
    setTextOrVar( myChordalError, data.myChordalError, data.myChordalErrorVar );
    myChordalError->setEnabled( myChordalErrorEnabled->isChecked() );
  }
  if (myAllowQuadrangles)
    myAllowQuadrangles->setChecked( data.myAllowQuadrangles );
  if (mySurfaceCurvature)
    mySurfaceCurvature->setChecked( data.mySurfaceCurvature );

  if ( myKeepExistingEdges )
  {
    myKeepExistingEdges->setChecked( data.myKeepExistingEdges );
    myMakeGroupsOfSurfaces->setChecked( data.myMakeGroupsOfSurfaces );
  }

  setTextOrVar( myElemSizeWeight, data.myElemSizeWeight, data.myElemSizeWeightVar );
  setTextOrVar( myNbSurfOptSteps, data.myNbSurfOptSteps, data.myNbSurfOptStepsVar );
  setTextOrVar( myNbVolOptSteps,  data.myNbVolOptSteps,  data.myNbVolOptStepsVar );

  if (myFuseEdges)
    myFuseEdges->setChecked( data.myFuseEdges );
  setTextOrVar( myWorstElemMeasure, data.myWorstElemMeasure, data.myWorstElemMeasureVar );
  if ( myUseDelauney )
    myUseDelauney->setChecked( data.myUseDelauney );
  if ( myCheckOverlapping )
    myCheckOverlapping->setChecked( data.myCheckOverlapping );
  if ( myCheckChartBoundary )
    myCheckChartBoundary->setChecked( data.myCheckChartBoundary );

  if ( myRidgeAngle )
  {
    setTextOrVar( myRidgeAngle, data.myRidgeAngle, data.myRidgeAngleVar );
    setTextOrVar( myEdgeCornerAngle, data.myEdgeCornerAngle, data.myEdgeCornerAngleVar );
    setTextOrVar( myChartAngle, data.myChartAngle, data.myChartAngleVar );
    setTextOrVar( myOuterChartAngle, data.myOuterChartAngle, data.myOuterChartAngleVar );
    setTextOrVar( myRestHChartDistFactor, data.myRestHChartDistFactor,
                  data.myRestHChartDistFactorVar );
    setTextOrVar( myRestHLineLengthFactor, data.myRestHLineLengthFactor,
                  data.myRestHLineLengthFactorVar );
    setTextOrVar( myRestHCloseEdgeFactor, data.myRestHCloseEdgeFactor,
                  data.myRestHCloseEdgeFactorVar );
    setTextOrVar( myRestHSurfCurvFactor, data.myRestHSurfCurvFactor,
                  data.myRestHSurfCurvFactorVar );
    setTextOrVar( myRestHEdgeAngleFactor, data.myRestHEdgeAngleFactor,
                  data.myRestHEdgeAngleFactorVar );
    setTextOrVar( myRestHSurfMeshCurvFactor, data.myRestHSurfMeshCurvFactor,
                  data.myRestHSurfMeshCurvFactorVar );

    myRestHChartDistEnable->setChecked( data.myRestHChartDistEnable );
    myRestHLineLengthEnable->setChecked( data.myRestHLineLengthEnable );
    myRestHCloseEdgeEnable->setChecked( data.myRestHCloseEdgeEnable );
    myRestHSurfCurvEnable->setChecked( data.myRestHSurfCurvEnable );
    myRestHEdgeAngleEnable->setChecked( data.myRestHEdgeAngleEnable );
    myRestHSurfMeshCurvEnable->setChecked( data.myRestHSurfMeshCurvEnable );
  }

  // update widgets
  ((NETGENPluginGUI_HypothesisCreator*) this )-> onSurfaceCurvatureChanged();

  if ( myLocalSizeTable )
  {
    NETGENPluginGUI_HypothesisCreator* that = (NETGENPluginGUI_HypothesisCreator*)this;
    QMapIterator<QString, QString> i(myLocalSizeMap);
    GeomSelectionTools* geomSelectionTools = that->getGeomSelectionTools();
    while (i.hasNext()) {
      i.next();
      const QString entry = i.key();
      std::string shapeName = geomSelectionTools->getNameFromEntry(entry.toStdString());
      const QString localSize = i.value();
      int row = myLocalSizeTable->rowCount();
      myLocalSizeTable->setRowCount(row+1);
      myLocalSizeTable->setItem(row, LSZ_ENTRY_COLUMN, new QTableWidgetItem(entry));
      myLocalSizeTable->item(row, LSZ_ENTRY_COLUMN)->setFlags(0);
      myLocalSizeTable->setItem(row, LSZ_NAME_COLUMN, new QTableWidgetItem(QString::fromStdString(shapeName)));
      myLocalSizeTable->item(row, LSZ_NAME_COLUMN)->setFlags(0);
      myLocalSizeTable->setItem(row, LSZ_LOCALSIZE_COLUMN, new QTableWidgetItem(localSize));
      myLocalSizeTable->item(row, LSZ_LOCALSIZE_COLUMN)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
    }
    myLocalSizeTable->resizeColumnToContents(LSZ_NAME_COLUMN);
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);

    myMeshSizeFile->setText( data.myMeshSizeFile );
  }
}

QString NETGENPluginGUI_HypothesisCreator::storeParams() const
{
  NetgenHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );

  return QString();
}

bool NETGENPluginGUI_HypothesisCreator::readParamsFromHypo( NetgenHypothesisData& h_data ) const
{
  NETGENPlugin::NETGENPlugin_Hypothesis_var h =
    NETGENPlugin::NETGENPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  h_data.myName = isCreation() ? hypName() : "";

  h_data.myMaxSize             = h->GetMaxSize();
  h_data.myMaxSizeVar          = getVariableName("SetMaxSize");
  h_data.myMinSize             = h->GetMinSize();
  h_data.myMinSizeVar          = getVariableName("SetMinSize");
  h_data.mySecondOrder         = h->GetSecondOrder();
  h_data.myOptimize            = h->GetOptimize();

  h_data.myFineness            = (int) h->GetFineness();
  h_data.myGrowthRate          = h->GetGrowthRate();
  h_data.myGrowthRateVar       = getVariableName("SetGrowthRate");
  h_data.myNbSegPerEdge        = h->GetNbSegPerEdge();
  h_data.myNbSegPerEdgeVar     = getVariableName("SetNbSegPerEdge");
  h_data.myNbSegPerRadius      = h->GetNbSegPerRadius();
  h_data.myNbSegPerRadiusVar   = getVariableName("SetNbSegPerRadius");
  h_data.myChordalError        = h->GetChordalError();
  h_data.myChordalErrorVar     = getVariableName("SetChordalError");
  h_data.myChordalErrorEnabled = h->GetChordalErrorEnabled();
  h_data.mySurfaceCurvature    = h->GetUseSurfaceCurvature();

  h_data.myElemSizeWeight      = h->GetElemSizeWeight    ();
  h_data.myElemSizeWeightVar   = getVariableName("SetElemSizeWeight");
  h_data.myNbSurfOptSteps      = h->GetNbSurfOptSteps    ();
  h_data.myNbSurfOptStepsVar   = getVariableName("SetNbSurfOptSteps");
  h_data.myNbVolOptSteps       = h->GetNbVolOptSteps     ();
  h_data.myNbVolOptStepsVar    = getVariableName("SetNbVolOptSteps");
  h_data.myFuseEdges           = h->GetFuseEdges();
  h_data.myWorstElemMeasure    = h->GetWorstElemMeasure  ();
  h_data.myWorstElemMeasureVar = getVariableName("SetWorstElemMeasure");
  h_data.myUseDelauney         = h->GetUseDelauney       ();
  h_data.myCheckOverlapping    = h->GetCheckOverlapping  ();
  h_data.myCheckChartBoundary  = h->GetCheckChartBoundary();

  h_data.myMeshSizeFile        = h->GetMeshSizeFile();

  //if ( myIs2D )
  {
    // NETGENPlugin::NETGENPlugin_Hypothesis_var h =
    //   NETGENPlugin::NETGENPlugin_Hypothesis::_narrow( initParamsHypothesis() );

    if ( !h->_is_nil() )
      h_data.myAllowQuadrangles = h->GetQuadAllowed();
  }
  if ( myIs2D )
  {
    NETGENPlugin::NETGENPlugin_RemesherHypothesis_2D_var rh =
      NETGENPlugin::NETGENPlugin_RemesherHypothesis_2D::_narrow( h );
    if ( !rh->_is_nil() )
    {
      h_data.myRidgeAngle                 = rh->GetRidgeAngle();
      h_data.myRidgeAngleVar              = getVariableName("SetRidgeAngle");
      h_data.myEdgeCornerAngle            = rh->GetEdgeCornerAngle        ();
      h_data.myEdgeCornerAngleVar         = getVariableName("SetEdgeCornerAngle");
      h_data.myChartAngle                 = rh->GetChartAngle             ();
      h_data.myChartAngleVar              = getVariableName("SetChartAngle");
      h_data.myOuterChartAngle            = rh->GetOuterChartAngle        ();
      h_data.myOuterChartAngleVar         = getVariableName("SetOuterChartAngle");
      h_data.myRestHChartDistFactor       = rh->GetRestHChartDistFactor   ();
      h_data.myRestHChartDistFactorVar    = getVariableName("SetRestHChartDistFactor");
      h_data.myRestHLineLengthFactor      = rh->GetRestHLineLengthFactor  ();
      h_data.myRestHLineLengthFactorVar   = getVariableName("SetRestHLineLengthFactor");
      h_data.myRestHCloseEdgeFactor       = rh->GetRestHCloseEdgeFactor   ();
      h_data.myRestHCloseEdgeFactorVar    = getVariableName("SetRestHCloseEdgeFactor");
      h_data.myRestHSurfCurvFactor        = rh->GetRestHSurfCurvFactor    ();
      h_data.myRestHSurfCurvFactorVar     = getVariableName("SetRestHSurfCurvFactor");
      h_data.myRestHEdgeAngleFactor       = rh->GetRestHEdgeAngleFactor   ();
      h_data.myRestHEdgeAngleFactorVar    = getVariableName("SetRestHEdgeAngleFactor");
      h_data.myRestHSurfMeshCurvFactor    = rh->GetRestHSurfMeshCurvFactor();
      h_data.myRestHSurfMeshCurvFactorVar = getVariableName("SetRestHSurfMeshCurvFactor");
      h_data.myRestHChartDistEnable       = rh->GetRestHChartDistEnable   ();
      h_data.myRestHLineLengthEnable      = rh->GetRestHLineLengthEnable  ();
      h_data.myRestHCloseEdgeEnable       = rh->GetRestHCloseEdgeEnable   ();
      h_data.myRestHSurfCurvEnable        = rh->GetRestHSurfCurvEnable    ();
      h_data.myRestHEdgeAngleEnable       = rh->GetRestHEdgeAngleEnable   ();
      h_data.myRestHSurfMeshCurvEnable    = rh->GetRestHSurfMeshCurvEnable();
      h_data.myKeepExistingEdges          = rh->GetKeepExistingEdges      ();
      h_data.myMakeGroupsOfSurfaces       = rh->GetMakeGroupsOfSurfaces   ();
    }
  }

  NETGENPluginGUI_HypothesisCreator*  that = (NETGENPluginGUI_HypothesisCreator*)this;
  NETGENPlugin::string_array_var myEntries = h->GetLocalSizeEntries();
  for ( CORBA::ULong i = 0; i < myEntries->length(); i++ )
  {
    QString entry = myEntries[i].in();
    if (myLocalSizeMap.contains(entry) &&
        myLocalSizeMap[entry] == "__TO_DELETE__")
      continue;
    double val = h->GetLocalSizeOnEntry( myEntries[i] );
    std::ostringstream tmp;
    tmp << val;
    that->myLocalSizeMap[entry] = tmp.str().c_str();
  }

  return true;
}

bool NETGENPluginGUI_HypothesisCreator::storeParamsToHypo( const NetgenHypothesisData& h_data ) const
{
  NETGENPlugin::NETGENPlugin_Hypothesis_var h =
    NETGENPlugin::NETGENPlugin_Hypothesis::_narrow( hypothesis() );

  bool ok = true;
  try
  {
    if ( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().data() );
    h->SetVarParameter( h_data.myMaxSizeVar.toLatin1().constData(), "SetMaxSize");
    h->SetMaxSize     ( h_data.myMaxSize );
    h->SetVarParameter( h_data.myMinSizeVar.toLatin1().constData(), "SetMinSize");
    h->SetMinSize     ( h_data.myMinSize );
    if ( mySecondOrder )
      h->SetSecondOrder ( h_data.mySecondOrder );
    if ( myOptimize )
      h->SetOptimize    ( h_data.myOptimize );
    h->SetFineness    ( h_data.myFineness );

    if ( h_data.myFineness == UserDefined )
    {
      h->SetVarParameter  ( h_data.myGrowthRateVar.toLatin1().constData(), "SetGrowthRate");
      h->SetGrowthRate    ( h_data.myGrowthRate );
      if ( myNbSegPerEdge )
      {
        h->SetVarParameter  ( h_data.myNbSegPerEdgeVar.toLatin1().constData(), "SetNbSegPerEdge");
        h->SetNbSegPerEdge  ( h_data.myNbSegPerEdge );
      }
      if ( myNbSegPerRadius )
      {
        h->SetVarParameter  ( h_data.myNbSegPerRadiusVar.toLatin1().constData(), "SetNbSegPerRadius");
        h->SetNbSegPerRadius( h_data.myNbSegPerRadius );
      }
    }
    if ( myChordalError )
    {
      h->SetVarParameter       ( h_data.myChordalErrorVar.toLatin1().constData(), "SetChordalError");
      h->SetChordalError       ( h_data.myChordalError );
      h->SetChordalErrorEnabled( h_data.myChordalErrorEnabled );
    }
    if ( mySurfaceCurvature )
      h->SetUseSurfaceCurvature( h_data.mySurfaceCurvature );
    h->SetMeshSizeFile         ( h_data.myMeshSizeFile.toUtf8().constData() );

    h->SetVarParameter  ( h_data.myElemSizeWeightVar.toLatin1().constData(), "SetElemSizeWeight");
    h->SetElemSizeWeight( h_data.myElemSizeWeight );
    if ( myNbSurfOptSteps )
    {
      h->SetVarParameter  ( h_data.myNbSurfOptStepsVar.toLatin1().constData(), "SetNbSurfOptSteps");
      h->SetNbSurfOptSteps((CORBA::Short) h_data.myNbSurfOptSteps );
    }
    if ( myNbVolOptSteps )
    {
      h->SetVarParameter ( h_data.myNbVolOptStepsVar.toLatin1().constData(), "SetNbVolOptSteps");
      h->SetNbVolOptSteps((CORBA::Short) h_data.myNbVolOptSteps );
    }
    if ( myFuseEdges )
      h->SetFuseEdges( h_data.myFuseEdges );
    h->SetVarParameter    ( h_data.myWorstElemMeasureVar.toLatin1().constData(), "SetWorstElemMeasure");
    h->SetWorstElemMeasure((CORBA::Short) h_data.myWorstElemMeasure );

    h->SetUseDelauney( h_data.myUseDelauney );
    h->SetCheckOverlapping( h_data.myCheckOverlapping );
    h->SetCheckChartBoundary( h_data.myCheckChartBoundary );
    
    //if ( myIs2D )
    {
      // NETGENPlugin::NETGENPlugin_Hypothesis_2D_var h_2d =
      //   NETGENPlugin::NETGENPlugin_Hypothesis_2D::_narrow( h );
      // if ( !h_2d->_is_nil() )
      //   h_2d->SetQuadAllowed( h_data.myAllowQuadrangles );
      if ( myAllowQuadrangles )
        h->SetQuadAllowed( h_data.myAllowQuadrangles );
    }
    if ( myIs2D )
    {
      NETGENPlugin::NETGENPlugin_RemesherHypothesis_2D_var rh =
        NETGENPlugin::NETGENPlugin_RemesherHypothesis_2D::_narrow( h );
      if ( !rh->_is_nil() )
      {
        rh->SetVarParameter   ( h_data.myRidgeAngleVar.toLatin1().constData(), "SetRidgeAngle");
        rh->SetRidgeAngle     ( h_data.myRidgeAngle );
        rh->SetVarParameter   ( h_data.myEdgeCornerAngleVar.toLatin1().constData(), "SetEdgeCornerAngle");
        rh->SetEdgeCornerAngle( h_data.myEdgeCornerAngle );
        rh->SetVarParameter   ( h_data.myChartAngleVar.toLatin1().constData(), "SetChartAngle");
        rh->SetChartAngle     ( h_data.myChartAngle );
        rh->SetVarParameter   ( h_data.myOuterChartAngleVar.toLatin1().constData(), "SetOuterChartAngle");
        rh->SetOuterChartAngle( h_data.myOuterChartAngle );

        rh->SetVarParameter           ( h_data.myRestHChartDistFactorVar.toLatin1().constData(), "SetRestHChartDistFactor");
        rh->SetRestHChartDistFactor   ( h_data.myRestHChartDistFactor );
        rh->SetVarParameter           ( h_data.myRestHLineLengthFactorVar.toLatin1().constData(), "SetRestHLineLengthFactor");
        rh->SetRestHLineLengthFactor  ( h_data.myRestHLineLengthFactor );
        rh->SetVarParameter           ( h_data.myRestHCloseEdgeFactorVar.toLatin1().constData(), "SetRestHCloseEdgeFactor");
        rh->SetRestHCloseEdgeFactor   ( h_data.myRestHCloseEdgeFactor );
        rh->SetVarParameter           ( h_data.myRestHSurfCurvFactorVar.toLatin1().constData(), "SetRestHSurfCurvFactor");
        rh->SetRestHSurfCurvFactor    ( h_data.myRestHSurfCurvFactor );
        rh->SetVarParameter           ( h_data.myRestHEdgeAngleFactorVar.toLatin1().constData(), "SetRestHEdgeAngleFactor");
        rh->SetRestHEdgeAngleFactor   ( h_data.myRestHEdgeAngleFactor );
        rh->SetVarParameter           ( h_data.myRestHSurfMeshCurvFactorVar.toLatin1().constData(), "SetRestHSurfMeshCurvFactor");
        rh->SetRestHSurfMeshCurvFactor( h_data.myRestHSurfMeshCurvFactor );

        rh->SetRestHChartDistEnable   ( h_data.myRestHChartDistEnable );
        rh->SetRestHLineLengthEnable  ( h_data.myRestHLineLengthEnable );
        rh->SetRestHCloseEdgeEnable   ( h_data.myRestHCloseEdgeEnable );
        rh->SetRestHSurfCurvEnable    ( h_data.myRestHSurfCurvEnable );
        rh->SetRestHEdgeAngleEnable   ( h_data.myRestHEdgeAngleEnable );
        rh->SetRestHSurfMeshCurvEnable( h_data.myRestHSurfMeshCurvEnable );

        rh->SetKeepExistingEdges      ( h_data.myKeepExistingEdges );
        rh->SetMakeGroupsOfSurfaces   ( h_data.myMakeGroupsOfSurfaces );
        //rh->SetFixedEdgeGroup         ( -1 );
      }
    }
    for ( QMapIterator<QString,QString> i(myLocalSizeMap); i.hasNext(); )
    {
      i.next();
      const QString&     entry = i.key();
      const QString& localSize = i.value();
      if (localSize == "__TO_DELETE__")
      {
        h->UnsetLocalSizeOnEntry(entry.toLatin1().constData());
      }
      else
      {
        h->SetLocalSizeOnEntry(entry.toLatin1().constData(), localSize.toDouble());
      }
    }
  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
    ok = false;
  }
  return ok;
}

bool NETGENPluginGUI_HypothesisCreator::readParamsFromWidgets( NetgenHypothesisData& h_data ) const
{
  h_data.myName           = myName ? myName->text() : "";
  h_data.myMaxSize        = myMaxSize->value();
  h_data.myMaxSizeVar     = myMaxSize->text();
  h_data.myMinSize        = myMinSize->value();
  h_data.myMinSizeVar     = myMinSize->text();
  if ( mySecondOrder )
    h_data.mySecondOrder  = mySecondOrder->isChecked();
  if ( myOptimize )
    h_data.myOptimize     = myOptimize->isChecked();
  h_data.myFineness       = myFineness->currentIndex();
  h_data.myGrowthRate     = myGrowthRate->value();
  if ( myNbSegPerEdge )
    h_data.myNbSegPerEdge = myNbSegPerEdge->value();
  if ( myNbSegPerRadius )
    h_data.myNbSegPerRadius = myNbSegPerRadius->value();

  h_data.myGrowthRateVar  = myGrowthRate->text();
  if ( myNbSegPerEdge )
    h_data.myNbSegPerEdgeVar = myNbSegPerEdge->text();
  if ( myNbSegPerRadius )
    h_data.myNbSegPerRadiusVar = myNbSegPerRadius->text();
  if ( myChordalError )
  {
    h_data.myChordalErrorVar = myChordalError->text();
    h_data.myChordalError    = myChordalError->value();
    h_data.myChordalErrorEnabled = myChordalError->isEnabled();
  }
  if ( myAllowQuadrangles )
    h_data.myAllowQuadrangles = myAllowQuadrangles->isChecked();

  if ( mySurfaceCurvature )
    h_data.mySurfaceCurvature = mySurfaceCurvature->isChecked();

  if ( myFuseEdges )
    h_data.myFuseEdges = myFuseEdges->isChecked();

  if ( myElemSizeWeight )
  {
    h_data.myElemSizeWeight    = myElemSizeWeight->value();
    h_data.myElemSizeWeightVar = myElemSizeWeight->text();
  }
  if ( myNbSurfOptSteps )
  {
    h_data.myNbSurfOptSteps    = myNbSurfOptSteps->value();
    h_data.myNbSurfOptStepsVar = myNbSurfOptSteps->text();
  }
  if ( myNbVolOptSteps )
  {
    h_data.myNbVolOptSteps    = myNbVolOptSteps->value();
    h_data.myNbVolOptStepsVar = myNbVolOptSteps->text();
  }
  if ( myWorstElemMeasure )
  {
    h_data.myWorstElemMeasure    = myWorstElemMeasure->value();
    h_data.myWorstElemMeasureVar = myWorstElemMeasure->text();
  }
  if ( myUseDelauney )
    h_data.myUseDelauney        = myUseDelauney->isChecked();

  if ( myCheckOverlapping )
    h_data.myCheckOverlapping   = myCheckOverlapping->isChecked();

  if ( myCheckChartBoundary )
    h_data.myCheckChartBoundary = myCheckChartBoundary->isChecked();

  if ( myRidgeAngle )
  {
    h_data.myRidgeAngle                 = myRidgeAngle             ->value();
    h_data.myRidgeAngleVar              = myRidgeAngle             ->text();
    h_data.myEdgeCornerAngle            = myEdgeCornerAngle        ->value();
    h_data.myEdgeCornerAngleVar         = myEdgeCornerAngle        ->text();
    h_data.myChartAngle                 = myChartAngle             ->value();
    h_data.myChartAngleVar              = myChartAngle             ->text();
    h_data.myOuterChartAngle            = myOuterChartAngle        ->value();
    h_data.myOuterChartAngleVar         = myOuterChartAngle        ->text();
    h_data.myRestHChartDistFactor       = myRestHChartDistFactor   ->value();
    h_data.myRestHChartDistFactorVar    = myRestHChartDistFactor   ->text();
    h_data.myRestHLineLengthFactor      = myRestHLineLengthFactor  ->value();
    h_data.myRestHLineLengthFactorVar   = myRestHLineLengthFactor  ->text();
    h_data.myRestHCloseEdgeFactor       = myRestHCloseEdgeFactor   ->value();
    h_data.myRestHCloseEdgeFactorVar    = myRestHCloseEdgeFactor   ->text();
    h_data.myRestHSurfCurvFactor        = myRestHSurfCurvFactor    ->value();
    h_data.myRestHSurfCurvFactorVar     = myRestHSurfCurvFactor    ->text();
    h_data.myRestHEdgeAngleFactor       = myRestHEdgeAngleFactor   ->value();
    h_data.myRestHEdgeAngleFactorVar    = myRestHEdgeAngleFactor   ->text();
    h_data.myRestHSurfMeshCurvFactor    = myRestHSurfMeshCurvFactor->value();
    h_data.myRestHSurfMeshCurvFactorVar = myRestHSurfMeshCurvFactor->text();
    h_data.myRestHChartDistEnable       = myRestHChartDistEnable   ->isChecked();
    h_data.myRestHLineLengthEnable      = myRestHLineLengthEnable  ->isChecked();
    h_data.myRestHCloseEdgeEnable       = myRestHCloseEdgeEnable   ->isChecked();
    h_data.myRestHSurfCurvEnable        = myRestHSurfCurvEnable    ->isChecked();
    h_data.myRestHEdgeAngleEnable       = myRestHEdgeAngleEnable   ->isChecked();
    h_data.myRestHSurfMeshCurvEnable    = myRestHSurfMeshCurvEnable->isChecked();
    h_data.myKeepExistingEdges          = myKeepExistingEdges      ->isChecked();
    h_data.myMakeGroupsOfSurfaces       = myMakeGroupsOfSurfaces   ->isChecked();
  }

  if ( myLocalSizeTable )
  {
    NETGENPluginGUI_HypothesisCreator* that = (NETGENPluginGUI_HypothesisCreator*)this;
    int nbRows = myLocalSizeTable->rowCount();
    for(int row=0 ; row < nbRows ; row++)
    {
      QString entry = myLocalSizeTable->item(row, LSZ_ENTRY_COLUMN)->text();
      QString localSize = myLocalSizeTable->item(row, LSZ_LOCALSIZE_COLUMN)->text().trimmed();
      that->myLocalSizeMap[entry] = localSize;
    }
    h_data.myMeshSizeFile = myMeshSizeFile->text();
  }
  return true;
}

void NETGENPluginGUI_HypothesisCreator::onChordalErrorEnabled()
{
  myChordalError->setEnabled( myChordalErrorEnabled->isChecked() );
}

void NETGENPluginGUI_HypothesisCreator::onSurfaceCurvatureChanged()
{
  bool isSurfaceCurvature = (mySurfaceCurvature ? mySurfaceCurvature->isChecked() : true);
  bool isCustom           = (myFineness->currentIndex() == UserDefined);
  //myFineness->setEnabled(isSurfaceCurvature);
  myGrowthRate->setEnabled(isCustom);
  if ( myNbSegPerEdge )
    myNbSegPerEdge->setEnabled(isCustom && isSurfaceCurvature);
  if ( myNbSegPerRadius )
    myNbSegPerRadius->setEnabled(isCustom && isSurfaceCurvature);
  // if ( myChordalError )
  // {
  //   myChordalError->setEnabled( isSurfaceCurvature );
  //   myChordalErrorEnabled->setEnabled( isSurfaceCurvature );
  // }
}

void NETGENPluginGUI_HypothesisCreator::onFinenessChanged()
{
  bool isCustom = (myFineness->currentIndex() == UserDefined);

  myGrowthRate->setEnabled(isCustom);
  if ( myNbSegPerEdge )
    myNbSegPerEdge->setEnabled(isCustom);
  if ( myNbSegPerRadius )
    myNbSegPerRadius->setEnabled(isCustom);

  if (!isCustom)
  {
    double aGrowthRate, aNbSegPerEdge, aNbSegPerRadius;

    switch ( myFineness->currentIndex() )
    {
    case VeryCoarse:
      aGrowthRate     = 0.7;
      aNbSegPerEdge   = 0.3;
      aNbSegPerRadius = 1;
      break;
    case Coarse:
      aGrowthRate     = 0.5;
      aNbSegPerEdge   = 0.5;
      aNbSegPerRadius = 1.5;
      break;
    case Fine:
      aGrowthRate     = 0.2;
      aNbSegPerEdge   = 2;
      aNbSegPerRadius = 3;
      break;
    case VeryFine:
      aGrowthRate     = 0.1;
      aNbSegPerEdge   = 3;
      aNbSegPerRadius = 5;
      break;
    case Moderate:
    default:
      aGrowthRate     = 0.3;
      aNbSegPerEdge   = 1;
      aNbSegPerRadius = 2;
      break;
    }

    myGrowthRate->setValue( aGrowthRate );
    if ( myNbSegPerEdge )
      myNbSegPerEdge->setValue( aNbSegPerEdge );
    if ( myNbSegPerRadius )
      myNbSegPerRadius->setValue( aNbSegPerRadius );
  }
}

void NETGENPluginGUI_HypothesisCreator::onAddLocalSizeOnVertex()
{
  addLocalSizeOnShape(TopAbs_VERTEX);
}

void NETGENPluginGUI_HypothesisCreator::onAddLocalSizeOnEdge()
{
  addLocalSizeOnShape(TopAbs_EDGE);
}

void NETGENPluginGUI_HypothesisCreator::onAddLocalSizeOnFace()
{
  addLocalSizeOnShape(TopAbs_FACE);
}

void NETGENPluginGUI_HypothesisCreator::onAddLocalSizeOnSolid()
{
  addLocalSizeOnShape(TopAbs_SOLID);
}

void NETGENPluginGUI_HypothesisCreator::addLocalSizeOnShape(TopAbs_ShapeEnum typeShapeAsked)
{
  NETGENPlugin::NETGENPlugin_Hypothesis_var h = NETGENPlugin::NETGENPlugin_Hypothesis::_narrow(initParamsHypothesis());
  GeomSelectionTools* geomSelectionTools = getGeomSelectionTools();
  LightApp_SelectionMgr* mySel = geomSelectionTools->selectionMgr();
  SALOME_ListIO ListSelectedObjects;
  mySel->selectedObjects(ListSelectedObjects, NULL, false );
  SALOME_ListIteratorOfListIO Object_It(ListSelectedObjects);
  for ( ; Object_It.More() ; Object_It.Next())
  {
    Handle(SALOME_InteractiveObject) anObject = Object_It.Value();
    std::string          entry = geomSelectionTools->getEntryOfObject(anObject);
    std::string      shapeName = anObject->getName();
    TopAbs_ShapeEnum shapeType = geomSelectionTools->entryToShapeType(entry);
    if (shapeType == TopAbs_SHAPE)
    {
      // E.A. if shapeType == TopAbs_SHAPE, it is NOT a TopoDS_Shape !!!
      continue;
    }
    // --
    if(shapeType != typeShapeAsked)
    {
      continue;
    }
    // --
    myLocalSizeTable->setFocus();
    QString shapeEntry = QString::fromStdString(entry);
    if (myLocalSizeMap.contains(shapeEntry) &&
        myLocalSizeMap[shapeEntry] != "__TO_DELETE__")
      continue;

    double phySize = h->GetMaxSize();
    std::ostringstream oss;
    oss << phySize;
    QString localSize;
    localSize  = QString::fromStdString(oss.str());
    // --
    int row = myLocalSizeTable->rowCount() ;
    myLocalSizeTable->setRowCount(row+1);
    myLocalSizeTable->setItem(row, LSZ_ENTRY_COLUMN, new QTableWidgetItem(shapeEntry));
    myLocalSizeTable->item(row, LSZ_ENTRY_COLUMN )->setFlags(0);
    myLocalSizeTable->setItem(row, LSZ_NAME_COLUMN, new QTableWidgetItem(QString::fromStdString(shapeName)));
    myLocalSizeTable->item(row, LSZ_NAME_COLUMN )->setFlags(0);
    myLocalSizeTable->setItem(row, LSZ_LOCALSIZE_COLUMN, new QTableWidgetItem(localSize));
    myLocalSizeTable->item(row, LSZ_LOCALSIZE_COLUMN )->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
    myLocalSizeTable->resizeColumnToContents(LSZ_NAME_COLUMN);
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
    myLocalSizeTable->clearSelection();
    myLocalSizeTable->scrollToItem( myLocalSizeTable->item( row, LSZ_LOCALSIZE_COLUMN ) );
    // --
  }
}

void NETGENPluginGUI_HypothesisCreator::onRemoveLocalSizeOnShape()
{
  QList<int> selectedRows;
  QList<QTableWidgetItem*> selected = myLocalSizeTable->selectedItems();
  QTableWidgetItem* item;
  int row;
  foreach(item, selected) {
    row = item->row();
    if (!selectedRows.contains(row))
      selectedRows.append( row );
  }
  qSort( selectedRows );
  QListIterator<int> it( selectedRows );
  it.toBack();
  while (it.hasPrevious())
  {
    row = it.previous();
    QString entry = myLocalSizeTable->item(row,LSZ_ENTRY_COLUMN)->text();
    if (myLocalSizeMap.contains(entry))
    {
      myLocalSizeMap[entry] = "__TO_DELETE__";
    }
    myLocalSizeTable->removeRow(row );
  }
  myLocalSizeTable->resizeColumnToContents(LSZ_NAME_COLUMN);
  myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
}

void NETGENPluginGUI_HypothesisCreator::onSetLocalSize(int row,int col)
{
  if (col == LSZ_LOCALSIZE_COLUMN) {
    QString     entry = myLocalSizeTable->item(row, LSZ_ENTRY_COLUMN)->text();
    QString localSize = myLocalSizeTable->item(row, LSZ_LOCALSIZE_COLUMN)->text().trimmed();
    myLocalSizeMap[entry] = localSize;
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
  }
}

void NETGENPluginGUI_HypothesisCreator::onSetSizeFile()
{
  QString dir = SUIT_FileDlg::getFileName( dlg(), QString(),
                                           QStringList() << tr( "ALL_FILES_FILTER" ) + "  (*)");
  myMeshSizeFile->setText( dir );
}

void NETGENPluginGUI_HypothesisCreator::onSTLEnable()
{
  myRestHChartDistFactor   ->setEnabled( myRestHChartDistEnable   ->isChecked() );
  myRestHLineLengthFactor  ->setEnabled( myRestHLineLengthEnable  ->isChecked() );
  myRestHCloseEdgeFactor   ->setEnabled( myRestHCloseEdgeEnable   ->isChecked() );
  myRestHSurfCurvFactor    ->setEnabled( myRestHSurfCurvEnable    ->isChecked() );
  myRestHEdgeAngleFactor   ->setEnabled( myRestHEdgeAngleEnable   ->isChecked() );
  myRestHSurfMeshCurvFactor->setEnabled( myRestHSurfMeshCurvEnable->isChecked() );
}

GeomSelectionTools* NETGENPluginGUI_HypothesisCreator::getGeomSelectionTools()
{
  if (myGeomSelectionTools == NULL) {
    delete myGeomSelectionTools;
    myGeomSelectionTools = new GeomSelectionTools();
  }
  return myGeomSelectionTools;
}

QString NETGENPluginGUI_HypothesisCreator::caption() const
{
  return tr( myIs2D ? "NETGEN_2D_TITLE" : "NETGEN_3D_TITLE");
}

QPixmap NETGENPluginGUI_HypothesisCreator::icon() const
{
  QString hypIconName = tr( myIs2D ?
                            "ICON_DLG_NETGEN_PARAMETERS_2D" :
                            "ICON_DLG_NETGEN_PARAMETERS");
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "NETGENPlugin", hypIconName );
}

QString NETGENPluginGUI_HypothesisCreator::type() const
{
  return tr( myIs2D ? "NETGEN_2D_HYPOTHESIS" : "NETGEN_3D_HYPOTHESIS");
}

QString NETGENPluginGUI_HypothesisCreator::helpPage() const
{
  return "netgen_2d_3d_hypo_page.html";
}
