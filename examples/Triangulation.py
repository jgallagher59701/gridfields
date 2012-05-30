import matplotlib.delaunay as triang
import pylab
import numpy
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from pylab import *
from rtree import index
from gridfield import *
import gridfield.core as gf
from gridfield.algebra import Apply, Restrict, Wrap, Bind, Remesh
import gridfield.core as gf
import gridfield.core as gf


#this is a simple example involving supermeshing between two triangulations
#of the unit square. The elements are assumed bilinear. The values given
#are interpreted as coefficients of the basis function that is one at
#the current point and zero elsewhere. 


idx = index.Index()

ntot1=40;
ntot2=80;
# create triangulation of unit square instead of importing it

x=  loadtxt('xgrid1.txt')
y=  loadtxt('ygrid1.txt')
u=  loadtxt('ugrid1.txt')
cens,edg,tri,neig = triang.delaunay(x,y)

xx=reshape(x,(ntot2,ntot1))
yy=reshape(y,(ntot2,ntot1))
uu=reshape(u,(ntot2,ntot1))

g = gf.Grid("points",2)

g.setImplicit0Cells(len(x));

twocells = gf.CellArray()
NumPhysElems=len(tri)
print tri
for i in range(1,NumPhysElems+1):
 twocells.addCellNodes(tri[i-1][:], 3)

# Construct the gridfield


# Import Second triangulation of unit square
x1p=  loadtxt('xgrid2.txt')
y1p=  loadtxt('ygrid2.txt')

cens1,edg1,tri1,neig1 = triang.delaunay(x1p,y1p)


g1 = gf.Grid("points1",2)

g1.setImplicit0Cells(len(x1p));

#construct second gridfield object.



twocells1 = gf.CellArray()
NumPhysElems=len(tri1)
for i in range(1,NumPhysElems+1):
 twocells1.addCellNodes(tri1[i-1][:], 3)


g.setKCells(twocells, 2);

Gp0 = gf.GridField(g)
#attach things to the second Gridfield

intermedp0=Bind("x",x,0,Gp0)
Gp1=intermedp0.getResult()
# Attach the data
Gp2=Bind("y",y,0,Gp1)
Gp3=Gp2.getResult()
Gp4=Bind("u",u,0,Gp3)
G=Gp4.getResult()

g1.setKCells(twocells1, 2);

G30 = gf.GridField(g1)
#attach things to the Gridfield

intermed=Bind("x",x1p,0,G30)
G31=intermed.getResult()
# Attach the data
G21=Bind("y",y1p,0,G31)
G32=G21.getResult()
G3=G32.getResult()


Grid=G3.GetGrid()

# apply remeshing between two gridfields!
# get a gridfield back that is defined on the new grid.

C=gf.RemeshOp("x","y",G3,G)
G3=C.Remesh("x","y",G,G3)

ua=G3.GetAttribute(0, "u")
xa=G3.GetAttribute(0, "x")
ya=G3.GetAttribute(0, "y")
u2=np.array(ua.makeArrayf(),ndmin=2);
x2=np.array(xa.makeArrayf(),ndmin=2);
y2=np.array(ya.makeArrayf(),ndmin=2);

print x2.T.shape,60*40
xx2=reshape(x2.T,(40,60))
yy2=reshape(y2.T,(40,60))
uu2=reshape(u2.T,(40,60))

fig = plt.figure()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(xx, yy, uu, rstride=1, cstride=1, cmap=cm.jet,linewidth=0, antialiased=False)


fig = plt.figure()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(xx2, yy2, uu2, rstride=1, cstride=1, cmap=cm.jet,linewidth=0, antialiased=False)
plt.show()

###added to avert confusion over garbage collection in c++ vs in SWIG
twocells.thisown=False
twocells1.thisown=False



print "madeithere"


#plt.show()
 

