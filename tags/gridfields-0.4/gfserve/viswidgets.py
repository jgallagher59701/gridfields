#!/usr/bin/env python

# This example demonstrates the use of vtkLabeledDataMapper.  This
# class is used for displaying numerical data from an underlying data
# set.  In the case of this example, the underlying data are the point
# and cell ids.

import vtk

def AddViewBox(ren,dataset, attr):
  return ViewBox(ren,dataset,attr)

class ViewBox:
  def __init__(self,ren, dataset, attr):
    self.ren = ren
  
    # Create a selection window.  
    xmin = 200
    xLength = 100
    xmax = xmin + xLength
    ymin = 200
    yLength = 100
    ymax = ymin + yLength
   
    self.xmin = xmin
    self.xLength = xLength
    self.xmax = xmax
    self.ymin = ymin
    self.yLength = yLength
    self.ymax = ymax
    
    pts = vtk.vtkPoints()
    pts.InsertPoint(0, xmin, ymin, 0)
    pts.InsertPoint(1, xmax, ymin, 0)
    pts.InsertPoint(2, xmax, ymax, 0)
    pts.InsertPoint(3, xmin, ymax, 0)
    self.pts = pts
    rect = vtk.vtkCellArray()
    rect.InsertNextCell(5)
    rect.InsertCellPoint(0)
    rect.InsertCellPoint(1)
    rect.InsertCellPoint(2)
    rect.InsertCellPoint(3)
    rect.InsertCellPoint(0)
    selectRect = vtk.vtkPolyData()
    selectRect.SetPoints(pts)
    selectRect.SetLines(rect)
    rectMapper = vtk.vtkPolyDataMapper2D()
    rectMapper.SetInput(selectRect)
    rectActor = vtk.vtkActor2D()
    rectActor.SetMapper(rectMapper)

    # Create labels for points
    #oldscalars = dataset.GetOutput().GetPointData().GetScalars()
    #oldscalars.PrintSelf()
    dataset.GetOutput().GetPointData().SetActiveScalars(attr)
    visPts = vtk.vtkSelectVisiblePoints()
    visPts.SetInput(dataset.GetOutput())
    visPts.SetRenderer(ren)
    visPts.SelectionWindowOn()
    visPts.SetSelection(xmin, xmin + xLength, ymin, ymin + yLength)
    self.visPts = visPts
    
    #dataset.SetScalars(oldscalars)

# Create the mapper to display the point ids.  Specify the format to
# use for the labels.  Also create the associated actor.
    ldm = vtk.vtkLabeledDataMapper()
    ldm.SetInput(visPts.GetOutput())
    ldm.SetLabelFormat("%.2g")
    ldm.SetLabelModeToLabelScalars()
    pointLabels = vtk.vtkActor2D()
    pointLabels.SetMapper(ldm)

# Create labels for cells
    '''
    dataset.GetOutput().GetCellData().SetActiveScalars("mindepth")
    cc = vtk.vtkCellCenters()
    cc.SetInput(dataset.GetOutput())
    visCells = vtk.vtkSelectVisiblePoints()
    visCells.SetInput(cc.GetOutput())
    visCells.SetRenderer(ren)
    visCells.SelectionWindowOn()
    visCells.SetSelection(xmin, xmin + xLength, ymin, ymin + yLength)
    self.visCells = visCells
# Create the mapper to display the cell ids.  Specify the format to
# use for the labels.  Also create the associated actor.
    cellMapper = vtk.vtkLabeledDataMapper()
    cellMapper.SetInput(visCells.GetOutput())
    cellMapper.SetLabelFormat("%g")
    cellMapper.SetLabelModeToLabelScalars()
    cellMapper.GetLabelTextProperty().SetColor(0, 1, 0)
    cellLabels = vtk.vtkActor2D()
    cellLabels.SetMapper(cellMapper)
    '''

# Create the RenderWindow and RenderWindowInteractor
# Add the actors to the renderer; set the background and size;
# render
    ren.AddActor2D(rectActor)
    ren.AddActor2D(pointLabels)
    #ren.AddActor2D(cellLabels)

# Create a function to move the selection window across the data set.
  def MoveWindow():
    for y in range(100, 300, 25):
      for x in range(100, 300, 25):
        PlaceWindow(x, y) 


# Create a function to draw the selection window at each location it
# is moved to.
  def PlaceWindow(xmin, ymin):
    global xLength, yLength, visPts, visCells, pts, renWin

    xmax = xmin + xLength
    ymax = ymin + yLength

    self.visPts.SetSelection(xmin, xmax, ymin, ymax)
    self.visCells.SetSelection(xmin, xmax, ymin, ymax)

    self.pts.InsertPoint(0, xmin, ymin, 0)
    self.pts.InsertPoint(1, xmax, ymin, 0)
    self.pts.InsertPoint(2, xmax, ymax, 0)
    self.pts.InsertPoint(3, xmin, ymax, 0)
# Call Modified because InsertPoints does not modify vtkPoints
# (for performance reasons)
    self.pts.Modified()
