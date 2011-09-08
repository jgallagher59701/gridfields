from __future__ import division
import vtk
import gridfield.gfvis as vtkgridfield
import actor
import scene
import sys
from gridfield.core import INT, FLOAT

import gc
import os
import os.path
import tempfile

PAPER = {
         "bg_color":(1,1,1),
         "text_color":(0,0,0)
        }
                                                                            
SCREEN = {
         "bg_color":(0.2375,0.3,0.3625),
         "text_color":(1,1,1)
        }


def ToVTK(op, scalar, vector=[], show=False, save=False, capture=False, fname="vis.png"):
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

    #f, imgname = tempfile.mkstemp(".png", "gf", dir=d)
    #os.close(f)
    #root, name = os.path.split(imgname)
    #base, ext = os.path.splitext(name)
    #WriteWorldFile(renderer, os.path.join(root, base + ".pfw"), width, height)

    pngwriter = vtk.vtkPNGWriter()
    pngwriter.SetInput(w2if.GetOutput())
    w2if.Modified()
    pngwriter.SetFileName(fname)
    pngwriter.Write()

  if show:
    iren = vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(renWin)
    iren.Start()


class DataProductRenderer:

  def __init__(self, renWin, mode=SCREEN, screenlock=None):
    import mutex
    self.mode = mode
    self.framestr = ""
    self.shot_id = 0
    self.renWin = renWin
    self.objwriter = vtk.vtkOBJExporter()
    self.objwriter.SetInput(self.renWin)
    
    self.vrmlwriter = vtk.vtkVRMLExporter()
    self.vrmlwriter.SetInput(self.renWin)
  
    #self.pswriter = vtk.vtkGL2PSExporter()
    #self.pswriter.SetInput(self.renWin)
    #self.pswriter.SetFileFormatToEPS()
    
    self.w2if = vtk.vtkWindowToImageFilter()
    self.w2if.SetInput(self.renWin)
    
    self.pngwriter = vtk.vtkPNGWriter()
    self.pngwriter.SetInput(self.w2if.GetOutput())
  
    self.jpgwriter = vtk.vtkJPEGWriter()
    self.jpgwriter.SetInput(self.w2if.GetOutput())
    self.jpgwriter.SetFilePrefix("scene")
    self.jpgwriter.SetQuality(90)
    
    self.scenes = {}
    
    self.mainCamera = None
    self.recordAnimation = False
    self.outerop = None

    def noanim(i):
      raise ValueError("No animation prepared.  Call 'AnimateScalar' first.")
    self.showframe = noanim
    
  def SetCamera(self, camera):
    self.mainCamera = camera

  def AnimationRate(self):
    return int(self.PolygonCount() / self._rateFactor)

  def MakeAnimation(self, path="./anim", k=0):

    if (self.outerop == None):
      raise ValueError("No Animations prepared.  Use AnimateScalar")

    frames = self.outerop.getResult().Size(k)
   
    if not "/" in path:
      path = "./" + path

    temp = self.recordAnimation

    (dirname, self.recordAnimation) = os.path.split(path)

    for i in range(frames):
      self.showframe(i)

    files = os.listdir(dirname)

    epss = [f for f in files if '.jpg' == f[-4:]]
    gifs = [os.path.splitext(f)[0] + '.gif' for f in epss]

    #cmd = "convert -resize 500x %s %s"

    #cmds = [cmd % p for p in zip(epss, gifs)]

    #for c in cmds: os.popen(c)

    cmdAnimate = "convert -loop 0 -delay 35 %s*.jpg %s.gif" % (path, self.recordAnimation)
    
    os.popen(cmdAnimate)

    rmcmd = "rm %s_*.*" % (self.recordAnimation,)
    print rmcmd
    os.popen(rmcmd)
    
    self.recordAnimation = temp


  def PrepareAnimation(self, bindop, outerop, iter_attr, 
                    rate=30, scenes=[], saveas="", k=0):
     
    self.outerop = outerop
    self._rateFactor = rate
    
    outergf = outerop.getResult()
    a = outergf.GetAttribute(k, iter_attr)
    #x  = outergf.getIntAttributeVal(iter_attr, 0)
    if a.type == FLOAT:
      getval = outergf.GetFloatAttributeValue
    elif a.type == INT:
      getval = outergf.GetIntAttributeValue
    else: 
      raise TypeError("Unknown type on attribute %s" % (a.name(),))

    if len(scenes) == 0:
      scenes = self.scenes.keys()

    for row, col in scenes:
      print row, col
      assert(isinstance(row,int) and isinstance(col, int))
      try:
        p = self.GetScene(row, col)
      except KeyError, e:
        raise KeyError("There is no display pane with coordinates (%i, %i); window size is %s" % (row, column, self.windowshape))
      p.PrepareAnimation(bindop)
    
    self.recordAnimation = saveas

    def showframe(i):
      val = getval(k, iter_attr, i % outergf.Size(k))
      for p in self.scenes.itervalues():
        #bindop.getResult().GetAttribute(0, "z").show()
        bindop.ChangeContext(iter_attr, val)
        bindop.getResult().GetAttribute(0, "salt").show()
        #bindop.getResult().GetAttribute(0, "z").show()
        p.NextFrame(i)

      self.Render()
      if self.recordAnimation: 
        self.CaptureImage( "%s_%05i.jpg" % (self.recordAnimation, i) )

    self.showframe = showframe
    
  def NextFrame(self, i):
    try:
      self.showframe(i)
    except AttributeError:
      raise AttributeError("No Animation Prepared!")
        
  def PolygonCount(self):
    return sum([p.PolygonCount() for p in self.scenes.itervalues()])
      
  def WriteVTKDataset(self, fname="gridfield.vtk"):
    for ((r,c),p) in self.scenes.iteritems():
      p.WriteVTKDataset("%s_%s_%s" % (r,c,fname))

  def CaptureImage(self, filename):
    (prefix, ext) = os.path.splitext(filename)
    if ext == ".eps":
      self.pswriter.Modified()
      self.pswriter.SetFilePrefix(prefix)
      self.pswriter.Write()
    elif ext == ".jpg":
      self.w2if.Modified()
      self.jpgwriter.SetFileName(filename)
      self.jpgwriter.SetQuality(100)
      self.jpgwriter.Write()

    elif ext == ".png":
      self.w2if.Modified()
      self.pngwriter.SetFileName(filename)
      self.pngwriter.Write()

    elif ext == ".wrl":
      self.vrmlwriter.Modified()
      self.vrmlwriter.SetFileName(filename)
      self.vrmlwriter.Write()
      
    elif ext == ".obj":
      self.objwriter.Modified()
      self.objwriter.SetFilePrefix(prefix)
      self.objwriter.Write()
    else:
      raise ValueError("Can't produce requested image type '%s'" % (ext,))

  def addActor(self, actor):
    for i in self.ren:
      ren[i].AddActor(actor)

  def ClearPanes(self):
    self.scenes.clear()

  def ClearRenderers(self):
    rens = self.renWin.GetRenderers() 
    rens.InitTraversal()
    ren = rens.GetNextItem()
    while ren:
      self.renWin.RemoveRenderer(ren)
      ren = rens.GetNextItem()

  
  def Reset(self):
    self.ClearRenderers()
    self.ClearPanes()
  
  def AddRenderer(self, ren, row=0, column=0):
    coords = self.GetCoords(row, column)
    self.renWin.RemoveRenderer(ren)
    ren.SetBackground(*self.mode["bg_color"])
    ren.SetViewport(*coords)
    self.renWin.AddRenderer(ren)
    return ren
  
  def GetCoords(self, row, column):
    rows, columns = self.windowshape
    if row >= rows or column >= columns:
      raise ValueError("Window shape is %s; can't add pane at (%i, %i)" \
        % (self.windowshape, row, column))

    coords = (column/columns, row/rows, (column+1)/columns, (row+1)/rows)
    return coords
  
  def PrepareSceneGrid(self, actors, rows=1, columns=1):
    self.ClearRenderers()
    self.windowshape = (rows, columns)
    for i in range(rows):
      for j in range(columns):
        ren = vtk.vtkOpenGLRenderer()
        self.AddRenderer(ren, i, j)
        a = actors[i*columns+j]
        s = scene.Scene(ren, a, self.mode)
        self.SetScene(s, i, j)
  
  def SetScene(self, s, row=0, column=0):
    self.scenes[(row,column)] = s

  def AllScenes(self):
    for s in self.scenes.itervalues():
      yield s

  def GetScene(self, row=0, column=0):
    return self.scenes[(row,column)]

  def AddActor(self, actor, row=0, column=0):
    s = self.GetScene(row,column)
    s.AddActor(actor)

  def ClearScene(self, actor, row=0, column=0):
    s = self.GetScene(row, column)
    s.Clear()
    s.AddActor(actor)

  def OneCamera(self, row=0, column=0):
    main = self.GetScene(row, column)
    for s in self.scenes.itervalues():
      s.ShareCamera(main)
 
  def Recompute(self):
    for s in self.AllScenes():
      s.Recompute()
      self.Render()
  
  def Render(self):
    self.renWin.Render()
    #for p in self.scenes.itervalues():
    #  p.Render()
  
  def Draw(self, op, scalar, vector=None, labels=False, axes=False, legend=True):
    try:
      ops = [o for o in op]
    except:
      ops = [op]

    ast = [actor.Actor(o, scalar, vector) for o in ops]

    if len(ast) == 0: raise "No gridfields to draw."
      
    self.PrepareSceneGrid(ast[:1], 1,1)
    p = self.GetScene()
    for a in ast[1:]: p.AddActor(a)

    #p.AmbientLightOn()
    if axes: 
      p.AddAxes()
    if legend:
      p.AddLegend(scalar)
    if labels:
      p.AddLabels(labels)

    self.Render()
    #r = vtk.vtkOpenGLRenderer()
    
    #cone = vtk.vtkConeSource()
    #cone.SetHeight( 3.0 )
    #cone.SetRadius( 1.0 )
    #cone.SetResolution( 10 )

    #coneMapper = vtk.vtkPolyDataMapper()
    #coneMapper.SetInput(cone.GetOutput())

    #coneActor = vtk.vtkActor()
    #coneActor.SetMapper(coneMapper)

    #r.AddActor(coneActor)
    #self.renWin.AddRenderer(r)
    #for i in range(100):
      #p.vtkMapper.SetScalarRange( i, i+2 )
      #cone.SetHeight( float(i) )
      #coneActor.GetProperty().SetColor(float(i), float(i)*2, float(i)*3)
      #raw_input("hello?")
      #self.renWin.Render()
      
class Trajectory2D:
  def __init__(self):
    self.coords = []

  def AddPoint(self, x, y):
    self.coords.append((x,y))

  def GetGridField(self, name="P"):
    P = gf.LinearGrid(name, len(coords))
    xs, ys = zip(*self.coords)
    axs = Array("x", FLOAT)
    axs.thisown = False
    axs.copyFloatData(xs, len(xs))
    ays = Array("y", FLOAT)
    ays.thisown = False
    ays.copyFloatData(ys, len(ys))
    gfP = gf.GridField(P)
    gfP.Bind(axs)
    gfP.Bind(ays)
    return gfP

  def ClearPoints(self, obj):
    self.coords[:] = []
 
   
class DataProductInteractor(DataProductRenderer):
  def __init__(self, renWin, iren, mode=SCREEN):
    self.iren = iren
    self.frame = 0
    self.trajectory = Trajectory2D()
    DataProductRenderer.__init__(self, renWin, mode)

  def AddObserver(self, event, callback):
    self.iren.AddObserver(event, callback)

  def OnLeftClick(self, obj, event):
    if obj.GetControlKey():
      wp = self.GetSelectedWorldPoint(obj)
      print wp
      self.trajectory.AddPoint(*wp[0:2])
    else:
      self.trajectory.ClearPoints(obj)
      
  def GetSelectedWorldPoint(self, obj):
    x, y = obj.GetEventPosition()
    # Convert display point to world point
    ren = self.GetScene(0,0).renderer
    ren.SetDisplayPoint( x, y, 0 )
    ren.DisplayToWorld()
    wp = ren.GetWorldPoint()
    return wp
 
  def OnKeyPress(self, obj, event):
    k = obj.GetKeyCode()
    print k
    if k == 'w':
      for p in self.AllScenes():
        p.AmbientLightOn()
    elif k == 's':
      for p in self.AllScenes():
        p.AmbientLightOff()
    elif k == 'k':  #keep
      self.CaptureImage("%s_snapshot.png" % (self.shot_id,))
      self.shot_id += 1
    #elif k in [str(i) for i in range(10)]:
    #  print "NUM"
    #  self.framestr += k
    #elif k == 'g': 
    #  print self.framestr
    #  self.showframe(int(self.framestr))
    elif k == '+':
      self.showframe(self.frame)
      self.frame += 1
    elif k == '-':
      self.showframe(self.frame - 1)
      self.frame -= 1
    elif k == 'S':
      self.animating = False
    elif k == '*':
      self.GetScene().AmbientLightOn()
    elif k == '-':
      self.GetScene().AmbientLightOff()
    elif k == 'l':
      p = self.GetScene()
      p.ToggleLabels()
      self.Render()
    
  def Display(self):
    self.AddObserver("KeyPressEvent", self.OnKeyPress)
    self.AddObserver("LeftButtonPressEvent", self.OnLeftClick)
    self.iren.Initialize()
    #self.renWin.Render()
    self.iren.Start()

def Visualize(op, attr="", vector=None, 
                         labels=False, axes=False, legend=True):
  renWin = vtk.vtkRenderWindow()
  renWin.SetSize(500,500)
  #renWin.OffScreenRenderingOn()
  iren = vtk.vtkRenderWindowInteractor()
  iren.SetRenderWindow(renWin)

  dpr = DataProductInteractor(renWin, iren)
  dpr.Draw(op, attr, vector, labels, axes, legend)

  return dpr

def Window(mode=SCREEN):
  renWin = vtk.vtkRenderWindow()
  renWin.SetSize(1000,500)
  #renWin.OffScreenRenderingOn()
  iren = vtk.vtkRenderWindowInteractor()
  iren.SetRenderWindow(renWin)

  dpr = DataProductInteractor(renWin, iren, mode)
  return dpr

def Inspect(op, attr="", vector=None, axes=False, legend=False, labels=False):
  print "inspecting..."
  dpr = Window()
  dpr.Draw(op, attr, vector, labels, axes, legend)
  dpr.Display()


def Show(op, scalar):
  G = op.getResult()

  G.GetGrid().normalize()
  convert = vtkgridfield.vtkGridField_New()
  convert.UseNamedPerspective()
  convert.SetScalarAttribute(scalar)
  convert.SetGridField(G)
  convert.Update()

  vtkGridField = convert.GetOutput()
  PvtkGridField = vtkgridfield.vtkPythonObject(vtkGridField)

  id = vtk.vtkIdentityTransform()
  instigator = vtk.vtkTransformFilter()
  instigator.SetTransform(id)
  instigator.SetInput(PvtkGridField)

  mapper = vtk.vtkDataSetMapper()
  mapper.SetInput(instigator.GetOutput())

  vtkMainActor = vtk.vtkActor()
  vtkMainActor.SetMapper(mapper)

  renderer = vtk.vtkRenderer()
  renderer.AddActor(vtkMainActor)

  renWin = vtk.vtkRenderWindow()
  renWin.SetSize(500,500)
  renWin.AddRenderer(renderer)
  #renWin.Render()

  #renWin.OffScreenRenderingOn()
  iren = vtk.vtkRenderWindowInteractor()
  iren.SetRenderWindow(renWin)
  iren.Start()
