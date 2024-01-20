// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

//=============================================================================
// File      : NETGENPlugin_Defs.hxx
// Author    : Alexander A. BORODIN
//
#ifndef _NETGENPlugin_DEFS_HXX_
#define _NETGENPlugin_DEFS_HXX_

#ifdef WIN32

  #if defined NETGENPLUGIN_EXPORTS || defined NETGENEngine_EXPORTS
    #define NETGENPLUGIN_EXPORT __declspec( dllexport )
  #else
    #define NETGENPLUGIN_EXPORT __declspec( dllimport )
  #endif

  #if defined(NETGEN_V5) || defined(NETGEN_V6)
    #define NETGENPLUGIN_DLL_HEADER DLL_HEADER
  #else
    #define NETGENPLUGIN_DLL_HEADER
  #endif

#else

  #define NETGENPLUGIN_EXPORT
  #define NETGENPLUGIN_DLL_HEADER

#endif

#endif
