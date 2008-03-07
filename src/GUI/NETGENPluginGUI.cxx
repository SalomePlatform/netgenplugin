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
//  File   : NETGENPluginGUI.cxx
//  Author : Michael Zorin
//  Module : NETGENPlugin
//  $Header: 

//#include "SMESHGUI_Hypotheses.h"
#include "NETGENPluginGUI_HypothesisCreator.h"

//=============================================================================
/*! GetHypothesisCreator
 *
 */
//=============================================================================
extern "C"
{
  NETGENPLUGIN_GUI_EXPORT
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator( const QString& aHypType )
  {
    SMESHGUI_GenericHypothesisCreator* aCreator = NULL;
    if( aHypType=="NETGEN_Parameters_2D" ||  aHypType=="NETGEN_Parameters" )
      aCreator =  new NETGENPluginGUI_HypothesisCreator( aHypType );
    return aCreator;
  }
}
