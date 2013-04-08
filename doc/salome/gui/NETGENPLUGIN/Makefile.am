# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

#  File   : Makefile.in
#  Author : Vasily Rusyaev (Open Cascade NN)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : doc
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST += images input static/footer.html static/doxygen.css

guidocdir = $(docdir)/gui/NETGENPLUGIN
guidoc_DATA = images/head.png

DOC_PYTHONPATH=$(prefix)/bin/salome:$(prefix)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(SMESH_ROOT_DIR)/bin/salome:$(SMESH_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(MED_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(GEOM_ROOT_DIR)/bin/salome:$(GEOM_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/bin/salome:$(KERNEL_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(OMNIORB_ROOT)/lib/python$(PYTHON_VERSION)/site-packages:$(OMNIORB_ROOT)/lib64/python$(PYTHON_VERSION)/site-packages
DOC_LD_LIBRARY_PATH=$(prefix)/lib/salome:$(SMESH_ROOT_DIR)/lib/salome:$(MED_ROOT_DIR)/lib/salome:$(GEOM_ROOT_DIR)/lib/salome:$(KERNEL_ROOT_DIR)/lib/salome
DOC_SMESH_MeshersList=NETGENPlugin

tmp/smeshBuilder.py: $(top_srcdir)/src/NETGENPlugin/NETGENPluginBuilder.py
	@mkdir -p tmp && PYTHONPATH=$(DOC_PYTHONPATH):${PYTHONPATH} LD_LIBRARY_PATH=$(DOC_LD_LIBRARY_PATH):${LD_LIBRARY_PATH} SMESH_MeshersList=$(DOC_SMESH_MeshersList) $(PYTHON) $(SMESH_ROOT_DIR)/bin/salome/collect_mesh_methods.py -d -o $@ NETGENPlugin

tmp/NETGENPluginBuilder.py: $(top_srcdir)/src/NETGENPlugin/NETGENPluginBuilder.py
	@mkdir -p tmp && $(KERNEL_ROOT_DIR)/bin/salome/prepare_generating_doc.py -o $@ $<

tmp/smesh_algorithm.py : @SMESH_ROOT_DIR@/lib/python@PYTHON_VERSION@/site-packages/salome/salome/smesh/smesh_algorithm.py
	@mkdir -p tmp && $(KERNEL_ROOT_DIR)/bin/salome/prepare_generating_doc.py -o $@ $<

usr_docs: doxyfile_py doxyfile tmp/smeshBuilder.py tmp/NETGENPluginBuilder.py tmp/smesh_algorithm.py
	@$(DOXYGEN) doxyfile_py ; \
	$(DOXYGEN) doxyfile

docs: usr_docs

clean-local:
	@for filen in `find . -maxdepth 1` ; do			\
	  case $${filen} in					\
	    ./Makefile | ./doxyfile | ./doxyfile_py ) ;;	\
	    . | .. | ./static ) ;;				\
	    *) echo "Removing $${filen}" ; rm -rf $${filen} ;;	\
	  esac ;						\
	done ;

install-data-local: usr_docs
	$(INSTALL) -d $(DESTDIR)$(docdir)/gui/NETGENPLUGIN
	@for filen in `find . -maxdepth 1` ; do							\
	  case $${filen} in									\
	    ./Makefile | ./doxyfile | ./doxyfile_py ) ;;					\
	    ./doxyfile.bak | ./doxyfile_py.bak ) ;;						\
	    . | .. | ./static | ./tmp ) ;;							\
	    *) echo "Installing $${filen}" ; cp -rp $${filen} $(DESTDIR)$(docdir)/gui/NETGENPLUGIN ;;	\
	  esac ;										\
	done ;
	cp -rp $(srcdir)/images/head.png $(DESTDIR)$(docdir)/gui/NETGENPLUGIN/netgenpluginpy_doc/ ;

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/gui/NETGENPLUGIN