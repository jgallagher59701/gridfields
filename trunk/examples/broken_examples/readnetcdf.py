import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Aggregate, Wrap, Sift

# This script uses a fairly low-level NetCDFAdaptor interface
# to access the variables of a netcdf file.
# The NetCDFAdaptor class holds a variety of convenience
# functions for reading (writing) parts of a GridField 
# from (to) parts of a netcdf file

# Since the netcdf data model does not exactly match the 
# netcdf data model, there are many different special
# cases to consider.  For example, arrays may associate
# cells with corners, edges with corners, cells with edges
# edges with cells, etc.

# The gridfield data model supports all these cases, but
# the current implementation does not always make it 
# convenient.  Therefore, we hide the complexity in
# the NetCDFAdaptor convenience functions.


# == Read a GridField from NetCDF ==

# Open the netcdf file
nc = gf.NetCDFAdaptor("data/corie_sample_06222004.nc")
nc.Open()

# Construct an empty "grid" named "test"
G = gf.Grid("test")

# Derive the 0-cells of the grid from the dimension named "nodes"
# A "0-cell" is just a node/point/vertex/corner.
nc.NodesFromDim("nodes", G)

# This function reads polygons encoded as
# encoded as an array (cell, node) and another array (cell, edge)
# with the assumption that, for a given cell,
# edge i connects node i and node i+1
# "WellSupported" means that both edges and nodes are present.
# For Karen's hydro dataset, the relationship between edges and corners
# is not obvious, so this function does not work
#nc.WellSupportedPolygonsFromVars("cell_corners", "cell_edges", G)

# If we don't care about edges, we can use a more general function.
# Derive the 2-cells from a netcdf array with two dimensions (cells, nodes)
# Assumes all cells have the same number of nodes -- that they are homogeneous
nc.HomogeneousCellsFromVar(2, "cell_vertices", G)

# Build an empty "GridField" from the Grid
Gf = gf.GridField(G)

# Read the vertex coordinates and bind them to the 0-cells
nc.AttributeFromVar(0, "x", Gf)
nc.AttributeFromVar(0, "y", Gf)
nc.AttributeFromVar(0, "salt", Gf)
nc.AttributeFromVar(0, "temp", Gf)

# For technical reasons, we have to wrap the "bare"
# gridfield that we constructed manually as a gridfield 
# operator
wGf = Wrap(Gf)

#wGf.getResult().show()

# A very simple restriction
condition = "(326000<x) & (x<345000)"

rG = Restrict(condition, 0, wGf)

# Apply some arithmetic to the data
A = Apply("saltflux_u=salt*u;saltflux_v=salt*v", 0, rG)

from gridfield.vis import ToVTK
ToVTK(A, "grid_corner_lat2", show=True)

# Evaluate the expression
rGf = A.getResult()

#rGf.show()


# Create an output file
nco = gf.NetCDFAdaptor("result.nc")
nco.Open('w')

# Create dimensions based on grid dimensions
# (name, gridfield source, rank) 
nco.DimFromDim("cells", rGf, 2)
nco.DimFromDim("corners", rGf, 0)

# Create simple dimensions from scratch
nco.CreateDim("cellcorners", 6)
nco.CreateDim("celledges", 6)

# Create variables from grid attributes
# (variable name, gridfield source, rank, list of netcdf dimensions)
nco.VarFromAttribute("grid_center_lat", rGf, 2, ["cells"])
nco.VarFromAttribute("grid_center_lon", rGf, 2, ["cells"])
nco.VarFromAttribute("grid_corner_lat2", rGf, 0, ["corners"])
nco.VarFromAttribute("grid_corner_lon2", rGf, 0, ["corners"])

# Create a variable from the incidence relation between 2-cells and 0-cells
# (name, gridfield source, from rank, to rank, list of netcdf dimensions)
nco.VarFromIncidence("cell_corners", rGf, 2, 0, "cells","cellcorners")
nco.VarFromIncidence("cell_edges", rGf, 2, 1, "cells", "celledges")

# Close the file
nco.Close()
