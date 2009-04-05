""" This package associates each gridfield operator with a vistrails module
""" 

import core.modules
import core.modules.module_registry
from core.modules.module_registry import registry
import core.modules.basic_modules
from core.modules.vistrails_module import Module, ModuleError

import gridfield.core as gridfield
import gridfield.algebra as algebra
import gridfield.gfvis as vtkgridfield
import gridfield.selfe as corie
import os

name = 'GridFields'
identifier = 'org.stccmop.gridfield'
version = '1.0'

catalog = corie.CORIECatalog()

###############################################################################


class GridFieldExpression(Module): 
  "A module representing a composed gridfield expression"

  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)

class GridFieldOperator(Module):
    "A base module for all gridfield operators"

    @classmethod
    def RegisterMethods(cls, reg):   
      reg.add_module(cls)
      reg.add_output_port(cls, "Result",
                        (GridFieldExpression, 'The resulting gridfield expression'))
    def GetDimensionParam(self, name):
      if self.hasInputFromPort(name):
        dim = self.getInputFromPort(name)
      else: 
        dim = 0
      return dim

class ZeroaryGridFieldOperator(GridFieldOperator):
    "A class representing gridfield operators with no arguments"
  
    def __init__(self):
      Module.__init__(self)

class UnaryGridFieldOperator(GridFieldOperator):
    "A class representing gridfield operators with a single argument"
        
    __quiet = True
    def __init__(self):
        Module.__init__(self)

    @classmethod
    def RegisterMethods(cls, reg):  
      reg.add_module(cls)
      reg.add_input_port(cls, "GridField",
                       (GridFieldExpression, 'The previous operator'))
       
class BinaryGridFieldOperator(GridFieldOperator):
    "A class representing gridfield operators with two arguments"
    def __init__(self):
        Module.__init__(self)
        
    @classmethod
    def RegisterMethods(cls, reg):  
      reg.add_module(cls)
      reg.add_input_port(cls, "Left GridField",
                       (GridFieldExpression, 'The left gridfield operator'))
      reg.add_input_port(cls, "Right GridField",
                       (GridFieldExpression, 'The right gridfield operator'))

class Context(Module, dict):
    "A Context is just a dictionary module"
    ports = []

    def compute(self):
      ks = [k for k in self.inputPorts.keys()]
      context = dict([(k,self.getInputFromPort(k)) for k in ks])
  
      # name conflict with VisTrails?
      dict.update(self, context)

      self.setResult("context", self) 

    @classmethod
    def RegisterMethods(cls, reg):
      reg.add_module(cls)
      for p in cls.ports:
        reg.add_input_port(cls, p, core.modules.basic_modules.String)

      reg.add_output_port(cls, "context", cls)

class SimulationContext(Context):
    mode = "no mode specified"
    ports = ["year", "db", "var", "runday"]

    def __init__(self):
      Module.__init__(self)

    def compute(self):
      ks = [k for k in self.inputPorts.keys()]
      context = dict([(k,self.getInputFromPort(k)) for k in ks])

      # name conflict with VisTrails?
      dict.update(self, context)
      self['mode'] = self.mode

      self.setResult("context", self) 

class ForecastContext(SimulationContext):
    mode = 'forecast'
    ports = ["day"]

class HindcastContext(SimulationContext):
    mode = 'hindcast'
    ports = ["week"]

class FileContext(SimulationContext):
   mode = 'rundir'
   ports = ["rundir"]

class MPIContext(FileContext):
  mode = 'mpi'
  ports = ["process"]

class TimestepContext(Context):
  ports = ["tstep"] 

  def compute(self):
    ks = [k for k in self.inputPorts.keys()]
    context = dict([(k,self.getInputFromPort(k)) for k in ks])
 
    sim = context["SimulationContext"]
    context.pop("SimulationContext")

    # name conflict with VisTrails?
    dict.update(self, context)
    dict.update(self, sim)

    self.setResult("context", self)

  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)
    for p in cls.ports:
      reg.add_input_port(cls, p, core.modules.basic_modules.String)

    reg.add_input_port(cls, "SimulationContext", SimulationContext)
    reg.add_output_port(cls, "context", cls)

class Scan(ZeroaryGridFieldOperator):
    "An operator to read a gridfield from the catalog"

    def __init__(self): 
      Module.__init__(self)

    @classmethod
    def RegisterMethods(cls, reg): 
      reg.add_module(cls)
      s = core.modules.basic_modules.String
      reg.add_input_port(cls, "name", (s, "The name of the grid to scan"))
      reg.add_input_port(cls, "context", SimulationContext)

    def compute(self):
      runcontext = self.getInputFromPort("context")
      name = self.getInputFromPort("name")
      
      s = algebra.Scan(dict(runcontext), name)

      self.setResult('Result', s) 
      pass

class Apply(UnaryGridFieldOperator):
  "Apply an arithmetic expression to a gridfield at a specific rank"
 
  @classmethod
  def RegisterMethods(cls, reg):
    s = core.modules.basic_modules.String
    f = core.modules.basic_modules.Float
    reg.add_module(cls)
    reg.add_input_port(cls, 'Expression', (s, "Arithmetic expression involving attributes"))
    reg.add_input_port(cls, 'Dimension', (f, "The dimension ('rank') at which to apply the expression"))

  def compute(self):
    gf = self.getInputFromPort('GridField')
    expr = self.getInputFromPort('Expression')

    dim = self.GetDimensionParam('Dimension')

    op = algebra.Apply(expr, dim, gf)
    self.setResult('Result', op) 

class SiftOp(UnaryGridFieldOperator):
  "Remove all cells not required by the given dimension.  For example, use Sift to remove all the edges to conserve memory."
    
  @classmethod
  def RegisterMethods(cls, reg):
    i = core.modules.basic_modules.Integer
    reg.add_module(cls)
    reg.add_input_port(cls, 'Dimension', (f, "The dimension ('rank') of the cells you want to keep."))

  def compute(self):
    gf = self.getInputFromPort('GridField')
    dim = self.GetDimensionParam('Dimension')

    op = algebra.Sift(dim, gf)
    self.setResult('Result', op) 

class Restrict(UnaryGridFieldOperator):
  "Restrict a gridfield using an arbitrary predicate over the attributes"

  @classmethod
  def RegisterMethods(cls, reg):
    s = core.modules.basic_modules.String
    f = core.modules.basic_modules.Float
    reg.add_module(cls)
    reg.add_input_port(cls, 'Expression', (s, "Arithmetic expression involving attributes"))
    reg.add_input_port(cls, 'Dimension', (f, "The dimension ('rank') at which to apply the expression"))


  def compute(self):
    gf = self.getInputFromPort('GridField')
    expr = self.getInputFromPort('Expression')

    dim = self.GetDimensionParam('Dimension')

    op = algebra.Restrict(expr, dim, gf)
    self.setResult('Result', op) 

class Bind(UnaryGridFieldOperator):
  '''Bind a new attribute to a gridfield.
See Howe, Maier 2004; Howe, 2006.'''

  @classmethod
  def RegisterMethods(cls, reg):
    s = core.modules.basic_modules.String
    f = core.modules.basic_modules.Float
    reg.add_module(cls)
    reg.add_input_port(cls, 'TimestepContext', (TimestepContext, "The context from which to extract the attribute"))
    reg.add_input_port(cls, 'Attribute', (s, "The name of the new attribute"))
    reg.add_input_port(cls, 'Variable', (s, "The name of the catalog variable to bind"))
    reg.add_input_port(cls, 'Addresses', (s, "(optional) The name of an attribute that stores addresses of the values to bind."))
    reg.add_input_port(cls, 'Dimension', (f, "The dimension ('rank') at which to bind the new attribute."))
    reg.add_input_port(cls, 'Type', (s, "The type of the new attribute ( INT or FLOAT)."))


  def compute(self):
    gf = self.getInputFromPort('GridField')
    context = self.getInputFromPort('TimestepContext')
    attr = self.getInputFromPort('Attribute')
    typ = self.getInputFromPort('Type')
    
    if self.hasInputFromPort('Addresses'):
      ptrs = self.getInputFromPort('Addresses')
    else:
      ptrs = ""

    if self.hasInputFromPort('Variable'):
      var = self.getInputFromPort('Variable')
    else:
      var = attr
    context['var'] = var
    dim = self.GetDimensionParam('Dimension')

    gtype = eval("gridfield.%s" % (typ,))

    #ar = catalog.getArrayReader(context, attr, ptrs)
    op = algebra.Bind(context, attr, dim, gf, ptrs=ptrs, gftype=gtype)

    self.setResult('Result', op)

class Cross(BinaryGridFieldOperator):
  '''The gridfield cross product of two gridfields.
See Howe, Maier 2004; Howe, 2006.'''
  
  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)

  def compute(self):
    left = self.getInputFromPort('Left GridField')
    right = self.getInputFromPort('Right GridField')
    op = algebra.Cross(left, right)
    self.setResult('Result', op) 

class Regrid(BinaryGridFieldOperator):
  '''Map the data of one gridfield onto the grid of another.
See Howe, Maier 2004; Howe, 2006.'''

  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)
    s = core.modules.basic_modules.String
    reg.add_input_port(cls, 'Assignment Function', (s, "The name of a function to assign each cell in the target Gridfield to a set of cells in the source gridfield"))
    reg.add_input_port(cls, 'Aggregation Function', (s, "The name of a function that will aggregate a set of tuples into a single tuple."))

  def compute(self):
    left = self.getInputFromPort('Left GridField')
    i = self.GetDimensionParam('Left Dimension')

    assignS = self.getInputFromPort('Assignment Function')
    aggS = self.getInputFromPort('Aggregation Function')

    right = self.getInputFromPort('Right GridField')
    j = self.GetDimensionParam('Right Dimension')

  
    assign = eval('gridfield.%s' % (assignS,))
    agg = eval('gridfield.%s' % (aggS,))

    op = algebra.Aggregate(assign, agg, left, i, right, j)
    self.setResult('Result', op) 

class Accumulate(UnaryGridFieldOperator):
  '''A specialization of Regrid that assigns all 'previous' cells 
to the target cell, where previous is defined by the physical order.
Often used to assign integer ids to cells.
See Howe, Maier 2004; Howe, 2006.'''
 
  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)
    s = core.modules.basic_modules.String
    f = core.modules.basic_modules.Float
    i = core.modules.basic_modules.Integer

    reg.add_input_port(cls, 'Attribute', (s, "The attribute name in which to store the accumulated values"))
    reg.add_input_port(cls, 'Expression', (s, "The expression to compute the next value."))
    reg.add_input_port(cls, 'Seed', (s, "The expression which to initialize the value."))
    reg.add_input_port(cls, 'Dimension', (f, "The dimension at which to operate."))
    reg.add_input_port(cls, 'Offset', (i, "An optional offset. Accumulation normally begins at the first element.  Use this parameter to change the behavior.  May be positive or negative."))

  def compute(self):
    gf = self.getInputFromPort('GridField')
    dim = self.GetDimensionParam('Dimension')
    
    expr = self.getInputFromPort('Expression')
    seed = self.getInputFromPort('Seed')
    offset = self.getInputFromPort('Offset')
    attr = self.getInputFromPort('Attribute')
    
    op = algebra.Accumulate(attr, expr, seed, dim, gf, offset)
    self.setResult('Result', op) 
 
class Fetch(UnaryGridFieldOperator):
  '''Execute a gridfield recipe at a remote server'''
  @classmethod
  def RegisterMethods(cls, reg):
    s = core.modules.basic_modules.String
    d = "Url of the GridField Server on which to execute the expression."
    reg.add_module(cls)
    reg.add_input_port(cls, 'Host', (s, d))

  def compute(self):
    gf = self.getInputFromPort('GridField')
    host = self.getInputFromPort('Host')
   
    op = algebra.Fetch(gf, host)

    self.setResult('Result', op) 

class Merge(BinaryGridFieldOperator):
  '''Merge the data of two gridfields that share cells.  The resulting grid is the intersection of the two arguments.
See Howe, Maier 2004; Howe, 2006.'''

  @classmethod
  def RegisterMethods(cls, reg):
    reg.add_module(cls)

  def compute(self):
    left = self.getInputFromPort('Left GridField')
    right = self.getInputFromPort('Left GridField')
    
    op = algebra.Merge(left, right)
    self.setResult('Result', op)

class GridFieldToVTK(UnaryGridFieldOperator):

    @classmethod
    def RegisterMethods(cls, reg): 
      reg.add_module(cls)
      s = core.modules.basic_modules.String
      reg.add_input_port(cls, "scalar", s)
      reg.add_input_port(cls, "vector", s)

      vtkugM = reg.registry.get_descriptor_by_name('edu.utah.sci.vistrails.vtk',
                                                   'vtkUnstructuredGrid').module
      reg.add_output_port(GridFieldToVTK, "VTKUnstructuredGrid",
                      (vtkugM, "VTKUnstructuredGrid output"))
 
      f = core.modules.basic_modules.Float
      reg.add_output_port(GridFieldToVTK, "range", [f,f])

    def compute(self):      
      gfop = self.getInputFromPort('GridField')

      # executes the recipe if necessary -- potentially big step
      gf = gfop.getResult()

      gf.GetGrid().normalize()

      scalar = self.getInputFromPort('scalar')

      convert = vtkgridfield.vtkGridField_New()
      convert.UseNamedPerspective()
 
      convert.SetScalarAttribute(scalar)
        
      if self.hasInputFromPort('vector'):
        vectorstr = self.getInputFromPort('vector')

        vector = vectorstr.split(',')

        if len(vector) == 2:
          convert.SetVectorAttributes(vector[0], vector[1])
        if len(vector) == 3:
          convert.SetVectorAttributes(vector[0], vector[1], vector[2])

      convert.SetGridField(gf)
      convert.Update()
      vtkug = convert.GetOutput()
      print "got output"
      # mangle pointer for vtk's sake
      pythonobj = vtkgridfield.vtkPythonObject(vtkug)

      pd = pythonobj.GetPointData()
      sclrs = pd.GetScalars()
      rng = sclrs.GetRange()

      # Find the wrapper class
      vUGModule = registry.get_descriptor_by_name('edu.utah.sci.vistrails.vtk',
                                                  'vtkUnstructuredGrid').module

      # Wrap it
      wrappedResult = vUGModule()
      wrappedResult.vtkInstance = pythonobj
   
      self.setResult('VTKUnstructuredGrid', wrappedResult) 
      self.setResult('range', list(rng)) 


def RegisterMethods(cls, reg):
   reg.add_module(cls)
   for p in cls.ports:
     reg.add_input_port(cls, p, core.modules.basic_modules.String)

   reg.add_output_port(Scan, "Result",
               (GridFieldExpression, 'The resulting gridfield'))


def initialize(*args, **keywords):


    reg = core.modules.module_registry
 
    moduleclasses = [
               GridFieldExpression,
               GridFieldOperator, ZeroaryGridFieldOperator, 
               UnaryGridFieldOperator, BinaryGridFieldOperator,
               Context, SimulationContext, TimestepContext,
               HindcastContext, ForecastContext, FileContext, MPIContext,
               Scan, Bind,
               Apply, Restrict,
               Cross, Merge, Regrid, Accumulate,
               Fetch, GridFieldToVTK
              ]

    for cls in moduleclasses:
      cls.RegisterMethods(reg)

def package_dependencies():
    return ['edu.utah.sci.vistrails.vtk']

def package_requirements():
    import core.requirements
    if not core.requirements.python_module_exists('vtk'):
        raise core.requirements.MissingRequirement('vtk')

