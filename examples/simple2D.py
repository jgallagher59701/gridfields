import gridfield.algebra as gf
import gridfield.core as clib
import urllib
import os.path, sys


# Fetch a 2D unstructured sample file
# Original data source is CMOP (http://stccmop.org/)
filename = "1_airt.61"
if not os.path.exists(filename):
  print "fetching sample data...."
  url = "http://www.cs.washington.edu/homes/billhowe/cmop/2006/2006-01-14/run/" + filename
  urllib.urlretrieve(url, filename)

# Call the C++ reader for CMOP files
# There's also a netCDF reader, but it's not as general as UGRID 
reader = clib.ElcircFile(filename)

# read the horizontal grid describing the surface of the water
H = reader.readHGrid()

# Inspect the gridfield

# How many cells at rank 0? (the "cardinality")
print "Vertices: ", H.Card(0)

# How many cells at rank 2?
print "Elements: ", H.Card(2)

# What are the data attributes bound at rank 0?
# Format is name:type, name:type, ...
print "Attributes of vertices: ", H.GetScheme(0).asString()

# Get an attribute
# Sadly, this object is not very useful in python
# It would be great to integrate this with numpy
x = H.GetAttribute(0, "x")
x.show()

# Now we want to restrict the gridfield to a particular region

# We don't operate on the gridfield directly;
# we build up an expression and then evaluate it lazily.
# This approach lets us optimize the expression,
# send it to a remote server for evaluation, or whatever
# (The language R, and it's predecessor S, do the same thing,
# as do many functional languages.)

# So first wrap our local gridfield object as an operator
HH = gf.Wrap(H)

# Construct a condition as a string
# We refer to the attributes found above
bounds = (326000,345000,287000,302000)
condition = "(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds

# Apply the condition to the vertices
print "Apply condition %s" % condition
rH = gf.Restrict(condition, 0, HH)

# Important: Nothing has actually happend yet. 
# GridField expressions are evaluated lazily 
# the longer we wait, the better than chance we can 
# find an optimization

# But here we'll ask for the result, which evaluates the expression
answer = rH.getResult()

# Check the number of vertices again
print "Number of vertices after restriction: ", answer.Card(0)

# Check the number of elements again
print "Number of vertices after restriction: ", answer.Card(2)

# Now let's try to draw a picture using vtk
try:
  import vtk
except:
  print "VTK not installed -- no visualization attempted"
  sys.exit()

import gridfield.vis as vis

# Show the gridfield, making whatever assumptions necessary
# color by the "h" attribute
vis.ToVTK(rH, "h", show=True)

# To save the image to a file and not interact with it, do this:
# vis.ToVTK(rH, "h", capture=True, fname="depth.png")
