import gridfield.algebra as gf
import gridfield.vis as vis
import sys
import vtk
import time
import datetime
from gridfield.vis import ToVTK

env = {}

def ToVTK2(op, scalar, vector=[], show=False, save=False, capture=False):
  gH = op.getResult()
  gH.GetGrid().normalize()

  convert = vis.vtkgridfield.vtkGridField_New()
  convert.UseNamedPerspective()
  convert.SetScalarAttribute(scalar)
  if vector:
    convert.SetVectorAttributes(vector[0], vector[1], vector[2])

  convert.SetGridField(gH)
  convert.Update()

  vtkGridField = convert.GetOutput()
  PvtkGridField = vis.vtkgridfield.vtkPythonObject(vtkGridField)
  pd = PvtkGridField.GetPointData()

  sclrs = pd.GetScalars()
  rng = sclrs.GetRange()

  id = vtk.vtkIdentityTransform()
  dummy = vtk.vtkTransformFilter()
  dummy.SetTransform(id)
  dummy.SetInput(PvtkGridField)

  mapper = vtk.vtkDataSetMapper()
  mapper.SetInput(PvtkGridField)
  mapper.SetScalarRange( rng[0], rng[1] )

  vtkMainActor = vtk.vtkActor()
  vtkMainActor.SetMapper(mapper)
  vtkMainActor.GetProperty().SetAmbient(1.0)
  vtkMainActor.GetProperty().SetDiffuse(0.0)
  vtkMainActor.GetProperty().SetSpecular(0.0)

  renderer = vtk.vtkRenderer()
  renderer.SetBackground(0.2375,0.3,0.3625)
  renderer.AddActor(vtkMainActor)

  renWin = vtk.vtkRenderWindow()
  renWin.SetSize(500,500)
  renWin.AddRenderer(renderer)

  if save:
    vtkwriter = vtk.vtkUnstructuredGridWriter()
    vtkwriter.SetInput(PvtkGridField)
    vtkwriter.SetFileName("DirectVis.vtk")
    vtkwriter.Write()


  if capture:
    renWin.Render()
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(renWin)

    pngwriter = vtk.vtkPNGWriter()
    pngwriter.SetInput(w2if.GetOutput())
    w2if.Modified()
    pngwriter.SetFileName("DirectVis.png")
    pngwriter.Write()

  if show:
    iren = vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(renWin)
    iren.Start()

  return PvtkGridField


def BindSome(context, attrs, X, addrattr="addr"):
  if not attrs:
    raise ValueError("No attributes provided.")

  if isinstance(attrs, str):
    attrs = [attrs]
  
  context["var"] = attrs[0]
  B = gf.Bind(context, attrs[0], 0, X, addrattr)
  for a in attrs[1:]:
    context["var"] = a
    B = gf.Bind(context, a, 0, B, addrattr)
  return B 

def WetGridV5(context, bounds, var, zcond="vpos>-1"):
  H = gfH(context)
  V = gfV(context)
  V0 = gf.Sift(0,V)
  rV = gf.Restrict(zcond, 0, V)
  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, H, -1)
  rH = zoom(bounds, aH)
  rH0 = gf.Sift(0,rH)
  #rH = gf.Restrict("hpos=10196", 0, rrH)

  contextsurf = context.copy()
  contextsurf["var"] = "surf"
  surfH = gf.Bind(contextsurf, "surf", 0, rH, "hpos")

  HxV = gf.Cross(rH, rV)
  wetgrid = gf.Restrict("(vpos>b)|(b=0)", 0, HxV)
  addr = gf.Apply("addr=column - b + vpos", 0, wetgrid)
  B = gf.Bind(context, "salt", 0, addr, "addr")
  #ToVTK(B, "salt", show=True)
  B = BindSome(context, var+ ["z"], addr)
  #B.getResult().GetAttribute(0, "salt").show()
  #B.getResult().GetAttribute(0, "temp").show()
  #B.getResult().GetAttribute(0, "surf").show()
  #B.getResult().GetAttribute(0, "b").show()
  #B.getResult().GetAttribute(0, "z").show()
  #sys.exit(0)
  #vis.Inspect(B, "salt", labels="salt")
  return B, rH, rV, addr

def AnimateDay(context, attrs, root, dpr):
  nsteps = 96
  for i in range(nsteps):
    context["tstep"] = i
    t = time.time()
    gf.ReplaceBindContext(context, attrs, root)
    dpr.Recompute()
    #dpr.CaptureImage("aug11/aug11_%02i.png" % (i,))

env = {}

def Surface(context, bounds, vars, scale=1000):
  H = gfH(context)
  V = gfV(context)

  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, H, -1)

  Hv = gf.Apply("vpos=0", 0, aH)
  
  addr = gf.Apply("addr=column - b + vpos", 0, Hv)
  B = BindSome(context, vars + ['z'], addr)

  B.getResult().GetAttribute(0, "z").show()
  sys.exit()
  return B

  #addr = gf.Apply("addr=hpos*%s+vpos" % (V.getResult().Card(0),), 0, Hv)


def ZInterpV5(context, bounds, vars, depth, scale=1000):

  H = gfH(context)
  V = gfV(context)
  #bH = gf.Apply("b=(b>-1)*b-1 + (b=-1)*b", 0, H)
  bH = H
  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, bH, -1)
  rH = zoom(bounds, aH)
  
  nodes = gf.Nodes()
  mn = gf.minint("vpos")
  mx = gf.maxint("vpos")
  minmax = gf.dotwo(mn, mx)
  V1 = gf.Aggregate(nodes, minmax, V, 1, V, 0)
                 
  # keep pointers so they aren't garbage collected
  env["mn"] = mn
  env["mx"] = mx
  env["minmax"] = minmax

  contextsurf = context.copy()
  contextsurf["var"] = "surf"
  surfH = gf.Bind(contextsurf, "surf", 0, rH, "hpos")

  rH0 = gf.Sift(0,surfH)
  
  # constructs stacks of vertical 1-cells
  HxV = gf.Cross(rH0, V1)
  wetgrid = gf.Restrict("(minvpos > b)", 1, HxV)
  
  addr = gf.Apply("loaddr=column - b + minvpos; hiaddr=column - b + maxvpos", 1, wetgrid)
  context["var"] = "z"
  loB = gf.Bind(context, "loz", 1, addr, "loaddr")
  B = gf.Bind(context, "hiz", 1, loB, "hiaddr")

  # convert to negative meters below free surface (up is positive)
  r = gf.Apply("loz=loz-surf; hiz=hiz-surf", 1, B)

  # Find those vertical 1-cells that contain the desired depth
  condition = '''(loz < %s) & ((%s < hiz) | (%s = hiz))''' % (depth,depth,depth)
  R = gf.Restrict(condition, 1, r)

  # sift discards all d-cells not incident to a k-cell, 
  # where k is an argument and d < k
  sR = gf.Sift(1, R)

  # Bind variables to endpoints of the vertical 1-cells
  baddr = gf.Apply("addr = column - b + vpos", 0, sR)
  bB = BindSome(context, vars + ['z'], baddr)

  # construct a variable representing the constant depth
  Adepth = gf.Apply("z=%s" % (depth,), 0, surfH)
  
  # Perform a pointer-regrid from the vertical 1-cells to the nodes of H
  #env["ptrjoin"] = gf.match("hpos")
  env["ptrjoin"] = gf.sortedmatch("hpos", "hpos")
  varsz = vars + ["z"]
  varlist = ",".join(varsz)
  # interpolate1D(fromattr, toattr, varlist) 
  # interpolates each v using fromattr and toattr.  Expects two values to be matched
  env["one"] = gf.interpolate1Dfloat("z","z",varlist)
  join = gf.Aggregate(env["ptrjoin"], env["one"], Adepth, 0, bB, 0)

  return join

def Stratification(context, bounds, vars):
  H = gfH(context)
  V = gfV(context)
  #bH = gf.Apply("b=(b>-1)*b-1 + (b=-1)*b", 0, H)
  bH = H
  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, bH, -1)
  rH = zoom(bounds, aH)

  if not "z" in vars: vars += ["z"]

  rvH = gf.Apply("top=1; bottom=%s" % (V.getResult().Card(0) - 1,), 0, rH)

  RrvH = rvH #gf.Restrict("top>b", 0, rvH)

  addr = gf.Apply("topaddr=column - b + top; botaddr=column - b + bottom", 0, RrvH)
  B = gf.Bind(context, "salt", 0, addr, "topaddr")
  ToVTK(B, "salt", show=True)

  Btop = BindSome(context, vars, addr, "topaddr")

  expr = ";".join(["top%s = %s" % (v, v) for v in vars])
  rename = gf.Apply(expr, 0, Btop)

  Bbot = BindSome(context, vars, rename, "botaddr")

def DepthAggregate(context, bounds, vars, mindepth):
  year, day = time.strftime("%Y %j", time.localtime()).split()

  W, rH, rV, addr = WetGridV5(context, bounds, vars, "vpos>-1")

  contextsurf = context.copy()
  contextsurf["var"] = "surf"
  surfW = gf.Bind(contextsurf, "elev", 0, W, "hpos")

  #rW = gf.Restrict("(z>elev-5) & (salt>-90)", 0, surfW)
  expr="%s=(z>elev-5)*%s+(z<elev-5)*-99"
  rW = gf.Apply("; ".join([expr % (v,v) for v in vars + ["z"]]), 0, surfW)

  join = gf.sortedmatch("hpos", "hpos")

  clump = gf.mkvector("ptrs")

  deref = gf.byPointerSet("ptrs", "hpos")
  msum = gf.avgfloat(",".join(vars + ["z", "elev"]), -99.0)
  mcount = gf.Count()
  both = gf.dotwo(msum, mcount)

  joinA = gf.Aggregate(join, both, rH, 0, rW, 0)
  
  #avg = joinA
  
  expr = "; ".join(["%s=avg%s" % (v,v) for v in vars + ["z", "elev"]])
  avg = gf.Apply(expr, 0, joinA)
  avg.getResult()
  avg.getResult().GetAttribute(0, "x").show()
  check = gf.Restrict("salt=-99", 0,avg)
  
  return check

def dateFromContext(context):
  date = " ".join(context["runid"].split("-")[:-1] + [context["runday"]])
  realdate = time.strptime(context["date"], "%Y %m %d") #no good way to do this! #time.strptime(date,"%Y %W %w")
  formatteddate = time.strftime("%Y-%m-%d 00:00:00", realdate)
  print date, formatteddate
  return date, formatteddate

def AsNetCDF(op, context, vars, filename):
  date, formatteddate = dateFromContext(context)
  op.getResult()
  # prepare netCDF file
  fixedvars = gf.Scheme("h:f")
  timevars = gf.Scheme(",".join(["%s:f" % (s,) for s in vars]))
  writer = gf.OutputNetCDFOp(filename, op._physicalop, fixedvars, timevars)
  writer.SetDate(formatteddate)
  writer.getResult()

  T = gf.Scan(context, "T")
  
  for i in range(T.getResult().Card(0)):
    context["tstep"] = i
    print "Timestep : %s" % (i,)
    gf.ReplaceBindContext(context, vars + ["z", "surf"], op) 
    writer.WriteTimeVars(op._physicalop, i, i*900)
  
def StuebeNetCDF(xyop, zop, crossop, context, project_list, filename):
  date, formatteddate = dateFromContext(context)
  xyop.getResult()
  zop.getResult()
  crossop.getResult()

  # prepare netCDF file
  fixedvars = gf.Scheme("h:f")
  timevars = gf.Scheme(",".join(["%s:f" % (s,) for s in project_list]))
  xyscheme = gf.Scheme("x:f,y:f")
  zscheme = gf.Scheme("depth_percent:f")
  writer = gf.StuebeNetCDFOp(filename, xyscheme, zscheme, timevars)
  zdop = gf.Apply("depth_percent=z", 0, zop)
  XY, Z, W = xyop.getResult(), zdop.getResult(), crossop.getResult()
  writer.WriteSELFENetCDF(XY, Z, W)
  writer.SetDate(formatteddate)

  T = gf.Scan(context, "T")

  for i in range(0,10): #T.getResult().Card(0)):
    context["tstep"] = i
    print "Timestep : %s" % (i,)
    gf.ReplaceBindContext(context, project_list + ["z", "surf"], crossop)
    W = crossop.getResult()
    writer.Write3DTimestep(W, i, i*900)

def SelectedRegion():
  context = {
   "mode"      : "rundir",
   "rundir"    : "/home/workspace/ccalmr/hindcasts/2004-25-14/run/",
   "runid"     : "2004-26-14",
   "runday"    : "5",
   "tstep"     : "0",
   "var"       : "u",
   "date"      : "2004 06 29"
  }

  #bounds = (-4000000, 9000000, -40000000, 9000000)
  bounds = (326000,345000,287000,302000)

  project_list = ['u', 'v', 'salt', 'temp', 'z']
  W, rH, rV, addr = WetGridV5(context, bounds, project_list)
  #ToVTK(W, "salt", capture=True)

  fname = "%s.nc" % ("".join(context['date'].split()),)
  print "filename: ", fname
  StuebeNetCDF(rH, rV, W, context, project_list, fname)

def TwoD():
  #DepthAggregate()
  context = {
   "mode"      : "rundir",
   "rundir"    : "/home/workspace/ccalmr/hindcasts/2004-25-14/run/",
   "runid"     : "2004-26-14",
   "runday"    : "5",
   "tstep"     : "0",
   "var"       : "u",
   "date"      : "2004 06 29"
  }

  #bounds = (-4000000, 9000000, -40000000, 9000000)
  bounds = (326000,345000,287000,302000)

  depth = -5 # negative meters below free surface

  vars = ['u', 'v', 'salt', 'temp', 'z']
  #W, rH, rV, addr = WetGridV5(context, bounds, vars)
  #ToVTK(W, "salt", show=True)
  
  #op = Surface(context, bounds, vars) 
  op = ZInterpV5(context, bounds, vars, depth, "vpos>-1")
  #op = DepthAggregate(context, bounds, vars, depth)
  #R = gf.Restrict("salt>-99", 0, op)
  #Stratification(context, bounds, vars)
  

  ToVTK(W, "salt", capture=True)
  fname = "%s_%sm.nc" % ("".join(context['date'].split()),-depth)
  print "filename: ", fname
  AsNetCDF(op, context, vars, fname)

if __name__ == "__main__":
  #TwoD()
  SelectedRegion()
