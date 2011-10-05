#from pkg_resources import require
#require("gridfield>0.2")
#from gridfield import *

#
#
import os.path, sys

import numpy as np
import netCDF4
URL = 'http://geoport.whoi.edu/thredds/dodsC/usgs/data1/rsignell/models/adcirc/fort.64.nc'
#from Scientific.IO import NetCDF


# Usually we will read gridfields from various 
# file formats.  Here, we construct one from scratch
# to illustrate the low-level interface

# A Grid is toplogy only; no data
# Construct a new grid named "points" 
# ZeroGrid is syntactic sugar for a zero dimensional grid
# A 0-d grid is a grid consisting of points only


try:
    nc = netCDF4.Dataset(URL)
    x=nc.variables['x']
    y=nc.variables['y']
    # read water depth at nodes
    h=nc.variables['depth']
    # read connectivity array
    nv=nc.variables['element']
    x1=x[:]
    y1=y[:]
    h1=h[:]
    nv=nv[:]-1
except:
    N = 50
    x1 = np.random.random(N)
    y1 = np.random.random(N)
    h1 = np.random.random(N)
NumPhysNodes=len(x1)
import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Wrap
#g = gf.ZeroGrid("points",NumPhysNodes)

g = gf.Grid("points",2)
g.setImplicit0Cells(NumPhysNodes);
twocells = gf.CellArray()
NumPhysElems=len(nv)
for i in range(1,NumPhysElems+1):
 twocells.addCellNodes(nv[i-1][:], 3)

print NumPhysNodes
# construct an array of X-values

x = gf.Array("x", gf.FLOAT, NumPhysNodes)
x.copyFloatData(x1,NumPhysNodes)


# construct an array of Y-values
y = gf.Array("y", gf.FLOAT, NumPhysNodes)
y.copyFloatData(y1,NumPhysNodes)

# construct an array of Y-values
h = gf.Array("h", gf.FLOAT, NumPhysNodes)
h.copyFloatData(h1,NumPhysNodes)

#twocells.buildIncidenceIndex()
# GridField = Topology + Data
#           = Grid + Arrays

# Construct the gridfield

g.setKCells(twocells, 2);
G = gf.GridField(g)

# Attach the data
G.Bind(0, x)
G.Bind(0, y)
G.Bind(0, h)


# Now we can use some operators to manipulate the data
# Operator expressions are chained together
# nothign actually happens until the expression is evaluated

# Normally we construct a gridfield implicitly
# with the "Scan" operator.
# Since we constrcuted ours from scratch,
# so we lift the raw gridfield into an operator
G = Wrap(G)

# calculate a new attribute
#aG = Apply("root=abs(q)", 2, G)
#H = aG.getResult()
#H.show()

# restrict using the data at rank 0
rG = Restrict("abs(h)>=0.0", 0, G)

# execute the recipe
Result = rG.getResult()

ax = Result.GetAttribute(0, "h")


print "Attributes of vertices: ", Result.GetScheme(0).asString()


Gp=Result.GetScheme(0)
try:
  import vtk
except:
  print "VTK not installed -- no visualization attempted"
  sys.exit()

import gridfield.vis as vis
from gridfield.vis import ToVTK

# Show the gridfield, making whatever assumptions necessary
# color by the "h" attribute

vis.ToVTK(Result, "h", show=True,save=True, capture=False, fname="vis.png")

#vis.Visualize(ax, "q")

# To save the image to a file and not interact with it, do this:
# vis.ToVTK(rH, "h", capture=True, fname="depth.png")

