#import matplotlib.pyplot as plt
import convert_triangles
#from matplotlib import tri
import numpy as np

try:
    import netCDF4
    url='http://geoport.whoi.edu/thredds/dodsC\
/usgs/data1/rsignell/models/adcirc/fort.64.nc'
    nc=netCDF4.Dataset(url)
    x=nc.variables['x']
    y=nc.variables['y']
    # read water depth at nodes
    h=nc.variables['depth']
    # read connectivity array
    nv=nc.variables['element']
    x=x[:]
    y=y[:]
    vals=h[:]
    nv=nv[:]-1
except:
    N=50
    x = np.random.random(N)
    y = np.random.random(N)
    h = np.random.random(N)

    
t=tri.Triangulation(x,y,triangles=nv)

plt.figure(1)
plt.tricontour(t, vals, 15, linewidths=0.5, colors='k')
plt.tricontourf(t, vals, 15, cmap=plt.cm.jet)
plt.title('tri.Triangulation: tricontourf')
plt.colorbar()
plt.show()

# convert Tri triangle object to Delaunay triangle object
# delaunay can't handle holes, so fill those in...
t_convex = convert_triangles.tri_nonconvex_to_convex(t)

# now we can create a delaunay object:
d=convert_triangles.tri_to_delaunay(t_convex)


lin_interp = d.linear_interpolator(vals)

# but we'd like to omit the extra triangles from the triangulation -
# reach inside the linear interpolator and mask
# out the new triangles we had to add to make it convex 
lin_interp.planes[len(t.triangles):,0] = np.nan

# Funny syntax for calling the linear interpolator.  The assumption is
# that the result is stored in [row,col] ordering, so the first slice
# says go from min(y) to max(y) with 500 samples, and from min(x) to max(x)
# with 400 samples.

# extent = [x.min(),x.max(),y.min(),y.max()]
extent = [-76.3,-75.3,35.5,36.5]
lin_field = lin_interp[extent[2]:extent[3]:500j, extent[0]:extent[1]:400j]

plt.figure(2)
plt.imshow(lin_field,origin='lower',extent=extent, interpolation='nearest')
plt.title('delaunay.Triangulation: linear interp')
plt.colorbar()
plt.show()


