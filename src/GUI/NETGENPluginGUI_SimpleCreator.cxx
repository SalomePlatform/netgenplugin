//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : NETGENPluginGUI_SimpleCreator.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "NETGENPluginGUI_SimpleCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_SpinBox.h>

// IDL includes
#include CORBA_SERVER_HEADER(NETGENPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

// SALOME GUI includes
#include <SalomeApp_Tools.h>
#include <QtxIntSpinBox.h>

// Qt includes
#include <qlabel.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qhbox.h>

#define SPACING 6
#define MARGIN  11

// copied from StdMeshersGUI_StdHypothesisCreator.cxx
const double VALUE_MAX = 1.0e+15, // COORD_MAX
             VALUE_MAX_2  = VALUE_MAX * VALUE_MAX,
             VALUE_MAX_3  = VALUE_MAX_2 * VALUE_MAX,
             VALUE_SMALL = 1.0e-15,
             VALUE_SMALL_2 = VALUE_SMALL * VALUE_SMALL,
             VALUE_SMALL_3 = VALUE_SMALL_2 * VALUE_SMALL;

NETGENPluginGUI_SimpleCreator::NETGENPluginGUI_SimpleCreator(const QString& theHypType)
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myName(0),
  myNbSeg(0),
  myLength(0),
  myNbSegRadioBut(0),
  myLengthRadioBut(0),
  myLenFromEdgesCheckBox(0),
  myArea(0),
  myLenFromFacesCheckBox(0),
  myVolume(0)
{
}

NETGENPluginGUI_SimpleCreator::~NETGENPluginGUI_SimpleCreator()
{
}

bool NETGENPluginGUI_SimpleCreator::checkParams() const
{
  return true;
}

QFrame* NETGENPluginGUI_SimpleCreator::buildFrame()
{
  QFrame* fr = new QFrame();

  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 0 );
  lay->setSpacing( 0 );

  QGroupBox* argGroup = new QGroupBox( 1, Qt::Horizontal, tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( argGroup );

  // Name
  if( isCreation() ) {
    QHBox* aHBox = new QHBox( argGroup );
    aHBox->setSpacing( SPACING );
    new QLabel( tr( "SMESH_NAME" ), aHBox );
    myName = new QLineEdit( aHBox );
  }

  // 1D params group

  QGroupBox* dimGroup = new QGroupBox( 2, Qt::Horizontal, tr( "NG_1D" ), argGroup );

  // *  number of segments
  myNbSegRadioBut  = new QRadioButton( tr( "SMESH_NB_SEGMENTS_HYPOTHESIS"  ), dimGroup );
  myNbSeg = new QtxIntSpinBox( dimGroup );
  myNbSeg->setMinValue( 1 );
  myNbSeg->setMaxValue( 9999 );
  myNbSeg->setValue( 1 );

  // * local length
  myLengthRadioBut = new QRadioButton( tr( "SMESH_LOCAL_LENGTH_HYPOTHESIS" ), dimGroup );
  myLength = new SMESHGUI_SpinBox( dimGroup );
  myLength->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 0.1, 6 );
  myLength->setValue( 1. );
  
  // 2D params group

  dimGroup = new QGroupBox( 2, Qt::Horizontal, tr( "NG_2D" ), argGroup );

  // *  Length from edges
  myLenFromEdgesCheckBox = new QCheckBox( tr( "NG_LENGTH_FROM_EDGES" ), dimGroup );
  new QLabel(" ", dimGroup);

  // * max area
  new QLabel( tr( "SMESH_MAX_ELEMENT_AREA_HYPOTHESIS" ), dimGroup);
  myArea = new SMESHGUI_SpinBox( dimGroup );
  myArea->RangeStepAndValidator( VALUE_SMALL_2, VALUE_MAX_2, 0.1, 6 );
  myArea->setValue( 1. );

  // 3D params group
  if ( hypType()=="NETGEN_SimpleParameters_3D" )
  {
    dimGroup = new QGroupBox( 2, Qt::Horizontal, tr( "NG_3D" ), argGroup );

    // *  Length from faces
    myLenFromFacesCheckBox = new QCheckBox( tr( "NG_LENGTH_FROM_FACES" ), dimGroup );
    new QLabel(" ", dimGroup);

    // * max volume
    new QLabel( tr("SMESH_MAX_ELEMENT_VOLUME_HYPOTHESIS"), dimGroup );
    myVolume = new SMESHGUI_SpinBox( dimGroup );
    myVolume->RangeStepAndValidator( VALUE_SMALL_3, VALUE_MAX_3, 0.1, 6 );
    myVolume->setValue( 1. );
  }

  connect( myNbSegRadioBut,  SIGNAL( clicked() ), this, SLOT( onValueChanged() ));
  connect( myLengthRadioBut, SIGNAL( clicked() ), this, SLOT( onValueChanged() ));
  connect( myLenFromEdgesCheckBox, SIGNAL( stateChanged(int)), this, SLOT( onValueChanged() ));
  if ( myLenFromFacesCheckBox )
    connect( myLenFromFacesCheckBox, SIGNAL( stateChanged(int)), this, SLOT( onValueChanged() ));

  return fr;
}

void NETGENPluginGUI_SimpleCreator::retrieveParams() const
{
  if ( isCreation() ) {
    myName->setText( hypName() );
    QFontMetrics metrics( myName->font() );
    myName->setMinimumWidth( metrics.width( myName->text() )+5 );
  }

  NETGENPlugin::NETGENPlugin_SimpleHypothesis_2D_var h =
    NETGENPlugin::NETGENPlugin_SimpleHypothesis_2D::_narrow( initParamsHypothesis() );

  // 1D
  int nbSeg = (int) h->GetNumberOfSegments();
  myNbSegRadioBut->setChecked( nbSeg );
  myLengthRadioBut->setChecked( !nbSeg );
  if ( nbSeg ) {
    myLength->setEnabled( false );
    myNbSeg->setEnabled( true );
    myNbSeg->setValue( nbSeg );
  }
  else {
    myNbSeg->setEnabled( false );
    myLength->setEnabled( true );
    myLength->setValue( h->GetLocalLength() );
  }

  // 2D
  if ( double area = h->GetMaxElementArea() ) {
    myLenFromEdgesCheckBox->setChecked( false );
    myArea->setEnabled( true );
    myArea->setValue( area );
  }
  else {
    myLenFromEdgesCheckBox->setChecked( true );
    myArea->setEnabled( false );
  }

  // 3D
  if ( myVolume ) {
    NETGENPlugin::NETGENPlugin_SimpleHypothesis_3D_var h =
      NETGENPlugin::NETGENPlugin_SimpleHypothesis_3D::_narrow( initParamsHypothesis() );
    if ( double volume = (double) h->GetMaxElementVolume() ) {
      myLenFromFacesCheckBox->setChecked( false );
      myVolume->setEnabled( true );
      myVolume->setValue( volume );
    }
    else {
      myLenFromFacesCheckBox->setChecked( true );
      myVolume->setEnabled( false );
    }
  }
}

QString NETGENPluginGUI_SimpleCreator::storeParams() const
{
  QString valStr;
  try
  {
    NETGENPlugin::NETGENPlugin_SimpleHypothesis_2D_var h =
      NETGENPlugin::NETGENPlugin_SimpleHypothesis_2D::_narrow( initParamsHypothesis() );

    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), myName->text().latin1() );

    // 1D
    if ( myNbSeg->isEnabled() ) {
      h->SetNumberOfSegments( myNbSeg->value() );
      valStr += "nbSeg=" + myNbSeg->text();
    }
    else {
      h->SetLocalLength( myLength->value() );
      valStr += "len=" + myNbSeg->text();
    }

    // 2D
    if ( myArea->isEnabled() ) {
      h->SetMaxElementArea( myArea->value() );
      valStr += "; area=" + myArea->text();
    }
    else {
      h->LengthFromEdges();
      valStr += "; lenFromEdges";
    }

    // 3D
    if ( myVolume ) {
      NETGENPlugin::NETGENPlugin_SimpleHypothesis_3D_var h =
        NETGENPlugin::NETGENPlugin_SimpleHypothesis_3D::_narrow( initParamsHypothesis() );
      if ( myVolume->isEnabled() ) {
        h->SetMaxElementVolume( myVolume->value() );
        valStr += "; vol=" + myVolume->text();
      }
      else {
        h->LengthFromFaces();
        valStr += "; lenFromFaces";
      }
    }
  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
  }

  return valStr;
}

void NETGENPluginGUI_SimpleCreator::onValueChanged()
{
  const QObject* changed = sender();

  if ( myNbSegRadioBut == changed )
  {
    myLengthRadioBut->setChecked( !myNbSegRadioBut->isChecked() );
  }
  else if ( myLengthRadioBut == changed )
  {
    myNbSegRadioBut->setChecked( !myLengthRadioBut->isChecked() );
  }
  else if ( myLenFromEdgesCheckBox == changed )
  {
    myArea->setEnabled( !myLenFromEdgesCheckBox->isChecked() );
  }
  else if ( myLenFromFacesCheckBox == changed )
  {
    myVolume->setEnabled( !myLenFromFacesCheckBox->isChecked() );
  }
  myLength->setEnabled( myLengthRadioBut->isChecked() );
  myNbSeg->setEnabled( myNbSegRadioBut->isChecked() );
}

QString NETGENPluginGUI_SimpleCreator::caption() const
{
  return tr( (hypType() + "_TITLE").latin1() );
}

QPixmap NETGENPluginGUI_SimpleCreator::icon() const
{
  QString hypIconName = tr( ("ICON_DLG_" + hypType()).latin1() );
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "NETGENPlugin", hypIconName );
}

QString NETGENPluginGUI_SimpleCreator::type() const
{
  return tr( (hypType() + "_HYPOTHESIS").latin1() );
}

QString NETGENPluginGUI_SimpleCreator::helpPage() const
{
  return "netgen_2d_3d_hypo_page.html";
}
