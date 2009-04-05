from pkg_resources import require
require("gridfield>0.2")
from gridfield import *

import gridfield.core as gf
import gridfield.gfvis as gfvis
from gridfield.algebra import Apply, Restrict, Wrap

# Usually we will read gridfields from various 
# file formats.  Here, we construct one from scratch
# to illustrate the low-level interface

# A Grid is toplogy only; no data
# Construct a new grid named "points" 
# ZeroGrid is syntactic sugar for a zero dimensional grid
# A 0-d grid is a grid consisting of points only
g = gf.ZeroGrid("points", 10)

# construct an array of X-values
x = gf.Array("x", gf.INT, 10)
x.copyIntData(range(10,20),10)

# construct an array of Y-values
y = gf.Array("y", gf.FLOAT, 10)
y.copyIntData(range(20,30),10)

# GridField = Topology + Data
#           = Grid + Arrays

# Construct the gridfield
G = gf.GridField(g)

# Attach the data
G.Bind(0, x)
G.Bind(0, y)

# Now we can use some operators to manipulate the data
# Operator expressions are chained together
# nothign actually happens until the expression is evaluated

# Normally we construct a gridfield implicitly
# with the "Scan" operator.
# Since we constrcuted ours from scratch,
# so we lift the raw gridfield into an operator
G = Wrap(G)

# calculate a new attribute
aG = Apply("root=sqrt(x*y)", 0, G)
H = aG.getResult()
H.show()

# restrict using the data at rank 0
rG = Restrict("x<15", 0, aG)

# execute the recipe
Result = rG.getResult()

# Get the "x" attribute at rank 0
ax = Result.GetAttribute(0, "root")

# print the result
ax.show()
