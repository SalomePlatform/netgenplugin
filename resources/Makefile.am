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

# -* Makefile *- 
# Author : Patrick GOLDBRONN (CEA)
# Date : 28/06/2001
#  Modified by : Alexander BORODIN (OCN) - autotools usage
# $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

dist_salomeres_DATA =		\
	NETGENPlugin.xml	\
	SalomeApp.xml

if NETGENPLUGIN_ENABLE_GUI
  dist_salomeres_DATA +=		\
	mesh_algo_netgen_2d.png		\
	mesh_algo_netgen_2d3d.png	\
	mesh_hypo_netgen.png		\
	mesh_hypo_netgen_2d.png		\
	mesh_tree_algo_netgen_2d.png	\
	mesh_tree_algo_netgen_2d3d.png	\
	mesh_tree_hypo_netgen.png	\
	mesh_tree_hypo_netgen_2d.png	\
	mesh_tree_algo_netgen.png
endif
