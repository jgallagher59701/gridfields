import gridfield.algebra as gf
import gridfield.vis as vis
import sys
import vtk
import time
import datetime

env = {}

def gfH(context):
  H = gf.Scan(context, "H")
  ordH = gf.Accumulate("hpos", "hpos+1", "0", 0, H)
  return ordH

def zoom(bounds, H):
  zoom = gf.Restrict("(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds, 0, H)
  #appzoom = gf.Apply("mask=((%s<x) & (x<%s) & (%s<y) & (y<%s))*(40+h)" % bounds, 0, H)
  #vis.Inspect(appzoom, "mask", legend=True)
  return zoom

def gfH0(H):
  return gf.Sift(0, H)

def gfV(context):
  V = gf.Scan(context, "V")
  ordV = gf.Accumulate("vpos", "vpos+1", "0", 0, V)
  return ordV


def ToVTK(op, scalar, vector=[], show=False, save=False, capture=False, fname="DirectVis.png"):
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
    vtkwriter.SetFileName(fname)
    vtkwriter.Write()

  if capture:
    renWin.Render()
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(renWin)

    pngwriter = vtk.vtkPNGWriter()
    pngwriter.SetInput(w2if.GetOutput())
    w2if.Modified()
    pngwriter.SetFileName(fname)
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



def main():

  context = {
   "mode"      : "rundir",
   "rundir"    : "~/db7/escience_datasets/ocean_model/2006-25-14/run/",
   "runid"     : "2004-26-14",
   "runday"    : "5",
   "tstep"     : "0",
   "var"       : "u",
   "date"      : "2004 06 29"
  }
  import tempfile
  import os
  
  of, fname = tempfile.mkstemp()
  os.close(of)
  #bounds = (-4000000, 9000000, -40000000, 9000000)
  bounds = (326000,345000,287000,302000)

  project_list = ['u', 'v', 'salt', 'temp', 'z']
  W, rH, rV, addr = WetGridV5(context, bounds, project_list)
  w = W.getResult()
  ToVTK(W, "salt", capture=True, fname=fname)
  print fname

if __name__ == '__main__':
  main()


def index(req):
  return main()
  
  req.content_type = 'image/png'
  f = file("/tmp/test.png")
  return f.read()

  #return img
