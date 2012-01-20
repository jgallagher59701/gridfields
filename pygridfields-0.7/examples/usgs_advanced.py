#from pkg_resources import require
#require("gridfield>0.2")
#from gridfield import *

#
#
import matplotlib.pyplot as plt
from matplotlib import tri

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

NumPhysNodes=len(x1)
#print nv[1][:]
import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Wrap
#g = gf.ZeroGrid("points",NumPhysNodes)

g = gf.Grid("points",2)
g.setImplicit0Cells(NumPhysNodes);
twocells = gf.CellArray()
NumPhysElems=len(nv)
for i in range(1,NumPhysElems+1):
 twocells.addCellNodes(nv[i-1][:], 3)

# print NumPhysNodes
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
rG = Restrict("abs(h)<=500.0", 0, G)

# restrict to lon/lat range
bounds = (-71.5,-63.0,39.5,46.0)
condition = "(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds

# Apply the condition to the vertices
print "Apply condition %s" % condition
rG = Restrict(condition, 0, rG)

# execute the recipe
Result = rG.getResult()

ax = Result.GetAttribute(0, "h")

# print the result
ax.show()

print "Attributes of vertices: ", Result.GetScheme(0).asString()

# Gridfields x,y,z to Numpy
d0 = Result.GetDataset(0)
lstvals = [gf.derefFloat(d0.GetAttributeVal("x", i)) for i in range(d0.Size())]
x = np.array(lstvals)
lstvals = [gf.derefFloat(d0.GetAttributeVal("y", i)) for i in range(d0.Size())]
y = np.array(lstvals)
lstvals = [gf.derefFloat(d0.GetAttributeVal("h", i)) for i in range(d0.Size())]
h = np.array(lstvals)

# Gridfields ele connectivity array to Numpy
newG   = Result.GetGrid()
newG.normalize()   # reindex the connectiviy array to the restricted grid
twocells = gf.castToCellArray(newG.getKCells(2))
ele=np.zeros((twocells.getsize(), 3))

for i in range(twocells.getsize()):
 cell = twocells.getCell(i)
 ele[i][:]=[cell.getnode(0), cell.getnode(1), cell.getnode(2)]

Gp=Result.GetScheme(0)


t=tri.Triangulation(x,y,triangles=ele)

plt.figure(1)
plt.tricontour(t, h, 15, linewidths=0.5, colors='k')
plt.tricontourf(t, h, 15, cmap=plt.cm.jet)
plt.title('tri.Triangulation: tricontour ')
plt.colorbar()
plt.show()
