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
//  NETGENPlugin : C++ implementation
// File      : NETGENPlugin_Mesher.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 31/03/2006
// Project   : SALOME
//
#ifndef _NETGENPlugin_Mesher_HXX_
#define _NETGENPlugin_Mesher_HXX_

#include "NETGENPlugin_Defs.hxx"
#include "StdMeshers_FaceSide.hxx"
#include "SMDS_MeshElement.hxx"

namespace nglib {
#include <nglib.h>
}

#include <map>
#include <vector>
#include <set>

class SMESH_Mesh;
class SMESH_Comment;
class SMESHDS_Mesh;
class TopoDS_Shape;
class TopTools_DataMapOfShapeShape;
class NETGENPlugin_Hypothesis;
class NETGENPlugin_SimpleHypothesis_2D;
class NETGENPlugin_Internals;
namespace netgen {
  class OCCGeometry;
  class Mesh;
}
//=============================================================================
/*!
 * \brief Struct storing nb of entities in netgen mesh
 */
//=============================================================================

struct NETGENPlugin_ngMeshInfo
{
  int _nbNodes, _nbSegments, _nbFaces, _nbVolumes;
  NETGENPlugin_ngMeshInfo( netgen::Mesh* ngMesh=0);
};

//=============================================================================
/*!
 * \brief This class calls the NETGEN mesher of OCC geometry
 */
//=============================================================================

class NETGENPLUGIN_EXPORT NETGENPlugin_Mesher 
{
 public:
  // ---------- PUBLIC METHODS ----------

  NETGENPlugin_Mesher (SMESH_Mesh* mesh, const TopoDS_Shape& aShape,
                       const bool isVolume);

  void SetParameters(const NETGENPlugin_Hypothesis* hyp);
  void SetParameters(const NETGENPlugin_SimpleHypothesis_2D* hyp);

  bool Compute();

  bool Evaluate(MapShapeNbElems& aResMap);

  static void PrepareOCCgeometry(netgen::OCCGeometry&          occgeom,
                                 const TopoDS_Shape&           shape,
                                 SMESH_Mesh&                   mesh,
                                 std::list< SMESH_subMesh* > * meshedSM=0,
                                 NETGENPlugin_Internals*       internalShapes=0);

  static int FillSMesh(const netgen::OCCGeometry&          occgeom,
                       const netgen::Mesh&                 ngMesh,
                       const NETGENPlugin_ngMeshInfo&      initState,
                       SMESH_Mesh&                         sMesh,
                       std::vector<const SMDS_MeshNode*>&  nodeVec,
                       SMESH_Comment&                      comment);

  bool fillNgMesh(netgen::OCCGeometry&                occgeom,
                  netgen::Mesh&                       ngMesh,
                  std::vector<const SMDS_MeshNode*>&  nodeVec,
                  const std::list< SMESH_subMesh* > & meshedSM,
                  NETGENPlugin_Internals*             internalShapes=0);

  void defaultParameters();

  static void RemoveTmpFiles();

  static SMESH_ComputeErrorPtr readErrors(const std::vector< const SMDS_MeshNode* >& nodeVec);

 private:
  SMESH_Mesh*          _mesh;
  const TopoDS_Shape&  _shape;
  bool                 _isVolume;
  bool                 _optimize;

  const NETGENPlugin_SimpleHypothesis_2D * _simpleHyp;
  std::map< int, std::pair<int,int> >      _faceDescriptors;
};

//=============================================================================
/*!
 * \brief Container of info needed to solve problems with internal shapes.
 *
 * Issue 0020676. It is made up as a class to be ready to extract from NETGEN
 * and put in SMESH as soon as the same solution is needed somewhere else.
 * The approach is to precompute internal edges in 2D and internal faces in 3D
 * and put their mesh correctly (twice) into netgen mesh.
 * In 2D, this class finds internal edges in faces and their vertices.
 * In 3D, it additionally finds internal faces, their edges shared with other faces,
 * and their vertices shared by several internal edges. Nodes built on the found
 * shapes and mesh faces built on the found internal faces are to be doubled in
 * netgen mesh to emulate a "crack"
 */
//=============================================================================

class NETGENPLUGIN_EXPORT NETGENPlugin_Internals
{
  SMESH_Mesh&       _mesh;
  bool              _is3D;
  //2D
  std::map<int,int> _ev2face; //!< edges and vertices in faces where they are TopAbs_INTERNAL
  // 3D
  std::set<int>     _intShapes;
  std::set<int>     _borderFaces; //!< non-intrnal faces sharing the internal edge

public:
  NETGENPlugin_Internals( SMESH_Mesh& mesh, const TopoDS_Shape& shape, bool is3D );

  bool isShapeToPrecompute(const TopoDS_Shape& s);

  // 2D
  bool hasInternalEdges() const { return !_ev2face.empty(); }
  bool isInternalEdge(int id ) const { return _ev2face.count( id ); }
  bool isInternalVertex(int id ) const { return _ev2face.count( id ); }
  const std::map<int,int>& getEdgesAndVerticesWithFaces() const { return _ev2face; }
  void getInternalEdges( TopTools_IndexedMapOfShape& fmap,
                         TopTools_IndexedMapOfShape& emap,
                         TopTools_IndexedMapOfShape& vmap,
                         list< SMESH_subMesh* >& smToPrecompute);

  // 3D
  bool hasInternalFaces() const { return !_intShapes.empty(); }
  bool isInternalShape(int id ) const { return _intShapes.count( id ); }
  void findBorderElements( std::set< const SMDS_MeshElement*, TIDCompare > & borderElems );
  bool isBorderFace(int faceID ) const { return _borderFaces.count( faceID ); }
  void getInternalFaces( TopTools_IndexedMapOfShape& fmap,
                         TopTools_IndexedMapOfShape& emap,
                         list< SMESH_subMesh* >&     facesSM,
                         list< SMESH_subMesh* >&     boundarySM);

};

//================================================================================
/*!
 * \brief It correctly initializes netgen library at constructor and
 *        correctly finishes using netgen library at destructor
 */
//================================================================================

struct NETGENPLUGIN_EXPORT NETGENPlugin_NetgenLibWrapper
{
  nglib::Ng_Mesh * _ngMesh;
  NETGENPlugin_NetgenLibWrapper();
  ~NETGENPlugin_NetgenLibWrapper();
  void setMesh( nglib::Ng_Mesh* mesh );
};

#endif
