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

// File   : NETGENPluginGUI_SimpleCreator.h
// Author : Open CASCADE S.A.S.
//
#ifndef NETGENPluginGUI_SimpleCreator_H
#define NETGENPluginGUI_SimpleCreator_H


#include "NETGENPluginGUI.h"
// SMESH includes
#include <SMESHGUI_Hypotheses.h>

class  QCheckBox;
class  QLineEdit;
class  QRadioButton;
class  SalomeApp_IntSpinBox;
class  SMESHGUI_SpinBox;

class NETGENPLUGIN_GUI_EXPORT NETGENPluginGUI_SimpleCreator :
  public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  NETGENPluginGUI_SimpleCreator(const QString& theHypType);
  virtual ~NETGENPluginGUI_SimpleCreator();

  virtual bool     checkParams(QString& msg) const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame();
  virtual void     retrieveParams() const;
  virtual QString  storeParams() const;

  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

protected slots:
  void             onValueChanged();

private:
  QLineEdit       * myName;

  SalomeApp_IntSpinBox*    myNbSeg;
  SMESHGUI_SpinBox* myLength;
  QRadioButton*     myNbSegRadioBut, *myLengthRadioBut;

  QCheckBox*        myLenFromEdgesCheckBox;
  SMESHGUI_SpinBox* myArea;
  QCheckBox*        myAllowQuadCheckBox;

  QCheckBox*        myLenFromFacesCheckBox;
  SMESHGUI_SpinBox* myVolume;
  
};

#endif // NETGENPluginGUI_SimpleCreator_H
