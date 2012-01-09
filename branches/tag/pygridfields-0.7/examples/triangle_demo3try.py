#from pkg_resources import require
#require("gridfield>0.2")
#from gridfield import *

#
#
import os.path, sys

import numpy as np
import netCDF4
import struct
import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Wrap,Bind
import gridfield.algebra as ga

from gridfield.core import makeArrayReader
URL = 'http://testbedapps.sura.org/thredds/dodsC/ugrid/TestCases/FVCOM/fvcom_delt.ncml'
#from Scientific.IO import NetCDF


# Usually we will read gridfields from various 
# file formats.  Here, we construct one from scratch
# to illustrate the low-level interface

# A Grid is toplogy only; no data
# Construct a new grid named "points" 
# ZeroGrid is syntactic sugar for a zero dimensional grid
# A 0-d grid is a grid consisting of points only

print "working"
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

#g = gf.ZeroGrid("points",NumPhysNodes)

g1=gf.Grid("height",1)
g1.setImplicit0Cells(10);
g2 = gf.Grid("points",2)
g2.setImplicit0Cells(NumPhysNodes);

twocells = gf.CellArray()
NumPhysElems=len(nv)
for i in range(1,NumPhysElems+1):
 twocells.addCellNodes(nv[:,i-1], 3)



places=range(0,10);
onecells = gf.CellArray()
for i in range(1,11):
  onecells.addCellNodes(places[i-1:i], 1)



NumPhysElems=len(sig)


print NumPhysNodes


# construct an array of lon-values

lon = gf.Array("lon", gf.FLOAT, NumPhysNodes)
lon.copyFloatData(lon1,NumPhysNodes)



# construct an array of lat-values
#lat = gf.Array("lat", gf.FLOAT, NumPhysNodes)
#lat.copyFloatData(lat1,NumPhysNodes)
#sig=sig.reshape(NumPhysNodes*10)
h=gf.Array("h",gf.FLOAT,10*NumPhysNodes)
h.copyFloatData(sig[:,0],10)

print sig[:,0]



t1=t1[1,:,:]
t1=t1.reshape(NumPhysNodes*10)



temp=gf.Array("temp",gf.FLOAT,NumPhysNodes*10)
temp.copyFloatData(t1[:],NumPhysNodes*10)

twocells.thisown=False
onecells.thisown=False

g2.setKCells(twocells, 2);



g1.setKCells(onecells,1);
Depth = gf.GridField(g1)
# Attach the data
#Surface.Bind(0, lon)
Depthw=Wrap(Depth);

#ar=gf.makeArrayReader(lat1,NumPhysNodes)
Surf = gf.GridField(g2)
print "hi"
Surface=Wrap(Surf)
#Surf.setArrayReader(ar)
print len(lat1)
intermed=Bind("lat",lat1,0,Surface)
#gf.BindOp("lat1",gf.FLOAT,ar,0,Surfacew)
Surfaceb=intermed.getResult()
#Surfacew=Wrap(Surfaceb);
#Surface.Bind(0,lat)
Depth.Bind(0,h)
print "herro"
ax = Surfaceb.GetAttribute(0, "lat")
ax.show()
print" peace"

#----------------------------------------

# can we avoid this?
#Surfacea = gf.Accumulate("hpos", "hpos+1", "0", 0, Surfacew)
#Depthb = gf.Accumulate("vpos", "vpos+1", "0", 0, Depthw)


#rSurface = Restrict("x<50", 0, Surfacew)
  
ThreeD=ga.Cross(Surfacew,Depthw)

#ThreeDpos = ga.Apply("pos=hpos*%s + vpos" % (Depth.Size(0),), 0, ThreeD)

#for i in range(0,20):
#  temp=gf.Array("temp",gf.FLOAT,NumPhysNodes*10)
#  temp.copyFloatData(t[i,:,:],NumPhysNodes*10)
  #TimeStepGF = gf.Bind(0, temp, "pos", ThreeDpos)
#  ThreeD.Bind(0,temp)
  

Answer = ThreeD.getResult()


print "hi"



rG = Restrict("lat^2>.65", 0, ThreeD)


# execute the recipe
Result = rG.getResult()

ax = Result.GetAttribute(0, "lat")

# print the result
ax.show()




