//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  NETGENPlugin GUI: GUI for plugged-in mesher NETGENPlugin
//  File   : NETGENPluginGUI_HypothesisCreator.cxx
//  Author : Michael Zorin
//  Module : NETGENPlugin
//  $Header: 
//
#include "NETGENPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_SpinBox.h>
#include <GeomSelectionTools.h>

#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

#include <SalomeApp_Tools.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIteratorOfListIO.hxx>

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

 enum Fineness
   {
     VeryCoarse,
     Coarse,
     Moderate,
     Fine,
     VeryFine,
     UserDefined
   };

enum {
  STD_TAB = 0,
  LSZ_TAB
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
#ifdef NETGEN_NEW
  LSZ_FACE_BTN,
#endif
  LSZ_SEPARATOR2,
  LSZ_REMOVE_BTN
};

NETGENPluginGUI_HypothesisCreator::NETGENPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType )
{
  myGeomSelectionTools = NULL;
  myLocalSizeMap.clear();
  myIs2D = ( theHypType.startsWith("NETGEN_Parameters_2D"));
  myIsONLY = ( theHypType == "NETGEN_Parameters_2D_ONLY" ||
               theHypType == "NETGEN_Parameters_3D");
}

NETGENPluginGUI_HypothesisCreator::~NETGENPluginGUI_HypothesisCreator()
{
}

bool NETGENPluginGUI_HypothesisCreator::checkParams(QString& msg) const
{
  NetgenHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  storeParamsToHypo( data_old );
  
  res = myMaxSize->isValid(msg,true) && res;
  res = myGrowthRate->isValid(msg,true) && res; ;
  if ( myNbSegPerEdge )
    res = myNbSegPerEdge->isValid(msg,true) && res;
  if ( myNbSegPerRadius )
    res = myNbSegPerRadius->isValid(msg,true) && res;
  return res;
}

QFrame* NETGENPluginGUI_HypothesisCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0 );
  fr->setObjectName( "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  QTabWidget* tab = new QTabWidget( fr );
  tab->setTabShape( QTabWidget::Rounded );
  tab->setTabPosition( QTabWidget::North );
  lay->addWidget( tab );
  QWidget* GroupC1 = new QWidget();
  tab->insertTab( STD_TAB, GroupC1, tr( "SMESH_ARGUMENTS" ) );
  
  QGridLayout* aGroupLayout = new QGridLayout( GroupC1 );
  aGroupLayout->setSpacing( 6 );
  aGroupLayout->setMargin( 11 );
  
  int row = 0;
  myName = 0;
  if( isCreation() )
  {
    aGroupLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row, 0 );
    myName = new QLineEdit( GroupC1 );
    aGroupLayout->addWidget( myName, row, 1 );
    row++;
  }

  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_MAX_SIZE" ), GroupC1 ), row, 0 );
  myMaxSize = new SMESHGUI_SpinBox( GroupC1 );
  myMaxSize->RangeStepAndValidator( 1e-07, 1e+06, 10., "length_precision" );
  aGroupLayout->addWidget( myMaxSize, row, 1 );
  row++;

  mySecondOrder = 0;
  if ( !myIsONLY )
  {
    mySecondOrder = new QCheckBox( tr( "NETGEN_SECOND_ORDER" ), GroupC1 );
    aGroupLayout->addWidget( mySecondOrder, row, 0 );
    row++;
  }

  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_FINENESS" ), GroupC1 ), row, 0 );
  myFineness = new QComboBox( GroupC1 );
  QStringList types;
  types << tr( "NETGEN_VERYCOARSE" ) << tr( "NETGEN_COARSE" )   << tr( "NETGEN_MODERATE" ) <<
           tr( "NETGEN_FINE" )       << tr( "NETGEN_VERYFINE" ) << tr( "NETGEN_CUSTOM" );
  myFineness->addItems( types );
  aGroupLayout->addWidget( myFineness, row, 1 );
  row++;

  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_GROWTH_RATE" ), GroupC1 ), row, 0 );
  myGrowthRate = new SMESHGUI_SpinBox( GroupC1 );
  myGrowthRate->RangeStepAndValidator( .0001, 10., .1, "parametric_precision" );
  aGroupLayout->addWidget( myGrowthRate, row, 1 );
  row++;

  myNbSegPerEdge = 0;
  myNbSegPerRadius = 0;
  if ( !myIsONLY )
  {
    const double VALUE_MAX = 1.0e+6;

    aGroupLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_EDGE" ), GroupC1 ), row, 0 );
    myNbSegPerEdge = new SMESHGUI_SpinBox( GroupC1 );
    myNbSegPerEdge->RangeStepAndValidator( .2, VALUE_MAX, .1, "parametric_precision" );
    aGroupLayout->addWidget( myNbSegPerEdge, row, 1 );
    row++;

    aGroupLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_RADIUS" ), GroupC1 ), row, 0 );
    myNbSegPerRadius = new SMESHGUI_SpinBox( GroupC1 );
    myNbSegPerRadius->RangeStepAndValidator( .2, VALUE_MAX, .1, "parametric_precision" );
    aGroupLayout->addWidget( myNbSegPerRadius, row, 1 );
    row++;
  }
  myAllowQuadrangles = 0;
  if ( myIs2D )
  {
    myAllowQuadrangles = new QCheckBox( tr( "NETGEN_ALLOW_QUADRANGLES" ), GroupC1 );
    aGroupLayout->addWidget( myAllowQuadrangles, row, 0 );
    row++;
  }

  myOptimize = 0;
  if ( !myIs2D )
  {
    myOptimize = new QCheckBox( tr( "NETGEN_OPTIMIZE" ), GroupC1 );
    aGroupLayout->addWidget( myOptimize, row, 0 );
    row++;
  }
  connect( myFineness, SIGNAL( activated( int ) ), this, SLOT( onFinenessChanged() ) );

  myLocalSizeTable = 0;
  if ( !myIsONLY )
  {
    QWidget* localSizeGroup = new QWidget();
    QGridLayout* localSizeLayout = new QGridLayout(localSizeGroup);

    myLocalSizeTable = new QTableWidget(0, LSZ_NB_COLUMNS, localSizeGroup);
    localSizeLayout->addWidget(myLocalSizeTable, 1, 0, 8, 1);
    QStringList localSizeHeaders;
    localSizeHeaders << tr( "LSZ_ENTRY_COLUMN" )<< tr( "LSZ_NAME_COLUMN" ) << tr( "LSZ_LOCALSIZE_COLUMN" );
    myLocalSizeTable->setHorizontalHeaderLabels(localSizeHeaders);
    myLocalSizeTable->horizontalHeader()->hideSection(LSZ_ENTRY_COLUMN);
    myLocalSizeTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    myLocalSizeTable->resizeColumnToContents(LSZ_NAME_COLUMN);
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
    myLocalSizeTable->setAlternatingRowColors(true);
    myLocalSizeTable->verticalHeader()->hide();

    QPushButton* addVertexButton = new QPushButton(tr("NETGEN_LSZ_VERTEX"), localSizeGroup);
    localSizeLayout->addWidget(addVertexButton, LSZ_VERTEX_BTN, 1, 1, 1);
    QPushButton* addEdgeButton = new QPushButton(tr("NETGEN_LSZ_EDGE"), localSizeGroup);
    localSizeLayout->addWidget(addEdgeButton, LSZ_EDGE_BTN, 1, 1, 1);
#ifdef NETGEN_NEW
    QPushButton* addFaceButton = new QPushButton(tr("NETGEN_LSZ_FACE"), localSizeGroup);
    localSizeLayout->addWidget(addFaceButton, LSZ_FACE_BTN, 1, 1, 1);
#endif

    QFrame *line2 = new QFrame(localSizeGroup);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    localSizeLayout->addWidget(line2, LSZ_SEPARATOR2, 1, 1, 1);

    QPushButton* removeButton = new QPushButton(tr("NETGEN_LSZ_REMOVE"), localSizeGroup);
    localSizeLayout->addWidget(removeButton, LSZ_REMOVE_BTN, 1, 1, 1);

    connect( addVertexButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnVertex()));
    connect( addEdgeButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnEdge()));
#ifdef NETGEN_NEW
    connect( addFaceButton, SIGNAL(clicked()), this, SLOT(onAddLocalSizeOnFace()));
#endif
    connect( removeButton, SIGNAL(clicked()), this, SLOT(onRemoveLocalSizeOnShape()));
    connect( myLocalSizeTable, SIGNAL(cellChanged(int, int)), this, SLOT(onSetLocalSize(int, int)));

    tab->insertTab(LSZ_TAB, localSizeGroup, tr("NETGEN_LOCAL_SIZE"));
  }
  return fr;
}

void NETGENPluginGUI_HypothesisCreator::retrieveParams() const
{
  NetgenHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );
  if(data.myMaxSizeVar.isEmpty())
    myMaxSize->setValue( data.myMaxSize );
  else
    myMaxSize->setText( data.myMaxSizeVar );

  if ( mySecondOrder )
    mySecondOrder->setChecked( data.mySecondOrder );
  if ( myOptimize )
    myOptimize->setChecked( data.myOptimize );
  myFineness->setCurrentIndex( data.myFineness );

  if(data.myGrowthRateVar.isEmpty())
    myGrowthRate->setValue( data.myGrowthRate );
  else
    myGrowthRate->setText( data.myGrowthRateVar );

  if ( myNbSegPerEdge )
  {
    if(data.myNbSegPerEdgeVar.isEmpty())
      myNbSegPerEdge->setValue( data.myNbSegPerEdge );
    else
      myNbSegPerEdge->setText( data.myNbSegPerEdgeVar );
  }
  if ( myNbSegPerRadius )
  {
    if(data.myNbSegPerRadiusVar.isEmpty())
      myNbSegPerRadius->setValue( data.myNbSegPerRadius );
    else
      myNbSegPerRadius->setText( data.myNbSegPerRadiusVar );
  }  
  if (myAllowQuadrangles)
    myAllowQuadrangles->setChecked( data.myAllowQuadrangles );

  // update widgets
  bool isCustom = (myFineness->currentIndex() == UserDefined);
  myGrowthRate->setEnabled(isCustom);
  if ( myNbSegPerEdge )
    myNbSegPerEdge->setEnabled(isCustom);
  if ( myNbSegPerRadius )
    myNbSegPerRadius->setEnabled(isCustom);

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
  }
}

QString NETGENPluginGUI_HypothesisCreator::storeParams() const
{
  NetgenHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );
  
  QString valStr = tr("NETGEN_MAX_SIZE") + " = " + QString::number( data.myMaxSize ) + "; ";
  if ( data.mySecondOrder )
    valStr +=  tr("NETGEN_SECOND_ORDER") + "; ";
  if ( data.myOptimize )
    valStr +=  tr("NETGEN_OPTIMIZE") + "; ";
  valStr += myFineness->currentText() + "(" +  QString::number( data.myGrowthRate )     + ", " +
                                               QString::number( data.myNbSegPerEdge )   + ", " +
                                               QString::number( data.myNbSegPerRadius ) + ")";

  if ( myIs2D && data.myAllowQuadrangles )
    valStr += "; " + tr("NETGEN_ALLOW_QUADRANGLES");
  
  return valStr;
}

bool NETGENPluginGUI_HypothesisCreator::readParamsFromHypo( NetgenHypothesisData& h_data ) const
{
  NETGENPlugin::NETGENPlugin_Hypothesis_var h =
    NETGENPlugin::NETGENPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName = isCreation() && data ? data->Label : "";

  SMESH::ListOfParameters_var aParameters = h->GetLastParameters();

  h_data.myMaxSize = h->GetMaxSize();
  h_data.myMaxSizeVar = (aParameters->length() > 0) ? QString(aParameters[0].in()) : QString("");
  h_data.mySecondOrder = h->GetSecondOrder();
  h_data.myOptimize = h->GetOptimize();

  h_data.myFineness = (int) h->GetFineness();
  h_data.myGrowthRate = h->GetGrowthRate();
  h_data.myGrowthRateVar = (aParameters->length() > 1) ? QString(aParameters[1].in()) : QString("");
  h_data.myNbSegPerEdge = h->GetNbSegPerEdge();
  h_data.myNbSegPerEdgeVar  = (aParameters->length() > 2) ? QString(aParameters[2].in()) : QString("");
  h_data.myNbSegPerRadius = h->GetNbSegPerRadius();
  h_data.myNbSegPerRadiusVar = (aParameters->length() > 3) ? QString(aParameters[3].in()) : QString("");

  if ( myIs2D )
    {
      NETGENPlugin::NETGENPlugin_Hypothesis_2D_var h_2d =
        NETGENPlugin::NETGENPlugin_Hypothesis_2D::_narrow( initParamsHypothesis() );

      if ( !h_2d->_is_nil() )
        h_data.myAllowQuadrangles = h_2d->GetQuadAllowed();
    }
  
  NETGENPluginGUI_HypothesisCreator* that = (NETGENPluginGUI_HypothesisCreator*)this;
  NETGENPlugin::string_array_var myEntries = h->GetLocalSizeEntries();
  for ( int i=0 ; i<myEntries->length() ; i++ )
    {
      QString entry = myEntries[i].in();
      double val = h->GetLocalSizeOnEntry(entry.toStdString().c_str());
      std::ostringstream tmp;
      tmp << val;
      QString valstring = QString::fromStdString(tmp.str());
      if (myLocalSizeMap.contains(entry))
        {
          if (myLocalSizeMap[entry] == "__TO_DELETE__")
            {
              continue;
            }
        }
      that->myLocalSizeMap[entry] = valstring;
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
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().data() );
    QStringList aVariablesList;
    h->SetMaxSize( h_data.myMaxSize );
    aVariablesList.append(h_data.myMaxSizeVar);
    h->SetSecondOrder( h_data.mySecondOrder );
    h->SetOptimize( h_data.myOptimize );
    int fineness = h_data.myFineness;
    h->SetFineness( fineness );

    if( fineness==UserDefined )
      {
        h->SetGrowthRate( h_data.myGrowthRate );
        h->SetNbSegPerEdge( h_data.myNbSegPerEdge );
        h->SetNbSegPerRadius( h_data.myNbSegPerRadius );
        
        aVariablesList.append(h_data.myGrowthRateVar);
        aVariablesList.append(h_data.myNbSegPerEdgeVar);
        aVariablesList.append(h_data.myNbSegPerRadiusVar);
      }
    
    if ( myIs2D )
      {
        NETGENPlugin::NETGENPlugin_Hypothesis_2D_var h_2d =
          NETGENPlugin::NETGENPlugin_Hypothesis_2D::_narrow( h );
        
        if ( !h_2d->_is_nil() )
          h_2d->SetQuadAllowed( h_data.myAllowQuadrangles );
      }

    h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    if( fineness==UserDefined )
      {
        h->SetParameters(aVariablesList.join(":").toLatin1().constData());
        h->SetParameters(aVariablesList.join(":").toLatin1().constData());
        h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      }

    QMapIterator<QString,QString> i(myLocalSizeMap);
    while (i.hasNext()) {
      i.next();
      const QString entry = i.key();
      const QString localSize = i.value();
      if (localSize == "__TO_DELETE__")
        {
          h->UnsetLocalSizeOnEntry(entry.toLatin1().constData());
        }
      else
        {
          std::istringstream tmp(localSize.toLatin1().constData());
          double val;
          tmp >> val;
          h->SetLocalSizeOnEntry(entry.toLatin1().constData(), val);
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

  
  if ( myAllowQuadrangles )
    h_data.myAllowQuadrangles = myAllowQuadrangles->isChecked();

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
  }
  return true;
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
          aGrowthRate = 0.7;
          aNbSegPerEdge = 0.3;
          aNbSegPerRadius = 1;
          break;
        case Coarse:
          aGrowthRate = 0.5;
          aNbSegPerEdge = 0.5;
          aNbSegPerRadius = 1.5;
          break;
        case Fine:
          aGrowthRate = 0.2;
          aNbSegPerEdge = 2;
          aNbSegPerRadius = 3;
          break;
        case VeryFine:
          aGrowthRate = 0.1;
          aNbSegPerEdge = 3;
          aNbSegPerRadius = 5;
          break;
        case Moderate:
        default:
          aGrowthRate = 0.3;
          aNbSegPerEdge = 1;
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

void NETGENPluginGUI_HypothesisCreator::addLocalSizeOnShape(TopAbs_ShapeEnum typeShapeAsked)
{
  NETGENPlugin::NETGENPlugin_Hypothesis_var h = NETGENPlugin::NETGENPlugin_Hypothesis::_narrow(initParamsHypothesis());
  GeomSelectionTools* geomSelectionTools = getGeomSelectionTools();
  LightApp_SelectionMgr* mySel = geomSelectionTools->selectionMgr();
  SALOME_ListIO ListSelectedObjects;
  mySel->selectedObjects(ListSelectedObjects, NULL, false );
  SALOME_ListIteratorOfListIO Object_It(ListSelectedObjects);
  for (Object_It ; Object_It.More() ; Object_It.Next())
    {
      Handle(SALOME_InteractiveObject) anObject = Object_It.Value();
      std::string entry, shapeName;
      entry = geomSelectionTools->getEntryOfObject(anObject);
      shapeName = anObject->getName();
      TopAbs_ShapeEnum shapeType;
      shapeType = geomSelectionTools->entryToShapeType(entry);
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
      QString shapeEntry;
      shapeEntry = QString::fromStdString(entry);
      if (myLocalSizeMap.contains(shapeEntry))
        {
          if (myLocalSizeMap[shapeEntry] != "__TO_DELETE__")
            {
              continue;
            }
        }
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
    QString entry = myLocalSizeTable->item(row, LSZ_ENTRY_COLUMN)->text();
    QString localSize = myLocalSizeTable->item(row, LSZ_LOCALSIZE_COLUMN)->text().trimmed();
    myLocalSizeMap[entry] = localSize;
    myLocalSizeTable->resizeColumnToContents(LSZ_LOCALSIZE_COLUMN);
  }
}

GeomSelectionTools* NETGENPluginGUI_HypothesisCreator::getGeomSelectionTools()
{
  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  if (myGeomSelectionTools == NULL || myGeomSelectionTools->getMyStudy() != aStudy) {
    delete myGeomSelectionTools;
    myGeomSelectionTools = new GeomSelectionTools(aStudy);
  }
  return myGeomSelectionTools;
}

QString NETGENPluginGUI_HypothesisCreator::caption() const
{
  return tr( QString( "NETGEN_%1_TITLE" ).arg(myIs2D?QString("2D"):QString("3D")).toLatin1().data() );
}

QPixmap NETGENPluginGUI_HypothesisCreator::icon() const
{
  QString hypIconName = tr( QString("ICON_DLG_NETGEN_PARAMETERS%1").arg(myIs2D?QString("_2D"):QString("")).toLatin1().data() );
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "NETGENPlugin", hypIconName );
}

QString NETGENPluginGUI_HypothesisCreator::type() const
{
  return tr( QString( "NETGEN_%1_HYPOTHESIS" ).arg(myIs2D?QString("2D"):QString("3D")).toLatin1().data() );
}

QString NETGENPluginGUI_HypothesisCreator::helpPage() const
{
  return "netgen_2d_3d_hypo_page.html";
}
