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
//  NETGENPlugin GUI: GUI for plugged-in mesher NETGENPlugin
//  File   : NETGENPluginGUI_HypothesisCreator.h
//  Author : Michael Zorin
//  Module : NETGENPlugin
//  $Header: 
//
#ifndef NETGENPLUGINGUI_HypothesisCreator_HeaderFile
#define NETGENPLUGINGUI_HypothesisCreator_HeaderFile

#ifdef WIN32
  #ifdef NETGENPLUGIN_GUI_EXPORTS
    #define NETGENPLUGIN_GUI_EXPORT __declspec( dllexport )
  #else
    #define NETGENPLUGIN_GUI_EXPORT __declspec( dllimport )
  #endif
#else
  #define NETGENPLUGIN_GUI_EXPORT
#endif

#include <SMESHGUI_Hypotheses.h>

class QtxDblSpinBox;
class QtxComboBox;
class QCheckBox;
class QLineEdit;

typedef struct
{
  double              myMaxSize, myGrowthRate, myNbSegPerEdge, myNbSegPerRadius;
  int                 myFineness;
  bool                mySecondOrder, myAllowQuadrangles, myOptimize;
  QString             myName;
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

  virtual bool     checkParams() const;
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

private:
  bool readParamsFromHypo( NetgenHypothesisData& ) const;
  bool readParamsFromWidgets( NetgenHypothesisData& ) const;
  bool storeParamsToHypo( const NetgenHypothesisData& ) const;

private:
 QLineEdit*       myName;
 QtxDblSpinBox*   myMaxSize;
 QCheckBox*       mySecondOrder;
 QCheckBox*       myOptimize;
 QtxComboBox*     myFineness;
 QtxDblSpinBox*   myGrowthRate;
 QtxDblSpinBox*   myNbSegPerEdge;
 QtxDblSpinBox*   myNbSegPerRadius;
 QCheckBox*       myAllowQuadrangles;

 bool myIs2D;
};

#endif
