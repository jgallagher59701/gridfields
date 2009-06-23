# Use the gridfield library to generate a netcdf file from CMOP hindcast databases
# holding the depth-averaged values in a given depth range.
# Demonstrate control over the bounding box, the depth range, the date, and the variables.
#
# Bill Howe
# Copyright 2008, Oregon Health & Science University


from pkg_resources import require
require("gridfield>0.2")

import sys

import gridfield.algebra as gf
import gridfield.core as core
#import gridfield.vis as vis
import urllib 
import time

import gridfield.optimize as tools

#from gridfield.vis import ToVTK
#import vtk

# The date to extract from
# First timestep is 15 minutes after midnight
date = '2004-06-22 00:15:00'

# Compute the CMOP representation of time from the date
# We use a web service here.

url = "http://data.stccmop.org/ws/util/modeltime"
qs = "?timestamp=%s" % (urllib.quote(date),)
resp = urllib.urlopen(url + qs)
data = resp.read()

data = resp.read()
print data
year, week, day, timestep = eval(data)
runid = '%s-%s-%s' % (year, week, 14)
print runid
# Describe the data you want to extract using a dictionary
context = {
   # mode can be "hindcast", "forecast" or "rundir" 
   # hindcast and forecast modes encode some assumptions 
   # about directory structures and file names.  
   # Rundir is more general, but requires knowledge 
   # of CMOP's filesystem organization
   "mode"      : "rundir",
   # rundir is the path to a CMOP run directory
   "rundir"    : "/home/workspace/ccalmr/hindcasts/%s/run/" % (runid,),
   # The runid for hindcasts has the form 'YYYY-DOY-DB'
   # where YYYY is the 4-digit year, DOY is 3-digit day of year,
   # and DB is the two digit database number. 
   # Email yinglong@stccmop.org for details 
   "runid"     : runid,
   # Each run spans multiple days.  runday is the day offset into the run.
   "runday"    : day,
   # tstep is the timestep you want to work with. 
   # Below, we will iterate over many timesteps
   "tstep"     : "0",
   # var is a variable name that indicates which file to open.  
   # Below, we will update this key and access multiple variables
   "var"       : "u",
   # Date is a string used as metadata.  Not currently derived 
   # from the other entries due to some idiosyncrasies in the way
   # day of year is used at CMOP
   "date"      : date
}

# Set the depth range used to extract data
# Units are meters *above* free surface
# (up is positive)
lodepth = -5
hidepth = 0

# Bounding box used to clip the data
# (xmin, xmax, ymin, ymax)
# Units are in meters in Oregon State Plane coordinates

# The whole grid
#bounds = (-4000000, 9000000, -40000000, 9000000)

# Around the mouth of the river
bounds = (326000,345000,287000,302000)

# Includes most of the estuary
#bounds = (309000,352000,260000,330000)

# A list of variables to include
vars = ['u', 'v', 'w', 'salt', 'temp', 'z']

# List of core variables:
# z    : computed z-coordinate (varies with time)
# salt : salinity in psu
# temp : temperature in C
# elev : Surface elevation in meters above MSL
# surf : Surface elevation.  Distinguished from elev for historical reasons
# u    : velocity in the x-direction 
# v    : velocity in the y-direction 
# w    : velocity in the z-direction

# Other variables exist for certain runs (heat flux and radiation, other tracers)


# ===========================
# GridField expressions
# ===========================

# Now we use the GridField API

# ==========================
# 1) Construct the 3D grid
# ==========================

# This example is more verbose than necessary for expository purposes.
# Much of this code is boilerplate and can be abstracted  
# as a function and reused.  See selfelib.py

# Load the 2D gridfield describing the horizontal domain
H = gf.Scan(context, "H")

# Enumerate each node and store the enumeration as an attribute
ordH = gf.Accumulate("hpos", "hpos+1", "0", 0, H)

# The Accumulate operator evaluates aggregation expressions
# The example above means:
#  for each cell of dimension 0 in the grid H:
#    hpos = hpos + 1

# Load the 1D gridfield describing the horizontal domain
V = gf.Scan(context, "V")

# Enumerate each node and store the enumeration as an attribute
ordV = gf.Accumulate("vpos", "vpos+1", "0", 0, V)

# Compute the address of each column
# Since the bottom of the grid is stair-stepped,
# not all water columns have the same number of levels
# The bottom index attribute "b" holds the index of the 
# lowest level for the column, so "column" is the 
# address of the previous column, plus the max
# height of the water column, less the bottom index
aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, ordH, -1)

# Restrict the horizontal grid to the region of interests
rH = gf.Restrict("(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds, 0, aH)

# The Restrict operator takes three arguments:
# -- a condition expression
# -- a dimension to restrict
# -- the previous operator

# The condition can be any expression.  arithmetic operators can be used.
# zero is interpreted as false and non-zero is interpreted as true.
# The dimension indicates which "rank" to look for the attributes.
# Cells at other dimensions will automatically be discarded to 
# maintain well-formedness properties of the grid.  For example, 
# if a node is discarded because it falls outside of the bounding box,
# any triangles to which it is incident are also discarded.  


# Now bind the 2D surface elevations to the 2D horizontal grid.

# Make a copy of the context so we can change the variable
contextsurf = context.copy()
# Change the variable name to "surf"
contextsurf["var"] = "surf"

# Pass the context to the Bind operator
# hpos indicates the attribute used as offsets into the file
surfH = gf.Bind(contextsurf, "surf", 0, rH, "hpos")

# Construct a 3D grid from the 2D horizontal grid and the 1D vertical grid
HxV = gf.Cross(surfH, ordV)

# The Cross operator computes a kind of grid-oriented "Cartesian product"
# turning triangles into prisms, and so on.

# Restrict the 3D grid to the "wet" portion -- discard
# any cells that are positioned underground
wetgrid = gf.Restrict("(vpos>b)|(b=0)", 0, HxV)

# The condition used here says we are only interested in nodes
# for which the vertical position is >= the bottom index, or
# for which the bottom index is zero. By convention, zero is 
# a distinguished sentinel value for bottom index.

# Compute the file offset for each node. 
addr = gf.Apply("offsets=column - b + vpos", 0, wetgrid)

# The Apply operator allows arbitrary arithmetic involving 
# the attributes of at any dimension. Here, we are working with
# the nodes (dimenision=0). The expression has the form
# "attribute_name1 = expression1; attribute_name2 = expression2; ...

# Now we are ready to bind data to the 3D grid

# Construct a chain of Bind operators, one for each variable
# For each Bind operator, set the "var" key in the context.
# Pass the name of the offsets attribute to allow random access
# Each cell in the grid will be associated with the data found at
# the corresponding offset we computed in the last step
# (This complication is only necessary due to the stairstep idiom
# used by SELFE grids)
context["var"] = vars[0]
B = gf.Bind(context, vars[0], 0, addr, "offsets")
for v in vars[1:]:
  context["var"] = v
  B = gf.Bind(context, v, 0, B, "offsets")

# ===============================
# 2) Aggregate over a depth range
# ===============================

# Everything prior to this point was a fairly standard 
# "set up" for working with CMOP grids (and can be reused).

# Now we compute depth bins and convert to NetCDF.

# First, compute some auxillary variables
# zfree = the z-coordinate at the free surface in MSL coordinates
# Also, set the velocity values to -99 wherever salt is -99
# -99 is used as NULL and indicates dry nodes
freeW = gf.Apply("zfree=max(z-surf,-h);u=-99*(salt=-99)+u*(salt>-99);v=-99*(salt=-99)+v*(salt>-99)", 0, B)

# Convenience function for constructing
# "greater than or equal to" and "less than or equal to"
# expressions
def or_eq(op, val):
  return "(zfree%s%s | zfree=%s)" % (op, val, val)
condition =  "( %s & %s )" % (or_eq('>',lodepth), or_eq('<',hidepth))

# Restrict the 3D grid to the depth range of interest
rW = gf.Restrict(condition, 0, freeW)

# Next, we need to compute the depth averages for each node
# We will use the Aggregate operator.  The Aggregate operator
# can map the cells of one grid onto the cells of another grid
# and aggregate the data in the process.
# In this case, we will map the nodes of the 3D grid to the
# corresponding nodes in the flat 2D grid.

# So the "assignment function" will just match up the values 
# of "hpos" -- the node identifiers
join = core.sortedmatch("hpos", "hpos")

# (For technical reasons, we want to explicitly indicate 
# that this object should not be garbage collected)
join.thisown = False

# and the "aggregation function" will compute both an 
# average and a count
mavg = core.avgfloat(",".join(vars), -99.0)
mavg.thisown = False
mcount = core.Count()
mcount.thisown = False
# Use "dotwo" to combine two aggregation functions into one
both = core.dotwo(mavg, mcount)
both.thisown = False

# null values are ignored in the calculation.  
# If all values are null, the result is null

joinA = gf.Aggregate(join, both, rH, 0, rW, 0)
joinA.getResult()


# Rename the attributes
expr = "; ".join(["%s=avg%s" % (v,v) for v in vars])
avg = gf.Apply(expr, 0, joinA)

# Rename the operator for use below
op = avg


# ====================================================
# Prepare netCDF file
# ====================================================

formatteddate = date
print "Preparing " +  formatteddate

# Get an initial result
op.getResult()

# prepare netCDF file

# The NetCDF Writer we use here needs to know which variables are time-varying
# Non-time-varying variables: bathymetry (h), which is a float (f)
fixedvars = core.Scheme("h:f")
# time-varying variables: all the others
timevars = core.Scheme(",".join(["%s:f" % (s,) for s in vars]))
# Construct the NetCDF Writer object
# This operator gets passed the raw C++ operator, accessible through op._physicalop

filename = "cmop_%s_%s-%s.nc" % ("_".join(date.split()), abs(lodepth), abs(hidepth))
writer = core.OutputNetCDFOp(filename, op._physicalop, fixedvars, timevars)
# Set the date so the metadata comes out right
writer.SetDate(formatteddate)
# compute the first result
writer.getResult()


# Now iterate over all the timesteps in the file

# Fetch the time grid T, which tells us how many timesteps there are in this file
T = gf.Scan(context, "T")

# For every timestep, re-evaluate the recipe
for i in range(T.getResult().Card(0)):
  context["tstep"] = i
  print "Timestep : %s" % (i,)

  # Convenience function for recusively propagating the context through the recipe
  tools.ReplaceBindContext(context, vars + ["z", "surf"], op)
 
  # get the result
  op.getResult()

  # Write the new values of the time-varying variables to the file
  writer.WriteTimeVars(op._physicalop, i, i*900)

