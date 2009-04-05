from __future__ import division
import vtk

import sys
import viswidgets

import gc
import os
import os.path

class Scene:
  def __init__(self, ren, actor, mode):
    self.mode = mode
    self.renderer = ren
    self.actors = []
    self.AmbientLightOn()
    self.AddActor(actor)

  def AddActor(self, actor):
    actor.PreparePipeline()
    actor.UpdateGrid()
    actor.UpdateScalarRange()
    #print "adding actor:" 
    self.actors.append(actor)
    self.RefreshActors()

  def ToggleLabels(self):
    if hasattr(self, "viewbox"):
      del self.viewbox
      self.RefreshActors()
      print "refreshed"
    else:
      self.AddLabels(self.LeadActor().GetScalarAttribute())

  def RefreshActors(self):
    self.renderer.Clear()
    for a in self.actors:
      a.DisplayTo(self.renderer)

  def UpdateLegend(self, title, rng):
    self.LeadActor().SetScalarRange( rng[0], rng[1] )
    self.legend.SetTitle(title)

  def AmbientLightOff(self):
    for a in self.actors:
      a.AmbientLightOff()

  def AmbientLightOn(self):
    for a in self.actors:
      a.AmbientLightOn()

  def LeadActor(self):
    return self.actors[0]

  def TextProperty(self, fontsize=20):
    color = self.mode["text_color"]
    if not hasattr(self, "tprop"):
      self.tprop = vtk.vtkTextProperty()
    self.tprop.SetColor(*color)
    self.tprop.SetFontSize(fontsize)
    #tprop.ShadowOn()
    return self.tprop

  def AddLabels(self, label_attr):
    filter = self.LeadActor().GetInstigator()
    self.viewbox = viswidgets.AddViewBox(self.renderer, filter, label_attr)
    print self.viewbox

  def AddAxes(self):
    #Add an outline
    tprop = self.TextProperty()
    ren = self.renderer
    filter = self.LeadActor().GetInstigator()

    axes = vtk.vtkCubeAxesActor2D()
    axes.SetInput(filter.GetOutput())
    axes.SetCamera(ren.GetActiveCamera())
    axes.SetLabelFormat("%6.3g")
    axes.SetFlyModeToOuterEdges()
    axes.SetFontFactor(1.0)
    axes.GetProperty().SetColor(tprop.GetColor())
    axes.SetAxisTitleTextProperty(tprop)
    axes.SetAxisLabelTextProperty(tprop)
    #axes.ZAxisVisibilityOff()
    ren.AddActor(axes)
  
  def AddLegend(self, title):
    # color legend
    tprop = self.TextProperty()
    ren = self.renderer
    mapper = self.LeadActor().Mapper()
    
    #legend_widget = vtk.vtkScalarBarWidget()
    #legend = legend_widget.GetScalarBarActor()
    legend = vtk.vtkScalarBarActor()
    legend.SetLookupTable(mapper.GetLookupTable())
    legend.SetLabelTextProperty(tprop)
    legend.SetTitleTextProperty(tprop)
    legend.SetOrientationToHorizontal()
    x, y = ren.GetOrigin()
    #x0, y0, x1, y1 = ren.GetViewport()
    w, h = ren.GetSize()
    #print x,y,x0,y0,x1,y1,w,h
    #raw_input()
    legend.SetDisplayPosition(int(x+.1*w), int(y+.1*w))
    legend.SetLabelFormat("%4.1f")
    legend.SetTitle(title)
    legend.SetHeight(0.08)
    legend.SetWidth(.8)
    #legend.SetHeight(.7)
    #legend.SetWidth(0.2)
 
    #legend_widget.SetInteractor(self.iren)
 
    self.legend = legend
    ren.AddActor(legend)

  def PolygonCount(self):
    return sum([a.PolygonCount() for a in self.actors])
  
  def WriteVTKDataset(self, fname="gridfield.vtk"):
    self.LeadActor().WriteVTKDataset(fname)
    
  def Recompute(self):
    for a in self.actors:
      a.UpdateGrid()

  def PrepareAnimation(self, bindop): 
     self.LeadActor().PrepareAnimation(bindop)

  def NextFrame(self, i):
    self.LeadActor().NextFrame(i)

  def ShareCamera(self, scene):
    c = scene.renderer.GetActiveCamera()
    self.renderer.SetActiveCamera(c)

  def Clear(self):
    self.renderer.Clear()
    self.actors[:] = []
