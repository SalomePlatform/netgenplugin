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

//  NETGENPlugin : C++ implementation
// File      : NETGENPlugin_Mesher.hxx
// Author    : Michael Sazonov (OCN)
// Date      : 31/03/2006
// Project   : SALOME
//
#ifndef _NETGENPlugin_Mesher_HXX_
#define _NETGENPlugin_Mesher_HXX_

#include "NETGENPlugin_Defs.hxx"

#include <StdMeshers_FaceSide.hxx>
#include <SMDS_MeshElement.hxx>
#include <SMESH_Algo.hxx>
#include <SMESH_ProxyMesh.hxx>
#include <SALOMEDS_Tool.hxx>
#include "Basics_Utils.hxx"
#include "SALOME_Basics.hxx"
#include <TopTools_IndexedMapOfShape.hxx>

// Netgen include files
#ifndef OCCGEOMETRY
#define OCCGEOMETRY
#endif
#include <occgeom.hpp>
#include <meshing.hpp>

namespace nglib {
#include <nglib.h>
}

#include <map>
#include <vector>
#include <set>

class NETGENPlugin_Hypothesis;
class NETGENPlugin_Internals;
class NETGENPlugin_SimpleHypothesis_2D;
class SMESHDS_Mesh;
class SMESH_Comment;
class SMESH_Mesh;
class SMESH_MesherHelper;
class StdMeshers_ViscousLayers;
class TopoDS_Shape;
namespace netgen {
  class OCCGeometry;
  class Mesh;
  NETGENPLUGIN_DLL_HEADER
  extern MeshingParameters mparam;
}

// Class for temporary folder switching
class ChdirRAII
{
  public:
#ifndef WIN32
    ChdirRAII(const std::string& wd):_wd(wd) { if(_wd.empty()) return ; char *pwd(get_current_dir_name()); _od = pwd; free(pwd); chdir(_wd.c_str()); }
    ~ChdirRAII() { if(_od.empty()) return ; chdir(_od.c_str()); }
#else
    ChdirRAII(const std::string& wd) : _wd(wd) {
      if (_wd.empty())
        return;
      TCHAR pwd[MAX_PATH];
      GetCurrentDirectory(sizeof(pwd), pwd);
      _od = Kernel_Utils::utf8_encode_s(pwd);
      SetCurrentDirectory(Kernel_Utils::utf8_decode_s(_wd).c_str());
    }
    ~ChdirRAII() {
      if (_od.empty()) return;
      SetCurrentDirectory(Kernel_Utils::utf8_decode_s(_od).c_str());
    }
#endif
  private:
    std::string _wd;
    std::string _od;
};
//=============================================================================
/*!
 * \brief Struct storing nb of entities in netgen mesh
 */
//=============================================================================

struct NETGENPlugin_ngMeshInfo
{
  int   _nbNodes, _nbSegments, _nbFaces, _nbVolumes;
  bool  _elementsRemoved; // case where netgen can remove free nodes
  char* _copyOfLocalH;
  NETGENPlugin_ngMeshInfo( netgen::Mesh* ngMesh=0, bool checkRemovedElems=false );
  void transferLocalH( netgen::Mesh* fromMesh, netgen::Mesh* toMesh );
  void restoreLocalH ( netgen::Mesh* ngMesh);
};

//================================================================================
/*!
 * \brief It correctly initializes netgen library at constructor and
 *        correctly finishes using netgen library at destructor
 */
//================================================================================

struct NETGENPLUGIN_EXPORT NETGENPlugin_NetgenLibWrapper
{
  bool           _isComputeOk;
  netgen::Mesh * _ngMesh;

  NETGENPlugin_NetgenLibWrapper();
  ~NETGENPlugin_NetgenLibWrapper();
  void setMesh( nglib::Ng_Mesh* mesh );
  nglib::Ng_Mesh* ngMesh() { return (nglib::Ng_Mesh*)(void*)_ngMesh; }



  static int GenerateMesh(netgen::OCCGeometry& occgeo, int startWith, int endWith,
                          netgen::Mesh* & ngMesh);
  int GenerateMesh(netgen::OCCGeometry& occgeo, int startWith, int endWith )
  {
    return GenerateMesh( occgeo, startWith, endWith, _ngMesh );
  }

  static void CalcLocalH( netgen::Mesh * ngMesh );

  static void RemoveTmpFiles();
  static int& instanceCounter();
  void setOutputFile(std::string);

 private:
  std::string getOutputFileName();
  void        removeOutputFile();
  std::string _outputFileName;
  // This will change current directory when the class is instanciated and switch
  ChdirRAII _tmpDir;


  ostream *       _ngcout;
  ostream *       _ngcerr;
  std::streambuf* _coutBuffer;   // to re-/store cout.rdbuf()
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
  ~NETGENPlugin_Mesher();
  void SetSelfPointer( NETGENPlugin_Mesher ** ptr );

  void SetParameters(const NETGENPlugin_Hypothesis*          hyp);
  void SetParameters(const NETGENPlugin_SimpleHypothesis_2D* hyp);
  void SetParameters(const StdMeshers_ViscousLayers*         hyp );
  void SetViscousLayers2DAssigned(bool isAssigned) { _isViscousLayers2D = isAssigned; }

  void SetLocalSizeForChordalError( netgen::OCCGeometry& occgeo, netgen::Mesh& ngMesh );
  static void SetLocalSize( netgen::OCCGeometry& occgeo, netgen::Mesh& ngMesh );

  
/**
 * @brief InitialSetup. Fill occgeo map with geometrical objects not meshed. Fill meshdSM with the already computed
 *        submeshes, and mesh the internal edges so faces with internal are eventurally properly meshed.
 *        Define the class members _ngMesh and _occgeom
 */
  void InitialSetup( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo, 
                      list< SMESH_subMesh* >* meshedSM, NETGENPlugin_Internals* internals, 
                      SMESH_MesherHelper &quadHelper, NETGENPlugin_ngMeshInfo& initState, netgen::MeshingParameters &mparams );

  void InitialSetupSA( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo, 
                      list< SMESH_subMesh* >* meshedSM, NETGENPlugin_Internals* internals, 
                      SMESH_MesherHelper &quadHelper, NETGENPlugin_ngMeshInfo& initState, 
                      netgen::MeshingParameters &mparams, bool useFMapFunction = false );

  void SetBasicMeshParameters( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::MeshingParameters &mparams, netgen::OCCGeometry& occgeo );
  void SetBasicMeshParametersFor2D( netgen::OCCGeometry& occgeo, vector< const SMDS_MeshNode* >& nodeVec, 
                                      netgen::MeshingParameters &mparams, NETGENPlugin_Internals* internals, 
                                        NETGENPlugin_ngMeshInfo& initState );
  void SetBasicMeshParametersFor3D( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo, 
                                      vector< const SMDS_MeshNode* >& nodeVec, netgen::MeshingParameters &mparams, 
                                        NETGENPlugin_Internals* internals, NETGENPlugin_ngMeshInfo& initState, SMESH_MesherHelper &quadHelper, 
                                          SMESH_Comment& comment );
  void CallNetgenConstAnalysis( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::MeshingParameters &mparams, netgen::OCCGeometry& occgeo );
  int CallNetgenMeshEdges( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo );
  int CallNetgenMeshFaces( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo, SMESH_Comment& comment );
  int CallNetgenMeshVolumens( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo, SMESH_Comment& comment );
  void MakeSecondOrder( netgen::MeshingParameters &mparams, netgen::OCCGeometry& occgeo, 
                          list< SMESH_subMesh* >* meshedSM, NETGENPlugin_ngMeshInfo& initState, SMESH_Comment& comment );
  int FillInternalElements( NETGENPlugin_NetgenLibWrapper& ngLib, NETGENPlugin_Internals& internals, netgen::OCCGeometry& occgeo,
                              NETGENPlugin_ngMeshInfo& initState, SMESH_MesherHelper &quadHelper, list< SMESH_subMesh* >* meshedSM );
  bool Fill2DViscousLayer( netgen::OCCGeometry& occgeo, vector< const SMDS_MeshNode* >& nodeVec, 
                            NETGENPlugin_Internals* internals, NETGENPlugin_ngMeshInfo& initState );

  bool Fill3DViscousLayerAndQuadAdaptor( netgen::OCCGeometry& occgeo, vector< const SMDS_MeshNode* >& nodeVec, 
                                          netgen::MeshingParameters &mparams, NETGENPlugin_ngMeshInfo& initState,
                                          list< SMESH_subMesh* >* meshedSM, SMESH_MesherHelper &quadHelper, int & err );

  int Fill0D1DElements( netgen::OCCGeometry& occgeo, vector< const SMDS_MeshNode* >& nodeVec, list< SMESH_subMesh* >* meshedSM, SMESH_MesherHelper &quadHelper );
  void FillSMESH( netgen::OCCGeometry& occgeo, NETGENPlugin_ngMeshInfo& initState, vector< const SMDS_MeshNode* >& nodeVec, SMESH_MesherHelper &quadHelper, SMESH_Comment& comment );
  ///// End definition methods to rewrite function
  
  enum DIM {
    D1 = 1,
    D2,
    D3
  };       
  
  bool Compute();
  bool Compute( NETGENPlugin_NetgenLibWrapper& ngLib, vector< const SMDS_MeshNode* >& nodeVec, bool write2SMESH, DIM dim );

  bool Evaluate(MapShapeNbElems& aResMap);

  double GetProgress(const SMESH_Algo* holder,
                     const int *       algoProgressTic,
                     const double *    algoProgress) const;

  static void PrepareOCCgeometry(netgen::OCCGeometry&          occgeom,
                                 const TopoDS_Shape&           shape,
                                 SMESH_Mesh&                   mesh,
                                 std::list< SMESH_subMesh* > * meshedSM=0,
                                 NETGENPlugin_Internals*       internalShapes=0);

  static double GetDefaultMinSize(const TopoDS_Shape& shape,
                                  const double        maxSize);

  static void RestrictLocalSize(netgen::Mesh& ngMesh,
                                const gp_XYZ& p,
                                double        size,
                                const bool    overrideMinH=true);

  static int FillSMesh(const netgen::OCCGeometry&          occgeom,
                       netgen::Mesh&                       ngMesh,
                       const NETGENPlugin_ngMeshInfo&      initState,
                       SMESH_Mesh&                         sMesh,
                       std::vector<const SMDS_MeshNode*>&  nodeVec,
                       SMESH_Comment&                      comment,
                       SMESH_MesherHelper*                 quadHelper=0);

  bool FillNgMesh(netgen::OCCGeometry&                occgeom,
                  netgen::Mesh&                       ngMesh,
                  std::vector<const SMDS_MeshNode*>&  nodeVec,
                  const std::list< SMESH_subMesh* > & meshedSM,
                  SMESH_MesherHelper*                 quadHelper=0,
                  SMESH_ProxyMesh::Ptr                proxyMesh=SMESH_ProxyMesh::Ptr());

  static void FixIntFaces(const netgen::OCCGeometry& occgeom,
                          netgen::Mesh&              ngMesh,
                          NETGENPlugin_Internals&    internalShapes);

  static bool FixFaceMesh(const netgen::OCCGeometry& occgeom,
                          netgen::Mesh&              ngMesh,
                          const int                  faceID);

  static void AddIntVerticesInFaces(const netgen::OCCGeometry&          occgeom,
                                    netgen::Mesh&                       ngMesh,
                                    std::vector<const SMDS_MeshNode*>&  nodeVec,
                                    NETGENPlugin_Internals&             internalShapes);

  static void AddIntVerticesInSolids(const netgen::OCCGeometry&         occgeom,
                                    netgen::Mesh&                       ngMesh,
                                    std::vector<const SMDS_MeshNode*>&  nodeVec,
                                    NETGENPlugin_Internals&             internalShapes);

  static SMESH_ComputeErrorPtr
    AddSegmentsToMesh(netgen::Mesh&                         ngMesh,
                      netgen::OCCGeometry&                  geom,
                      const TSideVector&                    wires,
                      SMESH_MesherHelper&                   helper,
                      std::vector< const SMDS_MeshNode* > & nodeVec,
                      const bool                            overrideMinH=true);

  void SetDefaultParameters();

  static SMESH_ComputeErrorPtr ReadErrors(const std::vector< const SMDS_MeshNode* >& nodeVec);


  static void toPython( const netgen::Mesh* ngMesh ); // debug

 private:

  bool Compute1D( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo );

  bool Compute2D( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo,
                  netgen::MeshingParameters &mparams, list< SMESH_subMesh* >* meshedSM, 
                  NETGENPlugin_ngMeshInfo& initState, NETGENPlugin_Internals* internals, 
                  vector< const SMDS_MeshNode* >& nodeVec, SMESH_Comment& comment, DIM dim );
                  
  bool Compute3D( NETGENPlugin_NetgenLibWrapper& ngLib, netgen::OCCGeometry& occgeo,
                  netgen::MeshingParameters &mparams, list< SMESH_subMesh* >* meshedSM, 
                  NETGENPlugin_ngMeshInfo& initState, NETGENPlugin_Internals* internals, 
                  vector< const SMDS_MeshNode* >& nodeVec, SMESH_MesherHelper &quadHelper, 
                  SMESH_Comment& comment);

  SMESH_Mesh*          _mesh;
  const TopoDS_Shape&  _shape;
  bool                 _isVolume;
  bool                 _optimize;
  int                  _fineness;
  bool                 _isViscousLayers2D;
  double               _chordalError;
  netgen::Mesh*        _ngMesh;
  netgen::OCCGeometry* _occgeom;

  int                  _curShapeIndex;
  volatile int         _progressTic;
  volatile double      _ticTime; // normalized [0,1] compute time per a SMESH_Algo::_progressTic
  volatile double      _totalTime;

  const NETGENPlugin_SimpleHypothesis_2D * _simpleHyp;
  const StdMeshers_ViscousLayers*   _viscousLayersHyp;

  // a pointer to NETGENPlugin_Mesher* field of the holder, that will be
  // nullified at destruction of this
  NETGENPlugin_Mesher ** _ptrToMe;
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
 *
 * For internal faces a more simple solution is found, which is just to duplicate
 * mesh faces on internal geom faces without modeling a "real crack". For this
 * reason findBorderElements() is no more used anywhere.
 */
//=============================================================================

class NETGENPLUGIN_EXPORT NETGENPlugin_Internals
{
  SMESH_Mesh&       _mesh;
  bool              _is3D;
  //2D
  std::map<int,int> _e2face;//!<edges and their vertices in faces where they are TopAbs_INTERNAL
  std::map<int,std::list<int> > _f2v;//!<faces with internal vertices
  // 3D
  std::set<int>     _intShapes;
  std::set<int>     _borderFaces; //!< non-internal faces sharing the internal edge
  std::map<int,std::list<int> > _s2v;//!<solids with internal vertices

public:
  NETGENPlugin_Internals( SMESH_Mesh& mesh, const TopoDS_Shape& shape, bool is3D );

  SMESH_Mesh& getMesh() const;

  bool isShapeToPrecompute(const TopoDS_Shape& s);

  // 2D meshing
  // edges
  bool hasInternalEdges() const { return !_e2face.empty(); }
  bool isInternalEdge( int id ) const { return _e2face.count( id ); }
  const std::map<int,int>& getEdgesAndVerticesWithFaces() const { return _e2face; }
  void getInternalEdges( TopTools_IndexedMapOfShape&  fmap,
                         TopTools_IndexedMapOfShape&  emap,
                         TopTools_IndexedMapOfShape&  vmap,
                         std::list< SMESH_subMesh* > smToPrecompute[]);
  // vertices
  bool hasInternalVertexInFace() const { return !_f2v.empty(); }
  const std::map<int,std::list<int> >& getFacesWithVertices() const { return _f2v; }

  // 3D meshing
  // faces
  bool hasInternalFaces() const { return !_intShapes.empty(); }
  bool isInternalShape( int id ) const { return _intShapes.count( id ); }
  void findBorderElements( std::set< const SMDS_MeshElement*, TIDCompare > & borderElems );
  bool isBorderFace( int faceID ) const { return _borderFaces.count( faceID ); }
  void getInternalFaces( TopTools_IndexedMapOfShape&  fmap,
                         TopTools_IndexedMapOfShape&  emap,
                         std::list< SMESH_subMesh* >& facesSM,
                         std::list< SMESH_subMesh* >& boundarySM);
  // vertices
  bool hasInternalVertexInSolid() const { return !_s2v.empty(); }
  bool hasInternalVertexInSolid(int soID ) const { return _s2v.count(soID); }
  const std::map<int,std::list<int> >& getSolidsWithVertices() const { return _s2v; }


};

#endif
