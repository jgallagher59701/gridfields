#from pkg_resources import require
#require("gridfield>0.2")
#from gridfield import *

#
#
import os.path, sys

import numpy as np
import netCDF4
import struct
URL = 'http://testbedapps.sura.org/thredds/dodsC/ugrid/TestCases/FVCOM/fvcom_delt.ncml'
#from Scientific.IO import NetCDF


# Usually we will read gridfields from various 
# file formats.  Here, we construct one from scratch
# to illustrate the low-level interface

# A Grid is toplogy only; no data
# Construct a new grid named "points" 
# ZeroGrid is syntactic sugar for a zero dimensional grid
# A 0-d grid is a grid consisting of points only


nc = netCDF4.Dataset(URL)
lon=nc.variables['lon_node']
lat=nc.variables['lat_node']

nv=nc.variables['connectivity_with_siglay']
sigma=nc.variables['siglay']
t=nc.variables['temperature_node']
lon1=lon[:]
lat1=lat[:]
nv=nv[:]-1
sig=sigma[:]
t1=t[:];
print "hi"

NumPhysNodes=len(lon1)
print nv[1][:]
import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Wrap
import gridfield.algebra as ga
#g = gf.ZeroGrid("points",NumPhysNodes)

g1=gf.Grid("height",1)

g2 = gf.Grid("points",2)
g2.setImplicit0Cells(NumPhysNodes);
twocells = gf.CellArray()
NumPhysElems=len(nv)
for i in range(1,NumPhysElems+1):
 twocells.addCellNodes(nv[:][i-1], 3)


onecells = gf.CellArray()
NumPhysElems=len(sig)
onecells.addCellNodes(range(0,10),10)

threecells = gf.CellArray()

for i in range(1,NumPhysElems+1):
 for j in range(1,11): 
  threecells.addCellNodes(t1[:,j-1,i-1], 8)

print NumPhysNodes


# construct an array of lon-values

lon = gf.Array("x", gf.FLOAT, NumPhysNodes)
lon.copyFloatData(lon1,NumPhysNodes)



# construct an array of lat-values
lat = gf.Array("y", gf.FLOAT, NumPhysNodes)
lat.copyFloatData(lat1,NumPhysNodes)

h=gf.Array("h",gf.FLOAT,10)
h.copyFloatData(sig[0,:],10)

t1=t1[1,:,:]
t1=t1.reshape(NumPhysNodes*10)

temp=gf.Array("temp",gf.FLOAT,NumPhysNodes*10)
temp.copyFloatData(t1[:],NumPhysNodes*10)

g2.setKCells(twocells, 2);

Surface = gf.GridField(g2)
g1.setKCells(onecells,1);
Depth = gf.GridField(g1)
# Attach the data
Surface.Bind(0, lon)
Surface.Bind(0, lat)
Depth.Bind(0,h)
Surface=Wrap(Surface);
Depth=Wrap(Depth);
ThreeD=ga.Cross(Surface,Depth)
Answer = ThreeD.getResult()
print "hi"
Answer.Dim()


rG = Restrict("abs(h)==0.95", 0, ThreeD)


# execute the recipe
Result = rG.getResult()

ax = Result.GetAttribute(0, "h")

# print the result
ax.show()

# Apply the condition to the vertices
print "Apply condition %s" % condition
rH = gf.Restrict(condition, 0, HH)



