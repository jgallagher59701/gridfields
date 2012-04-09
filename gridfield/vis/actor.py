from __future__ import division
import vtk

import gridfield.gfvis as vtkgridfield

import gc
import os, sys
import os.path

def toVTK(gf, scalar, vector=None):
  gf.GetGrid().normalize()

  convert = vtkgridfield.vtkGridField_New()
  convert.UseNamedPerspective()
  convert.SetScalarAttribute(scalar)
  if vector:
    convert.SetVectorAttributes(vector[0], vector[1])
  convert.SetGridField(gf)
  convert.Update()
  
  return convert

def pythonize(vtkug):
  return vtkgridfield.vtkPythonObject(vtkug)

class Actor:
  def __init__(self, op, scalar, vector=None, rank=0):
    self.sourceOperator = op
    self.scalar_attribute = scalar
    self.vector_attributes = vector 
    self.rank = rank

  def GetScalarAttribute(self):
    return self.scalar_attribute

  def Update(self):
    self.PvtkGridField = pythonize(self.vtkGridField)
    self.instigator.SetInput(self.PvtkGridField)
    self.vtkMapper.SetInput(self.instigator.GetOutput())
    self.instigator.Modified()

  def UpdateScalars(self):
    # Efficient method for updating just the scalar values of
    # a visualization.  If the grids don't match, behavior is undefined
    gf = self.sourceOperator.getResult()
    attr = self.scalar_attribute
    vtkgridfield.vtkGridField_UpdateScalars(self.vtkGridField, gf, 0, attr)
    self.Update()
 
  def SetSource(self, op, scalar, vector=None, rank=0):
    self.scalar_attribute = scalar
    self.vector_attributes = vector
    self.sourceOperator = op
    self.rank = rank

  def UpdateGrid(self):
    # Update the actor with an arbitrary gridfield.
    # Completely recomputes the visualization

    scalar = self.scalar_attribute
    vector = self.vector_attributes
    gf = self.sourceOperator.getResult()
    print gf

    if (self.vector_attributes):
      self.instigator = self.vtkbarbs
    else:
      if (gf.GetGrid().getdim() == 0):
        self.instigator = self.vtkdots
      else:
        self.instigator = self.vtkDefault
        
    
    vardata = toVTK(gf, scalar, vector)
    self.vtkGridField = vardata.GetOutput()
    self.Update()

  def GetScalarRange(self, dim=0):
    if dim == 0:
      pd = self.PvtkGridField.GetPointData()
    else:
      pd = self.PvtkGridField.GetCellData()
    
    sclrs = pd.GetScalars()
    rng = sclrs.GetRange()
    return rng
  
  def AmbientLightOff(self):
    self.vtkMainActor.GetProperty().SetAmbient(0.0)
    self.vtkMainActor.GetProperty().SetDiffuse(0.0)
    self.vtkMainActor.GetProperty().SetSpecular(0.0)

  def AmbientLightOn(self):
    self.vtkMainActor.GetProperty().SetAmbient(1.0)
    self.vtkMainActor.GetProperty().SetDiffuse(0.0)
    self.vtkMainActor.GetProperty().SetSpecular(0.0)
    pass
  
  def TextProperty(self, fontsize=20):
    if not hasattr(self, "tprop"):
      self.tprop = vtk.vtkTextProperty()
    self.tprop.SetColor(1,1,1)
    self.tprop.SetFontSize(fontsize)
    #tprop.ShadowOn()
    return self.tprop
  
  def Mapper(self, rng=None ,dim=0):
    if not hasattr(self, "vtkMapper"):
      self.vtkMapper = vtk.vtkDataSetMapper()

    if (rng):
      self.vtkMapper.SetScalarRange( rng[0], rng[1] )
      
    if dim>0: 
      self.vtkMapper.SetScalarModeToUseCellData()

    self.ColorTable()

    return self.vtkMapper
   
  # green/blue: val=0.8, 0.8, hue=0.666,0.27 
  def ColorTable(self, hue=(0.666,0), sat=(1,1), val=(1,1)):
    t = self.vtkMapper.GetLookupTable()

    #lut = vtk.vtkLookupTable()
    lut = t
    lut.SetNumberOfColors(256)
    #lut.SetNumberOfColors(20)
    lut.SetHueRange(*hue)
    #lut.SetHueRange(0.78, 0.22)
    lut.SetValueRange(*val)
    #lut.SetValueRange(.1,1)
    lut.SetSaturationRange(*sat)
    #lut.SetSaturationRange(0, 0)
    lut.Build()

    self.vtkMapper.SetLookupTable(lut)
    
  def UpdateScalarRange(self):
    rng = self.GetScalarRange(self.rank)
    self.Mapper().SetScalarRange(rng[0],rng[1])
  
  def MakeInstigator(self):
    id = vtk.vtkIdentityTransform()
    self.vtkDefault = vtk.vtkTransformFilter()
    self.vtkDefault.SetTransform(id)
    
  def PreparePipeline(self):
  
    self.vtkbarbs = vtk.vtkGlyph2D()
    arrow = vtk.vtkArrowSource()
    self.vtkbarbs.SetSource(0, arrow.GetOutput())
    self.vtkbarbs.SetScaleModeToDataScalingOff()
    self.vtkbarbs.SetColorModeToColorByVector()
    self.vtkbarbs.SetVectorModeToUseVector()
    self.vtkbarbs.SetScaleFactor(20000)

    self.vtkdots = vtk.vtkGlyph2D()
    glyph = vtk.vtkGlyphSource2D()
    glyph.SetGlyphTypeToCircle()
    self.vtkdots.SetSource(0, glyph.GetOutput())
    self.vtkdots.SetScaleModeToDataScalingOff()
    self.vtkdots.SetScaleFactor(1000)

    self.MakeInstigator()
    mapper = self.Mapper()

    self.vtkMainActor = vtk.vtkActor()
    self.vtkMainActor.SetMapper(mapper)
    
    #cam = self.renderer.GetActiveCamera()
    #cam.Elevation(20)
    #cam.Azimuth(80)
    #cam.Roll(-90)
  
  def DisplayTo(self, renderer):
    renderer.AddActor(self.vtkMainActor)

  def GetInstigator(self):
    return self.instigator

  def AmbientLightOff(self):
    self.vtkMainActor.GetProperty().SetAmbient(0.0)
    self.vtkMainActor.GetProperty().SetDiffuse(0.0)
    self.vtkMainActor.GetProperty().SetSpecular(0.0)
                                                                            
  def AmbientLightOn(self):
    self.vtkMainActor.GetProperty().SetAmbient(1.0)
    self.vtkMainActor.GetProperty().SetDiffuse(0.0)
    self.vtkMainActor.GetProperty().SetSpecular(0.0)
    pass

  def PrepareAnimation(self, bindop):
    
    if self.sourceOperator.SameGrid(bindop):
      # if the grid doesn't change, we can 
      # efficiently update the scalar vals
      self.showframe = self.UpdateGrid #self.UpdateScalars
    else:
      # Otherwise, we have to recompute the vtk object
      self.showframe = self.UpdateGrid

  def NextFrame(self, i):
    self.showframe()

  def PolygonCount(self):
    try:
      out = self.instigator.GetOutput()
      return out.GetNumberOfCells()
    except AttributeError:
      raise AttributeError("Prepare a visualization before counting polygons")
  
  def WriteVTKDataset(self, fname="gridfield.vtk"):
    self.vtkwriter = vtk.vtkUnstructuredGridWriter()
    self.vtkwriter.SetInput(self.instigator.GetOutput())
    self.vtkwriter.SetFileName(fname)
    self.vtkwriter.Write()

    
class WireFrameActor(Actor):
  def MakeInstigator(self):
    self.vtkDefault = vtk.vtkExtractEdges()

class IsoSurfaceActor(Actor):
  def MakeInstigator(self):
    self.vtkDefault = vtk.vtkContourGrid()
    #self.vtkIso.SetInput(pvardata)
    #rng = (10, 12)
    #self.vtkIso.GenerateValues(10, rng[0], rng[1]) 
  
  def UpdateScalarRange(self):
    rng = self.GetScalarRange(self.rank)
    self.vtkIso.GenerateValues(10, rng[0], rng[1]) 
    self.Mapper().SetScalarRange(rng[0], rng[1])
  
