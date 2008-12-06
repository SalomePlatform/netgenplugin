#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# -* Makefile *- 
# Author : Edward AGAPOV (OCC)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
# Module : NETGENPLUGIN
# Date : 10/01/2004
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# build .so of NETGEN itself

lib_LTLIBRARIES = libNETGEN.la

LIB_OBJ_O = \
	linopt.o bfgs.o linsearch.o global.o bisect.o meshtool.o refine.o ruler3.o \
	improve3.o adfront3.o tetrarls.o prism2rls.o profiler.o pyramidrls.o pyramid2rls.o \
	netrule3.o ruler2.o meshclass.o improve2.o adfront2.o netrule2.o triarls.o geomsearch.o \
	secondorder.o meshtype.o parser3.o quadrls.o specials.o parser2.o meshing2.o meshing3.o \
	meshfunc.o localh.o improve2gen.o delaunay.o boundarylayer.o msghandler.o \
	meshfunc2d.o smoothing2.o smoothing3.o topology.o curvedelems_new.o clusters.o zrefine.o \
	ngexception.o geomtest3d.o geom2d.o geom2dmesh.o geom3d.o adtree.o transform3d.o geomfuncs.o \
	polynomial.o densemat.o vector.o basemat.o sparsmat.o algprim.o brick.o manifold.o bspline2d.o \
	meshsurf.o csgeom.o polyhedra.o curve2d.o singularref.o edgeflw.o solid.o explicitcurve2d.o \
	specpoin.o gencyl.o revolution.o genmesh.o genmesh2d.o spline3d.o surface.o identify.o \
	triapprox.o meshstlsurface.o stlline.o stltopology.o stltool.o stlgeom.o stlgeomchart.o \
	stlgeommesh.o table.o optmem.o spbita2d.o hashtabl.o sort.o flags.o seti.o bitarray.o \
	array.o symbolta.o mystring.o moveablemem.o spline.o splinegeometry.o ngnewdelete.o nglib.o \
	hprefinement.o Partition_Inter2d.o Partition_Loop.o Partition_Loop3d.o Partition_Inter3d.o \
	Partition_Loop2d.o Partition_Spliter.o occgeom.o occgenmesh.o occmeshsurf.o

NETGEN_LIBS_DIR=@NETGEN_LIBS_DIR@

$(LIB_OBJ_O):
	ar x $(NETGEN_LIBS_DIR)/libnginterface.a
	ar x $(NETGEN_LIBS_DIR)/libocc.a
	ar x $(NETGEN_LIBS_DIR)/libcsg.a
	ar x $(NETGEN_LIBS_DIR)/libgprim.a
	ar x $(NETGEN_LIBS_DIR)/libmesh.a
	ar x $(NETGEN_LIBS_DIR)/libopti.a
	ar x $(NETGEN_LIBS_DIR)/libgen.a
	ar x $(NETGEN_LIBS_DIR)/libla.a
	ar x $(NETGEN_LIBS_DIR)/libstlgeom.a
	ar x $(NETGEN_LIBS_DIR)/libgeom2d.a
	rm -rf csgparser.o dynamicmem.o extrusion.o occconstruction.o parthreads.o readuser.o writeabaqus.o writediffpack.o writeelmer.o writefeap.o writefluent.o writegmsh.o writejcm.o writepermas.o writetecplot.o writetochnog.o writeuser.o wuchemnitz.o

#LIB_OBJ_LO = $(LIB_OBJ_O:%.o=%.lo)
#%.lo: %.o; @ln -s $< $@

nodist_libNETGEN_la_SOURCES =

libNETGEN_la_LDFLAGS  = $(LIB_OBJ_O) \
	$(CAS_LDPATH) -lTKernel -lTKBRep -lTKShHealing -lTKSTEP -lTKXSBase -lTKIGES -lTKMesh -lTKSTL $(FCLIBS) $(STDLIB)

libNETGEN_la_DEPENDENCIES = $(LIB_OBJ_O)

EXTRA_DIST += \
	ReadMeForNgUsers	\
	netgen43ForSalome.patch	\
	netgen45ForSalome.patch	\
	netgen45lib64.patch
