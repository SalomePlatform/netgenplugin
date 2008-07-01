//  NETGENPlugin GUI: GUI for plugged-in mesher NETGENPlugin
//
//  Copyright (C) 2003  CEA
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : NETGENPluginGUI_HypothesisCreator.cxx
//  Author : Michael Zorin
//  Module : NETGENPlugin
//  $Header: 

#include "NETGENPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

#include <SalomeApp_Tools.h>

#include <QtxDoubleSpinBox.h>
#include <QtxComboBox.h>

#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPixmap>

 enum Fineness
   {
     VeryCoarse,
     Coarse,
     Moderate,
     Fine,
     VeryFine,
     UserDefined
   };

NETGENPluginGUI_HypothesisCreator::NETGENPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs2D(false)
{
}

NETGENPluginGUI_HypothesisCreator::~NETGENPluginGUI_HypothesisCreator()
{
}

bool NETGENPluginGUI_HypothesisCreator::checkParams() const
{
  NetgenHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  storeParamsToHypo( data_old );
  return res;
}

QFrame* NETGENPluginGUI_HypothesisCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0 );
  fr->setObjectName( "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( GroupC1 );
  
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
  myMaxSize = new QtxDoubleSpinBox( GroupC1 );
  myMaxSize->setDecimals( 7 );
  myMaxSize->setMinimum( 1e-07 );
  myMaxSize->setMaximum( 1e+06 );
  myMaxSize->setSingleStep( 10 );
  aGroupLayout->addWidget( myMaxSize, row, 1 );
  row++;
  
  mySecondOrder = new QCheckBox( tr( "NETGEN_SECOND_ORDER" ), GroupC1 );
  aGroupLayout->addWidget( mySecondOrder, row, 0 );
  row++;
  
  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_FINENESS" ), GroupC1 ), row, 0 );
  myFineness = new QtxComboBox( GroupC1 );
  QStringList types;
  types.append( QObject::tr( "NETGEN_VERYCOARSE" ) );
  types.append( QObject::tr( "NETGEN_COARSE" ) );
  types.append( QObject::tr( "NETGEN_MODERATE" ) );
  types.append( QObject::tr( "NETGEN_FINE" ) );
  types.append( QObject::tr( "NETGEN_VERYFINE" ) );
  types.append( QObject::tr( "NETGEN_CUSTOM" ) );
  myFineness->insertItems( myFineness->count(), types );
  aGroupLayout->addWidget( myFineness, row, 1 );
  row++;

  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_GROWTH_RATE" ), GroupC1 ), row, 0 );
  myGrowthRate = new QtxDoubleSpinBox( GroupC1 );
  myGrowthRate->setMinimum( 0.1 );
  myGrowthRate->setMaximum( 10 );
  myGrowthRate->setSingleStep( 0.1 );
  aGroupLayout->addWidget( myGrowthRate, row, 1 );
  row++;

  const double VALUE_MAX = 1.0e+6;

  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_EDGE" ), GroupC1 ), row, 0 );
  myNbSegPerEdge = new QtxDoubleSpinBox( GroupC1 );
  myNbSegPerEdge->setMinimum( 0.2 );
  myNbSegPerEdge->setMaximum( VALUE_MAX ); // (PAL14890) max value in native netgen gui is 5
  aGroupLayout->addWidget( myNbSegPerEdge, row, 1 );
  row++;
  
  aGroupLayout->addWidget( new QLabel( tr( "NETGEN_SEG_PER_RADIUS" ), GroupC1 ), row, 0 );
  myNbSegPerRadius = new QtxDoubleSpinBox( GroupC1 );
  myNbSegPerRadius->setMinimum( 0.2 );
  myNbSegPerRadius->setMaximum( VALUE_MAX ); // (PAL14890) max value in native netgen gui is 5
  aGroupLayout->addWidget( myNbSegPerRadius, row, 1 );
  row++;

  if ( hypType()=="NETGEN_Parameters_2D" )
  {
    myAllowQuadrangles = new QCheckBox( tr( "NETGEN_ALLOW_QUADRANGLES" ), GroupC1 );
    aGroupLayout->addWidget( myAllowQuadrangles, row, 0 );
    myIs2D = true;
    row++;
  }

  myOptimize = new QCheckBox( tr( "NETGEN_OPTIMIZE" ), GroupC1 );
  aGroupLayout->addWidget( myOptimize, row, 0 );
  row++;
  
  connect( myFineness, SIGNAL( activated( int ) ), this, SLOT( onFinenessChanged() ) );
  
  return fr;
}

void NETGENPluginGUI_HypothesisCreator::retrieveParams() const
{
  NetgenHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );
  myMaxSize->setValue( data.myMaxSize );
  mySecondOrder->setChecked( data.mySecondOrder );
  myOptimize->setChecked( data.myOptimize );
  myFineness->setCurrentIndex( data.myFineness );
  myGrowthRate->setValue( data.myGrowthRate );
  myNbSegPerEdge->setValue( data.myNbSegPerEdge );
  myNbSegPerRadius->setValue( data.myNbSegPerRadius );
  if (myIs2D)
    myAllowQuadrangles->setChecked( data.myAllowQuadrangles );

  // update widgets
  bool isCustom = (myFineness->currentIndex() == UserDefined);
  myGrowthRate->setEnabled(isCustom);
  myNbSegPerEdge->setEnabled(isCustom);
  myNbSegPerRadius->setEnabled(isCustom);
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

  h_data.myMaxSize = h->GetMaxSize();
  h_data.mySecondOrder = h->GetSecondOrder();
  h_data.myOptimize = h->GetOptimize();

  h_data.myFineness = (int) h->GetFineness();
  h_data.myGrowthRate = h->GetGrowthRate();
  h_data.myNbSegPerEdge = h->GetNbSegPerEdge();
  h_data.myNbSegPerRadius = h->GetNbSegPerRadius();

  if ( myIs2D )
    {
      NETGENPlugin::NETGENPlugin_Hypothesis_2D_var h_2d =
	NETGENPlugin::NETGENPlugin_Hypothesis_2D::_narrow( initParamsHypothesis() );

      if ( !h_2d->_is_nil() )
	h_data.myAllowQuadrangles = h_2d->GetQuadAllowed();
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

    h->SetMaxSize( h_data.myMaxSize );
    h->SetSecondOrder( h_data.mySecondOrder );
    h->SetOptimize( h_data.myOptimize );
    int fineness = h_data.myFineness;
    h->SetFineness( fineness );

    if( fineness==UserDefined )
      {
	h->SetGrowthRate( h_data.myGrowthRate );
	h->SetNbSegPerEdge( h_data.myNbSegPerEdge );
	h->SetNbSegPerRadius( h_data.myNbSegPerRadius );
      }
    
    if ( myIs2D )
      {
	NETGENPlugin::NETGENPlugin_Hypothesis_2D_var h_2d =
	  NETGENPlugin::NETGENPlugin_Hypothesis_2D::_narrow( h );
	
	if ( !h_2d->_is_nil() )
	  h_2d->SetQuadAllowed( h_data.myAllowQuadrangles );
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
  h_data.mySecondOrder    = mySecondOrder->isChecked();
  h_data.myOptimize       = myOptimize->isChecked();
  h_data.myFineness       = myFineness->currentIndex();
  h_data.myGrowthRate     = myGrowthRate->value();
  h_data.myNbSegPerEdge   = myNbSegPerEdge->value();
  h_data.myNbSegPerRadius = myNbSegPerRadius->value();
  
  if ( myIs2D )
    h_data.myAllowQuadrangles = myAllowQuadrangles->isChecked();
  
  return true;
}

void NETGENPluginGUI_HypothesisCreator::onFinenessChanged()
{
  bool isCustom = (myFineness->currentIndex() == UserDefined);
  
  myGrowthRate->setEnabled(isCustom);
  myNbSegPerEdge->setEnabled(isCustom);
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
      myNbSegPerEdge->setValue( aNbSegPerEdge );
      myNbSegPerRadius->setValue( aNbSegPerRadius );
    }
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
