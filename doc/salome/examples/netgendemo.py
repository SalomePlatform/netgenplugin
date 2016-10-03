# 2d and 3d mesh generation with NETGEN

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBoxDXDYDZ(10., 10., 10.)
geompy.addToStudy(box, "Box")


# 1. Create a triangular 2D mesh on the box with NETGEN_1D2D algorithm
triaN = smesh.Mesh(box, "Box : triangular mesh by NETGEN_1D2D")

# create a NETGEN_1D2D algorithm for solids
algo2D = triaN.Triangle(smeshBuilder.NETGEN_1D2D)

# define hypotheses
n12_params = algo2D.Parameters()

# define number of segments
n12_params.SetNbSegPerEdge(19)

# define max element
n12_params.SetMaxSize(300)

# 2. Create a tetrahedral mesh on the box with NETGEN_1D2D3D algorithm (full netgen)
tetraN = smesh.Mesh(box, "Box : tetrahedrical mesh by NETGEN_1D2D3D")

# create a NETGEN_1D2D3D algorithm for solids
algo3D = tetraN.Tetrahedron(smeshBuilder.FULL_NETGEN)

# define hypotheses
n123_params = algo3D.Parameters()

# define number of segments
n123_params.SetNbSegPerEdge(11)

# define max element size
n123_params.SetMaxSize(300)

# compute the meshes
triaN.Compute()
tetraN.Compute()
