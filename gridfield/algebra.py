
import selfe as corie
import copy
import gridfield.core as gridfield 
import log
import time
import re
import xmlrpclib as xrl
from gridfield.core import makeArrayReader, BindOp
import numpy as np

def attachclass(obj):
  #attaches the class name as an attribute for transmission via xmlrpc
  obj.classname = str(obj.__class__)

module = "gridfield.algebra"

def queryFromDict(op):
  #Constructs an operator tree from the dictionary tree returned by xmlrpc
  if "Scan" in op["classname"]:
    return Scan(op["address"], op["name"])

  if "Restrict" in op["classname"]:
    r = Restrict(op["condition"], op["dim"], queryFromDict(op["previous"]))
    return r

  if "Bind" in op["classname"]:
    return Bind(op["address"], op["attr"], op["dim"], queryFromDict(op["previous"]), op["ptrs"])

  if "Cross" in op["classname"]:
    return Cross(queryFromDict(op["left"]), queryFromDict(op["right"]))

  if "Apply" in op["classname"]:
    return Apply(op["expr"], op["dim"], queryFromDict(op["previous"]))

  if "Accumulate" in op["classname"]:
    return Accumulate(op["attr"], op["expr"], op["seed"], op["dim"], queryFromDict(op["previous"]), op["offset"])

  if "Aggregate" in op["classname"]:
    return Aggregate(queryFromDict(op["left"]), op["i"], op["assign"], op["aggregate"], queryFromDict(op["right"]), op["j"])

  if "Regrid" in op["classname"]:
    return Aggregate(queryFromDict(op["left"]), op["i"], op["assign"], op["aggregate"], queryFromDict(op["right"]), op["j"])

  if "Fetch" in op["classname"]:
    return Fetch(queryFromDict(op["previous"]))

  raise TypeError("Unknown operator: " + str(op))

catalog = corie.CORIECatalog()

i=0
def newid():
  global i
  i=i+1
  return i

# Properties of query fragments

def SameGrid(laterOp, earlierOp=None):
  '''
Input: An operator.  The query plan is traversed until 
earlierOp is reached, checking for any operators that
affect the grid.  If any are found, True is immediately returned.
  
  If earlierOp is provided but not found, a ValueError is raised.

  If earlierOp is not provided, the search continues to the
  leaves (Scans) of the tree.

  '''
  print laterOp.__class__
  if laterOp is earlierOp: return True
  elif isinstance(laterOp, Scan):  return True
  elif isinstance(laterOp, Restrict) \
    or isinstance(laterOp, Merge) \
    or isinstance(laterOp, Cross):
    return False
  elif isinstance(laterOp, Aggregate):
    return SameGrid(laterOp.left, earlierOp)
  elif isinstance(laterOp, UnaryOperator):
    return SameGrid(laterOp.previous, earlierOp)
  else:
    raise ValueError("Unknown Operator: %s" % (laterOp,))
  

def SubGrid(e, t):
  print e.__class__
  if isinstance(e, Scan) and isinstance(t, Scan): return True
  elif isinstance(laterOp, Scan):  return True
  elif isinstance(laterOp, Restrict) \
    or isinstance(laterOp, Merge) \
    or isinstance(laterOp, Cross):
    return False
  elif isinstance(laterOp, Aggregate):
    return SameGrid(laterOp.left, earlierOp)
  elif isinstance(laterOp, UnaryOperator):
    return SameGrid(laterOp.previous, earlierOp)
  else:
    raise ValueError("Unknown Operator: %s" % (laterOp,))

  

class Operator:
  def __repr__(self):
    attrs = ",".join(["nm=val" for (nm, val) in self.__dict__.iteritems()])
    return "%s(%s)" % (self.__class__, attrs)

  def __setattr__(self, a, v):
    self.__dict__['dirty'] = True
    self.__dict__[a] = v

  def __del__(self):
    #print "~" + self.__class__.__name__.upper()
    if hasattr(self, '_Result'):
      self._Result.unref()

  def SameGrid(self, otherOp):
    return SameGrid(self, otherOp)

  def idstr(self):
    return "%i: %s" % (self.oid(), self.opstr())

  def Updated(self, since):
    if hasattr(self, "_physicalop"):
      return self._physicalop.Updated(since)
    else:
      return False

  def callCPPMethod(self):
    G = self._physicalop.getResult()
    #print "GETTING RESULT for ", self.__class__.__name__, G.refcount
    return G
    
  def marshallable(self):
    c = copy.copy(self)
    c.classname = str(c.__class__)
    try:
      del c._physicalop
    except AttributeError:
      pass
    try:
      del c._Result
    except AttributeError:
      pass
    return c

  def oid(self):
    return self._oid

  
  def getResult(self):
    if not self.__dict__.has_key("_physicalop"):
      self.init()
    self.execute()
    return self._Result

class ZeroaryOperator(Operator):
  def __init__(self):
    self._oid = newid()

  def map(self, f):
    f(self)

  def Inputs(self):
    # no children
    if False: yield self

  def marshallable(self):
    return Operator.marshallable(self)

  def mkGraph(self, addEdge, addNode):
    addNode(self.oid(), self.opstr())

  def execute(self):
    log.opentag(self)
    try:
      self._Result = self.callCPPMethod()
    finally:
      log.closetag(self)
    self.dirty = False

class UnaryOperator(Operator):
  def __init__(self, previous):
    self.previous = previous
    self._oid = newid()
  
  def SetPrevious(self, op):
    self.previous = op
    if hasattr(self, "_physicalop"):
      if not hasattr(op, "_physicalop"):
        op.init()
      self._physicalop.SetPrevious(op._physicalop)
   
  def Inputs(self):
    yield self.previous

  def map(self, f):
    f(self)
    self.previous.map(f)
  
  def marshallable(self):
    c = Operator.marshallable(self)
    c.previous = self.previous.marshallable()
    return c

  def execute(self):
    if self._physicalop.Updated(self._physicalop.getModTime()):
      self._physicalop.Update()
    log.opentag(self)
    try:
      self.previous.execute()
      self._Result = self.callCPPMethod()
    finally:
      log.closetag(self)
    self.dirty = False
  
  def mkGraph(self, addEdge, addNode):
    self.previous.mkGraph(addEdge, addNode)
    addNode(self.oid(), self.opstr())
    addEdge(self.oid(), self.previous.oid())

  def init(self):
   if isinstance(self.previous, gridfield.GridField):
     self.previous = Wrap(self.previous)
   self.previous.init()
   return not hasattr(self, "_physicalop") 
 
class BinaryOperator(Operator):
  def __init__(self, left, right):
    self.left = left
    self.right = right
    self._oid = newid()
   
  def Inputs(self):
    yield self.left
    yield self.right
     
  def map(self, f):
    f(self)
    self.left.map(f)
    self.right.map(f)  
    
  def SetLeft(self, op):
    self.left = op
    if hasattr(self, "_physicalop"):
      if not hasattr(op, "_physicalop"):
        op.init()
      self._physicalop.SetLeft(op._physicalop)

  def SetRight(self, op):
    self.right = op
    if hasattr(self, "_physicalop"):
      if not hasattr(op, "_physicalop"):
        op.init()
      self._physicalop.SetRight(op._physicalop)

  def marshallable(self):
    c = Operator.marshallable(self)
    c.left = self.left.marshallable()
    c.right = self.right.marshallable()
    return c
    
  def execute(self):
    if self._physicalop.Updated(self._physicalop.getModTime()):
      self._physicalop.Update()
    log.opentag(self)
    try:
      self.left.execute()
      self.right.execute()
      self._Result = self.callCPPMethod()
    finally:
      log.closetag(self)
    self.dirty = False

  def mkGraph(self, addEdge, addNode):
    self.left.mkGraph(addEdge, addNode)
    self.right.mkGraph(addEdge, addNode)
    addNode(self.oid(), self.opstr())
    addEdge(self.oid(), self.left.oid())
    addEdge(self.oid(), self.right.oid())

  def init(self):
    if isinstance(self.left, gridfield.GridField):
     self.left = Wrap(self.left)
    if isinstance(self.right, gridfield.GridField):
     self.right = Wrap(self.right)
    self.left.init()
    self.right.init()
    return not hasattr(self, "_physicalop")

  def init2(self):
    if isinstance(self.left, gridfield.GridField):
     self.left = Wrap(self.left)
    if isinstance(self.right, gridfield.GridField):
     self.right = Wrap(self.right)
    if isinstance(self.state, gridfield.GridField):
     self.state = Wrap(self.state)
    self.left.init()
    self.right.init()
    return not hasattr(self, "_physicalop")

class Scan(ZeroaryOperator):
  def __init__(self, address, name):
    ZeroaryOperator.__init__(self)
    self.address = eval("dict(%s)" % (address,))
    self.name = name

  def __repr__(self):
    return "Scan(%s, %s)" % (self.address, self.name)

  def opstr(self):
    return "Scan(%s)" % (self.name,)

  def init(self):
    self._Result = catalog.loadGridField(self.address, self.name)
    self._physicalop = self._Result
    self._physicalop.thisown = False

  def execute(self):
    #self._Result = catalog.loadGridField(self.address, self.name)
    log.opentag(self)
    log.closetag(self)

class Lift(ZeroaryOperator):
  def __init__(self, gf):
    ZeroaryOperator.__init__(self)
    self._Result = gf

  def __repr__(self):
    return "Lift"
    
  def opstr(self):
    return "Lift"
  
  def init(self):
    self._physicalop = self._Result

  def execute(self):
    pass
 
class Restrict(UnaryOperator):
  def __init__(self, condition, dim, gf):
    UnaryOperator.__init__(self, gf)
    self.dim = dim
    self.condition = condition
    
  def __repr__(self):
    return "Restrict(%s, %s, %s)" % (self.condition, self.dim, self.previous)


  def opstr(self):

    return "Restrict(%s)" % (self.condition,)
    
  def init(self):
    if UnaryOperator.init(self):
      self._physicalop = gridfield.RestrictOp(self.condition, self.dim, self.previous._physicalop)

class Remesh(UnaryOperator):
  def __init__(self, attributex,attributey,grid, gf):
    UnaryOperator.__init__(self, gf)
    self.attributex = attributex
    self.attributey = attributey
    self.grid=grid
    
  def __repr__(self):
    return "Remesh(%s, %s, %s, %s)" % (self.attributex, self.attributey,self.grid,self.previous._physicalop)


  def opstr(self):
    return "Remesh(%s)" % (self.attributex)

  def init(self):
    if UnaryOperator.init(self):
      #try:
      self._physicalop = gridfield.RemeshOp(self.attributex, self.attributey,  self.previous._physicalop,self.grid)
  
class Bind2(UnaryOperator):
  def __init__(self, address, attr, dim, gf, ptrs=""):
    UnaryOperator.__init__(self, gf)
    self.address = eval("dict(%s)" % (address,))
    self.dim = dim
    self.attr = attr
    self.ptrs = ptrs

  def __repr__(self):
    return "Bind(%s, %s, %s, %s, %s)" % (self.address, self.attr, self.ptrs, self.dim, self.previous)
    
  def opstr(self):
    return "Bind(%s, %s, %s)" % (self.attr, self.ptrs, self.dim)
    
  def init(self):
    self.previous.init()
      
  def execute(self):
    gf = self.previous._physicalop.getResult()
    for k, v in self.address.iteritems():
      a = gridfield.ConstArray(k, gf.Size(self.dim), v)
      a.thisown = False
      gf.Bind(self.dim, a)
    readcalls = catalog.gatherReadCalls(gf)
    ##send readcalls to server, if necessary 
    buffer = catalog.ReadData(readcalls)
    #array = catalog.Reorder(gf, buffer)
    newarr = gridfield.Array(self.attr, gridfield.FLOAT)
    newarr.setVals(buffer, len(buffer))
    newarr.thisown = False
    gf.Bind(newarr)
    self._Result = gf
   
class BindConstant(UnaryOperator):
  def __init__(self, attr, val, dim, gf):
    self.attr = attr
    self.dim = dim
    self.val = val
    UnaryOperator.__init__(self, gf)
  
  def ChangeContext(self, attr, val):
    self._physicalop.setAttributeVal(attr, val)
    
  def __repr__(self):
    return "BindConstant(%s, %s, %s, %s)" % (self.attr, self.val, self.dim, self.previous)
  
  def opstr(self):
    return "BindConstant(%s, %s, %s)" % (self.attr, self.val, self.dim)
    
  def init(self):
    if UnaryOperator.init(self):
      self._physicalop = gridfield.BindConstantOp(self.attr, self.val, 
                                           self.previous._physicalop)

class Wrap(ZeroaryOperator):
  def __init__(self, gf):
    ZeroaryOperator.__init__(self)
    self.gf = gf
    gf.thisown = 0
    self._physicalop = gf
    gf.ref()
    ZeroaryOperator.__init__(self)

  def __repr__(self):
    return "Wrap(%s)" % (self.gf.GetGrid().getName(),)

  def opstr(self):
    return self.gf.GetGrid().getName()

  def init(self):
    pass

  def set(self, gf):
    gf.ref()
    self._physicalop.unref()
    self._physicalop = gf

  def execute(self):
    self._Result = self.gf
      
class Bind(UnaryOperator):
#  def __init__(self, address, attr, dim, gf, ptrs="", gftype=gridfield.FLOAT):
#    UnaryOperator.__init__(self, gf)
  def __init__(self, name, array, dim, previous, gftype=gridfield.FLOAT, patternAttribute=None):
    self.array = array
    self.name  = name
    self.previous=previous
    self.patternAttribute=patternAttribute;
    if array.dtype==np.int32:
      self.type=gridfield.INT
    else:
      self.type=gftype;
#    if type(address) == type("foo"):
      # in case we are passed a string of python syntax
#      self.address = eval("dict('%s')" % (address,))
#    else:
#      self.address = dict(address)
#    self.attr = attr
    self.dim = dim
#    self.ptrs = ptrs

  def ReplaceContext(self, context):
    self.address = context.copy()
    self.dirty = True
    if self.__dict__.has_key("_physicalop"):
##      ar = catalog.getArrayReader(self.address, self.attr, self.ptrs)
      ar = makeArrayReader(self.array,len(self.array))
      ar =  makeArrayReader(self.array,len(self.array))
      if (self.patternAttribute !=None):
         ar.setPatternAttribute(self.patternAttribute)
      self._physicalop.setArrayReader(ar)
    #fn, o = catalog.resolveContext(self.address)
    #self._physicalop.setFilename(fn)
    #self._physicalop.setOffsetInt(o)

  def ChangeContext(self, attr, val):
    self.address[attr] = val
    #fn, o = catalog.resolveContext(self.address)
    #self._physicalop.setFilename(fn)
    #self._physicalop.setOffsetInt(o)
    if self.__dict__.has_key("_physicalop"):
#      ar = catalog.getArrayReader(self.address, self.attr, self.ptrs)
      ar = makeArrayReader(self.array,len(self.array))
      ar = makeArrayReader(self.array,len(self.array))
      if (self.patternAttribute !=None):
         ar.setPatternAttribute(self.patternAttribute)
      self._physicalop.setArrayReader(ar)



  def __repr__(self):
    return "Bind(%s, %s, %s, %s, %s,%s)" % (self.name, self.array, self.dim, self.previous, self.gftype, self.patternAttribute)
    

  def opstr(self):
    return "Bind(%s, %s, %s)" % (self.attr, self.dim, self.ptrs)
    
  def init(self):
    #try:
#    if isinstance(self.previous, gridfield.GridField):
#     self.previous = Wrap(self.previous)
#    self.previous.init()
    #except AttributeError:  
     #self.previous=Wrap(self.previous) 
     #self.previous.init()
    if UnaryOperator.init(self):
      #fn, o = catalog.resolveContext(self.address)
      #ef = gridfield.ElcircFile(fn)
      #ar = ef.getVariableReader(0, "addr")
      #fn, o = catalog.resolveContext(self.address)
      #ar = catalog.getArrayReader(self.address, self.attr, self.ptrs)
      ar = makeArrayReader(self.array,len(self.array))
      if (self.patternAttribute !=None):
         ar.setPatternAttribute(self.patternAttribute)
      #try:
      self._physicalop = BindOp(self.name,self.type, ar, self.dim, self.previous._physicalop)
      #except AttributeError:  
      # self.previous=Wrap(self.previous)    
      # self._physicalop = BindOp(self.name,self.type, ar, self.dim, self.previous._physicalop)

class Cross(BinaryOperator):
  def __init__(self, left, right):
    BinaryOperator.__init__(self, left, right)

  def __repr__(self):
    return "Cross(%s, %s)" % (self.left, self.right)
  
  def opstr(self):
    return "X"
  
  def init(self):
    if BinaryOperator.init(self):
      self._physicalop = gridfield.CrossOp(self.left._physicalop, self.right._physicalop)
    
class Apply(UnaryOperator):
  def __init__(self, expr, dim, gf):
    UnaryOperator.__init__(self, gf)
    self.expr = expr
    self.dim = dim
    
  def __repr__(self):
    return "Apply(%s, %s, %s)" % (self.expr, 
                                  self.dim, 
                                  self.previous)

  def SetExpression(self, expr):
    self._physicalop.SetExpression(expr)

  def ChangeContext(self, attr, val):
    self._physicalop.SetExpression("%s=%s" % (attr, val))

  def opstr(self):
    return "Apply(%s, %s)" % \
       (';\n'.join(self.expr.split(';')),self.dim)

  def init(self):
    if UnaryOperator.init(self):
      self._physicalop = gridfield.ApplyOp(self.expr, self.dim, self.previous._physicalop)

class Accumulate(UnaryOperator):
  def __init__(self, attr, expr, seed, dim, gf, offset=0):
    UnaryOperator.__init__(self, gf)
    self.expr = expr
    self.seed = seed
    self.attr = attr
    self.dim = dim
    self.offset = offset

  def __repr__(self):
    return "Accumulate(%s, %s, %s, %s, %s, %s)" \
            % (self.attr, self.expr, self.seed, self.dim, self.previous, self.offset)

  def opstr(self):
    return "Accumulate(%s, %s, %s, %s, %s)" \
            % (self.attr, self.expr, self.seed, self.dim, self.offset)

  def init(self):
    if UnaryOperator.init(self):
      self._physicalop = gridfield.AccumulateOp(self.previous._physicalop, self.dim,
                                             self.attr, 
                                             self.expr, 
                                             self.seed)
    self._physicalop.SetOffset(self.offset)

class Sift(UnaryOperator):
  def __init__(self, dim, previous):
    UnaryOperator.__init__(self, previous)
    self.dim = dim
    
  def init(self):
    if UnaryOperator.init(self):
      self._physicalop = gridfield.SiftOp(self.dim, self.previous._physicalop)
    
  def __repr__(self):
    return "Extend(%s, %s)" % (self.dim,self.previous)

  def opstr(self):
    return "Extend(%s)" % (self.dim,)

class Aggregate(BinaryOperator):
  def __init__(self, assign, agg, left, i, right, j):
    BinaryOperator.__init__(self, left, right)
    self.assign = assign
    self.i = i
    self.j = j
    self.aggregate = agg
    
  def init(self):
    if BinaryOperator.init(self):
      self._physicalop \
           = gridfield.AggregateOp(self.left._physicalop, self.i,
                                 self.assign, 
                                 self.aggregate, 
                                 self.right._physicalop, self.j) 

  def __repr__(self):
    return "Regrid(%s, %s, %s, %s, %s, %s)" % (self.left, self.i, self.assign, self.aggregate, self.right, self.j)

  def opstr(self):
    return "Regrid(%s, %s)" % (self.assign.__class__.__name__, self.aggregate.__class__.__name__)



class Merge(BinaryOperator):
  def __init__(self, left, right):
    BinaryOperator.__init__(self, left, right)
    
  def init(self):
    if BinaryOperator.init(self):
      self._physicalop = gridfield.MergeOp(self.left._physicalop, 
                                        self.right._physicalop)

  def __repr__(self):
    return "Merge(%s, %s)" % (self.left, self.right)

  def opstr(self):
    return "Merge"

class Tag(BinaryOperator):
  def __init__(self, left, i, right, j):
    BinaryOperator.__init__(self, left, i, right, j)
    self.i = i
    self.j = j

  def init(self):
    if BinaryOperator.init(self):
      self._physicalop = gridfield.TagOp(self.left._physicalop, self.i,
                                      self.right._physicalop, self.j)

  def __repr__(self):
    return "Tag(%s, %s, %s, %s)" % (self.left, self.i, self.right, self.j)

  def opstr(self):
    return "Tag"

class State(UnaryOperator):
  def __init__(self, previous):
    self.refresh = True
    UnaryOperator.__init__(self, previous)

  def init(self):
    if UnaryOp.init(self):
      self._physicalop = gridfield.StateOp(self.previous._physicalop)

  def SetState(self, state):
    self.refresh = False
    self._Result = gridfield.GridField(state.getResult())
    self._physicalop.SetState(self._Result)
 

  def getResult(self):
    self.execute()
    return self._Result

  def execute(self):
    if self.refresh:
      UnaryOperator.execute(self)
      

  def __repr__(self):
    return "State(%s)" % (self.previous,)

  def opstr(self):
    return "State"

class Iterate(BinaryOperator):
  def __init__(self, state, left, i, right, j):
    self.state = state
    BinaryOperator.__init__(self, left, i, right, j)
    self.i = i
    self.j = j
    
  def init(self):
    self.state.init()
    self.left.init()
    self.right.init()
    if not self.__dict__.has_key("_physicalop"):
      self._physicalop = gridfield.IterateOp(self.state._physicalop,
                                          self.left._physicalop, 
                                          self.right._physicalop)

  def __repr__(self):
    return "Iterate(%s, %s, %s)" % (self.state, self.left, self.right)

  def opstr(self):
    return "Iterate"

class ScanLocal(ZeroaryOperator):
  def __init__(self, filename, offset=0):
    ZeroaryOperator.__init__(self)
    self.fn = filename
    self.offset = offset

  def __repr__(self):
    return "ScanLocal(%s)" % self.fn

  def opstr(self):
    return self.__repr__()

  def SetFileName(self, fn):
    self.fn = fn
    if hasattr(self,"_physicalop"):
      self._physicalop.setFileName(fn)

  def init(self):
    if not hasattr(self,"_physicalop"):
      self._physicalop = gridfield.ScanInternal(self.fn, self.offset)

class Project(UnaryOperator):
  def __init__(self, attrs, i, previous):
    self.attrs = attrs
    self.i = i
    UnaryOperator.__init__(self, previous)
  
  def __repr__(self):
    return "Project(%s, %s, %s)" % (self.attrs, self.i, self.previous)
    
  def opstr(self):
    return "Project(%s)" % (self.attrs,)

  def init(self):
    if UnaryOp.init(self):
      self._physicalop = gridfield.ProjectOp(self.previous._physicalop, self.i, self.attrs)

class Fetch(UnaryOperator):
  def __init__(self, previous, host="http://localhost:9000"):
    UnaryOperator.__init__(self, previous) 
    self.host = host

  def __repr__(self):
    return "Fetch(%s)" % (self.previous,)
    
  def opstr(self):
    return "Fetch"
  
  def init(self):
    self.plan = self.getplan()
    result = self.fetch(self.plan)
    if not hasattr(self,"_physicalop"):
      self._physicalop = gridfield.ScanInternal(result.data)
  
  def getplan(self):
    c = self.previous.marshallable()
    c.map(attachclass)
    return c

  def fetch(self, plan):
    self.server = xrl.ServerProxy(self.host)
    result = self.server.execute(plan)
    return result

  def execute(self):
    log.opentag(self)
    plan = self.getplan()
    if plan != self.plan:
      self.plan = plan
      result = self.fetch(plan)
      self._physicalop.setRawBytes(result.data)
    try:
      self._Result = self.callCPPMethod()
    finally:
      log.closetag(self)

class Store(UnaryOperator):
  def __init__(self, filename, previous):
    UnaryOperator.__init__(self, previous)
    self.filename = filename

  def __repr__(self):
    return "Store(%s, %s)" % (self.filename, self.previous)

  def opstr(self):
    return "Store(%s)" % (filename,)

  def SetFileNamet(self, filename):
    self.filename = filename
    if self.__dict__.has_key("_physicalop"):
      self._physicalop.setFileName(filename)

  def init(self):
    if UnaryOp.init(self):
      self._physicalop = gridfield.OutputOp(self.filename,0,self.previous._physicalop)
    

def LinearGridField(vals, attr):
  # vals might be a slice expression
  vals = vals.replace('-',':')
  vals = vals.replace(':',' ')
  slice = [int(x) for x in vals.split()]
  if len(slice) > 1 and len(slice) < 4:
    newvals = range(*slice)
    sz = len(newvals)
  else:
    # or it might be an explicit list of values
    # or it might be a singleton value
    newvals = [float(x) for x in vals.split(',')]
    sz = len(newvals)
  
  g = gridfield.OneGrid("Linear", sz)
  g.thisown = False
  gf = gridfield.GridField(g)
  g.unref()
  a = gridfield.Array(attr, gridfield.FLOAT)
  a.thisown = False
  a.copyFloatData(newvals, sz)
  gf.Bind(0, a)
  a.show()
  a.unref()

  return gf
