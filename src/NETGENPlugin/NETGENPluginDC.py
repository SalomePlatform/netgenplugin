# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

from smesh import Mesh_Algorithm, AssureGeomPublished, ParseParameters, IsEqual

# import NETGENPlugin module if possible
noNETGENPlugin = 0
try:
    import NETGENPlugin
except ImportError:
    noNETGENPlugin = 1
    pass

# Types of algorithms
NETGEN_3D     = "NETGEN_3D"
NETGEN_1D2D3D = "NETGEN_2D3D"
NETGEN_1D2D   = "NETGEN_2D"
NETGEN_2D     = "NETGEN_2D_ONLY"
NETGEN_FULL   = NETGEN_1D2D3D
NETGEN        = NETGEN_3D
FULL_NETGEN   = NETGEN_FULL

SOLE   = 0
SIMPLE = 1

# Fineness enumeration
VeryCoarse = 0
Coarse     = 1
Moderate   = 2
Fine       = 3
VeryFine   = 4
Custom     = 5

## Base of all NETGEN algorithms.
#
class NETGEN_Algorithm(Mesh_Algorithm):

    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        if noNETGENPlugin: print "Warning: NETGENPlugin module unavailable"
        self.Create(mesh, geom, self.algoType, "libNETGENEngine.so")
        self.params = None

    ## Sets MaxSize
    #
    def SetMaxSize(self, theSize):
        if self.Parameters():
            self.params.SetMaxSize(theSize)

    ## Sets MinSize
    #
    def SetMinSize(self, theSize):
        if self.Parameters():
            self.params.SetMinSize(theSize)


    ## Sets Optimize flag
    #
    def SetOptimize(self, theVal):
        if self.Parameters():
            self.params.SetOptimize(theVal)

    ## Sets Fineness
    #  @param theFineness is:
    #  VeryCoarse, Coarse, Moderate, Fine, VeryFine or Custom
    #
    def SetFineness(self, theFineness):
        if self.Parameters():
            self.params.SetFineness(theFineness)

    ## Sets GrowthRate
    #
    def SetGrowthRate(self, theRate):
        if self.Parameters():
            self.params.SetGrowthRate(theRate)

    ## Defines hypothesis having several parameters
    #
    def Parameters(self, which=SOLE):
        if self.algoType == NETGEN_1D2D:
            if which == SIMPLE:
                hypType = "NETGEN_SimpleParameters_2D"
            else:
                hypType = "NETGEN_Parameters_2D"
        elif self.algoType == NETGEN_1D2D3D:
            if which == SIMPLE:
                hypType = "NETGEN_SimpleParameters_3D"
            else:
                hypType = "NETGEN_Parameters"
        elif self.algoType == NETGEN_2D:
            hypType = "NETGEN_Parameters_2D_ONLY"
        else:
            hypType = "NETGEN_Parameters_3D"

        if self.params and self.params.GetName() != hypType:
            self.mesh.RemoveHypothesis( self.params, self.geom )
            self.params = None
        if not self.params:
            self.params = self.Hypothesis(hypType, [],"libNETGENEngine.so",UseExisting=0)

        return self.params



## Defines a tetrahedron 1D-2D-3D algorithm
#  It is created by calling Mesh.Triangle( NETGEN_1D2D3D, geom=0 )
#
class NETGEN_1D2D3D_Algorithm(NETGEN_Algorithm):

    meshMethod = "Tetrahedron"
    algoType   = NETGEN_1D2D3D

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        NETGEN_Algorithm.__init__(self, mesh, geom)

    ## Sets SecondOrder flag
    #
    def SetSecondOrder(self, theVal):
        if self.Parameters():
            self.params.SetSecondOrder(theVal)

    ## Sets NbSegPerEdge
    #
    def SetNbSegPerEdge(self, theVal):
        if self.Parameters():
            self.params.SetNbSegPerEdge(theVal)

    ## Sets NbSegPerRadius
    #
    def SetNbSegPerRadius(self, theVal):
        if self.Parameters():
            self.params.SetNbSegPerRadius(theVal)

    ## Sets QuadAllowed flag.
    def SetQuadAllowed(self, toAllow=True):
        if self.Parameters():
            self.params.SetQuadAllowed(toAllow)


    ## Sets number of segments overriding the value set by SetLocalLength()
    #
    def SetNumberOfSegments(self, theVal):
        self.Parameters(SIMPLE).SetNumberOfSegments(theVal)

    ## Sets number of segments overriding the value set by SetNumberOfSegments()
    #
    def SetLocalLength(self, theVal):
        self.Parameters(SIMPLE).SetLocalLength(theVal)

    ## Defines "MaxElementArea" parameter of NETGEN_SimpleParameters_3D hypothesis.
    #  Overrides value set by LengthFromEdges()
    def MaxElementArea(self, area):
        self.Parameters(SIMPLE).SetMaxElementArea(area)

    ## Defines "LengthFromEdges" parameter of NETGEN_SimpleParameters_3D hypothesis
    #  Overrides value set by MaxElementArea()
    def LengthFromEdges(self):
        self.Parameters(SIMPLE).LengthFromEdges()

    ## Defines "LengthFromFaces" parameter of NETGEN_SimpleParameters_3D hypothesis
    #  Overrides value set by MaxElementVolume()
    def LengthFromFaces(self):
        self.Parameters(SIMPLE).LengthFromFaces()

    ## Defines "MaxElementVolume" parameter of NETGEN_SimpleParameters_3D hypothesis
    #  Overrides value set by LengthFromFaces()
    def MaxElementVolume(self, vol):
        self.Parameters(SIMPLE).SetMaxElementVolume(vol)


## Triangle NETGEN 1D-2D algorithm. 
#  It is created by calling Mesh.Triangle( NETGEN_1D2D, geom=0 )
#
class NETGEN_1D2D_Algorithm(NETGEN_1D2D3D_Algorithm):

    meshMethod = "Triangle"
    algoType   = NETGEN_1D2D

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        NETGEN_1D2D3D_Algorithm.__init__(self, mesh, geom)



## Triangle NETGEN 2D algorithm
#  It is created by calling Mesh.Triangle( NETGEN_2D, geom=0 )
#
class NETGEN_2D_Only_Algorithm(NETGEN_Algorithm):

    meshMethod = "Triangle"
    algoType = NETGEN_2D
    
    ## Private constructor.
    def __init__(self, mesh, geom=0):
        NETGEN_Algorithm.__init__(self, mesh, geom)

    ## Defines "MaxElementArea" hypothesis basing on the definition of the maximum area of each triangle
    #  @param area for the maximum area of each triangle
    #  @param UseExisting if ==true - searches for an  existing hypothesis created with the
    #                     same parameters, else (default) - creates a new one
    #
    def MaxElementArea(self, area, UseExisting=0):
        compFun = lambda hyp, args: IsEqual(hyp.GetMaxElementArea(), args[0])
        hyp = self.Hypothesis("MaxElementArea", [area], UseExisting=UseExisting,
                              CompareMethod=compFun)
        hyp.SetMaxElementArea(area)
        return hyp

    ## Defines "LengthFromEdges" hypothesis to build triangles
    #  based on the length of the edges taken from the wire
    #
    def LengthFromEdges(self):
        hyp = self.Hypothesis("LengthFromEdges", UseExisting=1, CompareMethod=self.CompareEqualHyp)
        return hyp

    ## Sets QuadAllowed flag.
    def SetQuadAllowed(self, toAllow=True):
        if not self.params:
            # use simple hyps
            hasSimpleHyps = False
            simpleHyps = ["QuadranglePreference","LengthFromEdges","MaxElementArea"]
            for hyp in self.mesh.GetHypothesisList( self.geom ):
                if hyp.GetName() in simpleHyps:
                    hasSimpleHyps = True
                    if hyp.GetName() == "QuadranglePreference":
                        if not toAllow: # remove QuadranglePreference
                            self.mesh.RemoveHypothesis( self.geom, hyp )
                        else:
                            return hyp
                        return None
                    pass
                pass
            if hasSimpleHyps:
                if toAllow: # add QuadranglePreference
                    return self.Hypothesis("QuadranglePreference", UseExisting=1, CompareMethod=self.CompareEqualHyp)
                return None
            pass
        self.Parameters().SetQuadAllowed( toAllow )
        return self.params

## Defines a tetrahedron 3D algorithm
#  It is created by calling Mesh.Tetrahedron()
#
class NETGEN_3D_Algorithm(NETGEN_Algorithm):

    meshMethod = "Tetrahedron"
    algoType   = NETGEN
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        NETGEN_Algorithm.__init__(self, mesh, geom)

    ## Defines "MaxElementVolume" hypothesis to give the maximun volume of each tetrahedron
    #  @param vol for the maximum volume of each tetrahedron
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                   the same parameters, else (default) - creates a new one
    def MaxElementVolume(self, vol, UseExisting=0):
        compFun = lambda hyp, args: IsEqual(hyp.GetMaxElementVolume(), args[0])
        hyp = self.Hypothesis("MaxElementVolume", [vol], UseExisting=UseExisting,
                              CompareMethod=compFun)
        hyp.SetMaxElementVolume(vol)
        return hyp


# Class just to create NETGEN_1D2D by calling Mesh.Triangle(NETGEN)
class NETGEN_1D2D_Algorithm_2(NETGEN_1D2D_Algorithm):

    algoType = NETGEN

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        self.algoType = NETGEN_1D2D
        NETGEN_1D2D_Algorithm.__init__(self,mesh, geom)


# Class just to create NETGEN_1D2D3D by calling Mesh.Netgen()
class NETGEN_1D2D3D_Algorithm_2(NETGEN_1D2D3D_Algorithm):

    meshMethod = "Netgen"

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        NETGEN_1D2D3D_Algorithm.__init__(self,mesh, geom)
