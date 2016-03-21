// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
#include <SalomeApp_IntSpinBox.h>

// Qt includes
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QLineEdit>
//#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QCheckBox>

#define SPACING 6
#define MARGIN  11

using namespace NETGENPlugin;

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
  myAllowQuadCheckBox(0),
  myLenFromFacesCheckBox(0),
  myVolume(0)
{
}

NETGENPluginGUI_SimpleCreator::~NETGENPluginGUI_SimpleCreator()
{
}

bool NETGENPluginGUI_SimpleCreator::checkParams(QString& msg) const
{
  bool result = true;
  if ( myNbSeg->isEnabled() )
    result = myNbSeg->isValid(msg,true) && result;
  if ( myLength->isEnabled() )
    result = myLength->isValid(msg,true) && result;
  if ( myArea->isEnabled() )
    result = myArea->isValid(msg,true) && result;
  if (myVolume && myVolume->isEnabled() )
    result = myVolume->isValid(msg,true) && result;

  return result;
}

QFrame* NETGENPluginGUI_SimpleCreator::buildFrame()
{
  QFrame* fr = new QFrame();

  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 0 );
  lay->setSpacing( 0 );

  QGroupBox* argGroup = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( argGroup );

  QGridLayout* argLay = new QGridLayout( argGroup );
  argLay->setSpacing( SPACING );
  argLay->setMargin( MARGIN );
  argLay->setColumnStretch( 0, 0 );
  argLay->setColumnStretch( 1, 1 );
  int argRow = 0;

  // Name
  if( isCreation() ) {
    myName = new QLineEdit( argGroup );
    argLay->addWidget( new QLabel( tr( "SMESH_NAME" ), argGroup ), argRow, 0 );
    argLay->addWidget( myName, argRow, 1 );
    argRow++;
  }

  QGroupBox* dimGroup;
  QGridLayout* dimLay;
  int dimRow;

  // 1D params group

  dimGroup = new QGroupBox( tr( "NG_1D" ), argGroup );
  argLay->addWidget( dimGroup, argRow, 0, 1, 2 );
  argRow++;

  dimLay = new QGridLayout( dimGroup );
  dimLay->setSpacing( SPACING );
  dimLay->setMargin( MARGIN );
  dimLay->setColumnStretch( 0, 0 );
  dimLay->setColumnStretch( 1, 1 );
  dimRow = 0;

  // *  number of segments
  myNbSegRadioBut  = new QRadioButton( tr( "SMESH_NB_SEGMENTS_HYPOTHESIS"  ), dimGroup );
  myNbSeg = new SalomeApp_IntSpinBox( dimGroup );
  myNbSeg->setMinimum( 1 );
  myNbSeg->setMaximum( 9999 );
  myNbSeg->setValue( 1 );
  dimLay->addWidget( myNbSegRadioBut, dimRow, 0 );
  dimLay->addWidget( myNbSeg, dimRow, 1 );
  dimRow++;

  // * local length
  myLengthRadioBut = new QRadioButton( tr( "SMESH_LOCAL_LENGTH_HYPOTHESIS" ), dimGroup );
  myLength = new SMESHGUI_SpinBox( dimGroup );
  myLength->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 0.1, "length_precision" );
  myLength->setValue( 1. );
  dimLay->addWidget( myLengthRadioBut, dimRow, 0 );
  dimLay->addWidget( myLength, dimRow, 1 );
  dimRow++;

  // 2D params group

  dimGroup = new QGroupBox( tr( "NG_2D" ), argGroup );
  argLay->addWidget( dimGroup, argRow, 0, 1, 2 );
  argRow++;

  dimLay = new QGridLayout( dimGroup );
  dimLay->setSpacing( SPACING );
  dimLay->setMargin( MARGIN );
  dimLay->setColumnStretch( 0, 0 );
  dimLay->setColumnStretch( 1, 1 );
  dimRow = 0;

  // *  Length from edges
  myLenFromEdgesCheckBox = new QCheckBox( tr( "NG_LENGTH_FROM_EDGES" ), dimGroup );
  dimLay->addWidget( myLenFromEdgesCheckBox, dimRow, 0, 1, 2 );
  dimRow++;

  // * max area
  dimLay->addWidget( new QLabel( tr( "SMESH_MAX_ELEMENT_AREA_HYPOTHESIS" ), dimGroup), dimRow, 0);
  myArea = new SMESHGUI_SpinBox( dimGroup );
  myArea->RangeStepAndValidator( VALUE_SMALL_2, VALUE_MAX_2, 0.1, "area_precision" );
  myArea->setValue( 1. );
  dimLay->addWidget( myArea, dimRow, 1 );
  dimRow++;

  // * allow quadrangles
  const bool is3D = ( hypType()=="NETGEN_SimpleParameters_3D" );
  if ( !is3D )
  {
    myAllowQuadCheckBox = new QCheckBox( tr( "NETGEN_ALLOW_QUADRANGLES" ), dimGroup );
    dimLay->addWidget( myAllowQuadCheckBox, dimRow, 0, 1, 2 );
    dimRow++;
  }

  // 3D params group
  if ( is3D )
  {
    dimGroup = new QGroupBox( tr( "NG_3D" ), argGroup );
    argLay->addWidget( dimGroup, argRow, 0, 1, 2 );
    argRow++;

    dimLay = new QGridLayout( dimGroup );
    dimLay->setSpacing( SPACING );
    dimLay->setMargin( MARGIN );
    dimLay->setColumnStretch( 0, 0 );
    dimLay->setColumnStretch( 1, 1 );
    dimRow = 0;

    // *  Length from faces
    myLenFromFacesCheckBox = new QCheckBox( tr( "NG_LENGTH_FROM_FACES" ), dimGroup );
    dimLay->addWidget( myLenFromFacesCheckBox, dimRow, 0, 1, 2 );
    dimRow++;

    // * max volume
    dimLay->addWidget(new QLabel( tr("SMESH_MAX_ELEMENT_VOLUME_HYPOTHESIS"), dimGroup), dimRow, 0);
    myVolume = new SMESHGUI_SpinBox( dimGroup );
    myVolume->RangeStepAndValidator( VALUE_SMALL_3, VALUE_MAX_3, 0.1, "volume_precision" );
    myVolume->setValue( 1. );
    dimLay->addWidget( myVolume, dimRow, 1 );
    dimRow++;
  }

  connect( myNbSegRadioBut,  SIGNAL( clicked(bool) ), this, SLOT( onValueChanged() ));
  connect( myLengthRadioBut, SIGNAL( clicked(bool) ), this, SLOT( onValueChanged() ));
  connect( myLenFromEdgesCheckBox, SIGNAL( stateChanged(int)), this, SLOT( onValueChanged() ));
  if ( myLenFromFacesCheckBox )
    connect( myLenFromFacesCheckBox, SIGNAL( stateChanged(int)), this, SLOT( onValueChanged() ));

  return fr;
}

void NETGENPluginGUI_SimpleCreator::retrieveParams() const
{
  if ( isCreation() )
    myName->setText( hypName() );

  // Set default values

  NETGENPlugin_SimpleHypothesis_2D_var h =
    NETGENPlugin_SimpleHypothesis_2D::_narrow( initParamsHypothesis( hasInitParamsHypothesis() ));

  int dfltNbSeg = (int) h->GetNumberOfSegments();
  myNbSeg->setValue( dfltNbSeg );
  if ( double len = h->GetLocalLength() ) {
    myLength->setValue( len );
    myArea->setValue( len * len );
    if ( myVolume )
      myVolume->setValue( len * len * len );
  }

  h = NETGENPlugin_SimpleHypothesis_2D::_narrow( hypothesis() );

  // set values of hypothesis

  //SMESH::ListOfParameters_var aParameters = h->GetLastParameters();

  // 1D
  int nbSeg = isCreation() ? dfltNbSeg : (int) h->GetNumberOfSegments();
  myNbSegRadioBut->setChecked( nbSeg );
  myLengthRadioBut->setChecked( !nbSeg );
  QString aPrm;
  if ( nbSeg ) {
    myLength->setEnabled( false );
    myNbSeg->setEnabled( true );
    aPrm = getVariableName("SetNumberOfSegments");
    if(aPrm.isEmpty())
      myNbSeg->setValue( nbSeg );
    else
      myNbSeg->setText(aPrm);
  }
  else {
    myNbSeg->setEnabled( false );
    myLength->setEnabled( true );
    aPrm = getVariableName("SetLocalLength");
    if(aPrm.isEmpty())
      myLength->setValue( h->GetLocalLength() );
    else
      myLength->setText(aPrm);
  }

  // 2D
  if ( double area = h->GetMaxElementArea() ) {
    myLenFromEdgesCheckBox->setChecked( false );
    myArea->setEnabled( true );
    aPrm = getVariableName("SetMaxElementArea");
    if(aPrm.isEmpty()) 
      myArea->setValue( area );
    else
      myArea->setText( aPrm );
  }
  else {
    myLenFromEdgesCheckBox->setChecked( true );
    myArea->setEnabled( false );
  }
  if ( myAllowQuadCheckBox )
    myAllowQuadCheckBox->setChecked( h->GetAllowQuadrangles() );

  // 3D
  if ( myVolume ) {
    NETGENPlugin_SimpleHypothesis_3D_var h = NETGENPlugin_SimpleHypothesis_3D::_narrow( hypothesis() );
    if ( double volume = (double) h->GetMaxElementVolume() ) {
      myLenFromFacesCheckBox->setChecked( false );
      myVolume->setEnabled( true );
      aPrm = getVariableName("SetMaxElementVolume");
      if(aPrm.isEmpty())
        myVolume->setValue( volume );
      else
        myVolume->setText( aPrm );
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
    NETGENPlugin_SimpleHypothesis_2D_var h =
      NETGENPlugin_SimpleHypothesis_2D::_narrow( hypothesis() );

    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), myName->text().toLatin1().data() );

    

    // 1D
    if ( myNbSeg->isEnabled() ) {
      h->SetVarParameter( myNbSeg->text().toLatin1().constData(), "SetNumberOfSegments");
      h->SetNumberOfSegments( myNbSeg->value() );
      valStr += "nbSeg=" + myNbSeg->text();
    }
    else {
      h->SetVarParameter( myLength->text().toLatin1().constData(), "SetLocalLength");
      h->SetLocalLength( myLength->value() );
      valStr += "len=" + myLength->text();
    }
    
    // 2D
    if ( myArea->isEnabled() ) {
      h->SetVarParameter( myArea->text().toLatin1().constData(), "SetMaxElementArea");
      h->SetMaxElementArea( myArea->value() );
      valStr += "; area=" + myArea->text();
    }
    else {
      h->LengthFromEdges();
      valStr += "; lenFromEdges";
    }
    if ( myAllowQuadCheckBox )
      h->SetAllowQuadrangles( myAllowQuadCheckBox->isChecked() );

    // 3D
    if ( myVolume ) {
      NETGENPlugin_SimpleHypothesis_3D_var h =
        NETGENPlugin_SimpleHypothesis_3D::_narrow( hypothesis() );
      if ( myVolume->isEnabled() ) {
        h->SetVarParameter( myVolume->text().toLatin1().constData(), "SetMaxElementVolume");
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
  QObject* changed = sender();

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
  return tr( (hypType() + "_TITLE").toLatin1().data() );
}

QPixmap NETGENPluginGUI_SimpleCreator::icon() const
{
  QString hypIconName = tr( ("ICON_DLG_" + hypType()).toLatin1().data() );
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "NETGENPlugin", hypIconName );
}

QString NETGENPluginGUI_SimpleCreator::type() const
{
  return tr( (hypType() + "_HYPOTHESIS").toLatin1().data() );
}

QString NETGENPluginGUI_SimpleCreator::helpPage() const
{
  return "netgen_2d_3d_hypo_page.html";
}
