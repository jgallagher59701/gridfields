#!/usr/bin/env python
import sys
import os
import os.path
#import gather
import struct

import gridfield.core as gf

class CORIECatalog:
  def __init__(self, path="/home/workspace/ccalmr"):
    self.filecache = {}
    self.path = path
    self.varsuffix = {
       "zcor":"63", 
       "salt":"63", 
       "hvel":"64", 
       "temp":"63", 
       "elev":"61", 
       "vert":"63"
    }
    self.varfile = {
       "z":"zcor",
       "salt":"salt",
       "surf":"salt",  # surf is the same in all files
       "u":"hvel",
       "v":"hvel",
       "temp":"temp",
       "elev":"elev",
       "vert":"vert",
       "w":"vert"
    }

  #def gatherReadCalls(self, gf):
  #  return gather.gather(gf, self.resolveTuple)

  def ReadData(self, readcalls):
    xs = []
    for fn, rds in readcalls.iteritems():
      f = file(fn)
      for pos, sz in rds:
        f.seek(pos)
        xs = xs + list(struct.unpack("%if" % (sz/4,), f.read(sz)))
    return xs
      
  def getForecastFilename(self, db, year, dayofyear, var, runday):
    #path = "/%s/%s/%s-%s/run/%s_%s.%s" \
    path = "/forecasts/%s/%s/%s-%s/run/%s_%s.%s" \
          % (db, year, year, dayofyear, runday, var, self.varsuffix[var])
    return self.path + path
    
  def getHindcastFilename(self, var, week, db, year, day):
    path = "/hindcasts/%s-%s-%s/run/%s_%s.%s" \
         % (year, week, db, day, var, self.varsuffix[var])
    return self.path + path

  def resolveTuple(self, tuple):
    c = {}
    c['mode'] = derefPyObject(tuple.get("mode"))
    c['file'] = derefPyObject(tuple.get("file"))
    c['db'] = derefPyObject(tuple.get("db"))
    c['day'] = derefPyObject(tuple.get("day"))
    c['year'] = derefPyObject(tuple.get("year"))
    c['runday'] = derefPyObject(tuple.get("runday"))
    c['tstep'] = derefInt(tuple.get("tstep"))
    c['hpos'] = int(derefFloat(tuple.get("hpos")))
    c['vpos'] = int(derefFloat(tuple.get("vpos")))
    
    fn = self.fileFromContext(c)
    ef = self.elcircfileFromAddress(fn)
    var = derefPyObject(tuple.get("var"))
    if var == "surf":
      offset = ef.getSurfOffset(c['tstep'], c['hpos'])
    else:
      offset = ef.getVariableOffset(c['tstep'], c['hpos'], c['vpos'])

    return fn, offset

  def fileFromContext(self, context):
    a = context
    filetag = self.varfile[a['var']]
    suffix = self.varsuffix[filetag]
    if a['mode'] == 'rundir':
      fn = "%s/%s_%s.%s" % (a['rundir'], a['runday'], filetag, suffix)

    elif a['mode'] == 'forecast':
      fn = self.getForecastFilename(a['db'], a['year'], a['day'], 
                                    filetag, a['runday'])
    
    elif a['mode'] == 'hindcast':
      fn = self.getHindcastFilename(filetag, a['week'], a['db'], 
                                    a['year'], a['runday'])
    else:
      raise ValueError("Unknown mode '%s' in context." % (a['mode']))

    return fn

  def elcircfileFromAddress(self, fn):
    if not os.path.exists(fn):
      raise ValueError("Filename '%s' not found." %(fn,))
    try:
      return self.filecache[fn]
    except:
      pass
      
    ef = gf.ElcircFile(fn)  
    self.filecache[fn] = ef
    
    return ef

  def readTransectGrid(self, address, gridname):
    fn = self.fileFromContext(address)
    dir = os.path.dirname(fn)
    filename = "%s/vslice_%s.bp" % (dir, gridname)
  
    f = file(filename)
    lines = f.readlines()    
    fname_check = lines[0].strip()
    assert(fname_check == os.path.basename(filename))
    nodes = int(lines[1])

    tuples = [line.split() for line in lines[2:]]
    xs = [float(tuple[1]) for tuple in tuples]
    ys = [float(tuple[2]) for tuple in tuples]
    ids = [int(tuple[0]) for tuple in tuples]

    G = gf.OneGrid(gridname, nodes)
    G.thisown = False
    ax = Array("x", FLOAT)
    ax.thisown = False
    ax.copyData(xs, nodes)
    ay = Array("y", FLOAT)
    ay.thisown = False
    ay.copyData(ys, nodes)
    aid = Array("pos", INT)
    aid.thisown = False
    aid.copyData(ids, nodes)
    
    GF = GridField(G, 0)
    GF.thisown = False
    
    GF.Bind(ax)
    GF.Bind(ay)
    GF.Bind(aid)
    
    ay.unref()
    ax.unref()

    return GF

  def loadGridField(self, address, name):

    if name in ["H", "V", "T", "D"]:
      fn = self.fileFromContext(address)
      ef = self.elcircfileFromAddress(fn)
      if name == "H":
        gf = ef.readHGrid()
      elif name == "V":
        gf = ef.readVGrid()
      elif name == "T":
        gf = ef.readTGrid()
      elif name == "D":
        gf = ef.readDGrid()
      del ef
      return gf
    elif name in ["mchann", "ambr", "circ1", "circ2", "circ3", "clriver", "cpmeares", "grharbor", "mouth", "mplume", "nchann", "nplume", "red26", "schann", "splume"]:
      GF = self.readTransectGrid(address, name)
      return GF
    else:
      raise ValueError("No grid named '%s' in this context." % (name,)) 

   
      
  def resolveContext(self, context):
    fn = self.fileFromContext(context)
    ef = self.elcircfileFromAddress(fn)
    if context["var"] == "surf":
      offset = ef.getSurfOffset(int(context["tstep"]))
    else:
      offset = ef.getVariableOffset(int(context["tstep"]))
    
    return fn, offset

  def getArrayReader(self, address, attr, ptrs=""):
    fn = self.fileFromContext(address)
    ef = self.elcircfileFromAddress(fn)
    if address["var"] == "surf":
      sr = ef.getSurfReader(int(address["tstep"]), ptrs)
      return sr
    else:
      r = ef.getVariableReader(address["var"], int(address["tstep"]), ptrs)
      return r

  def getDomainV4(self, fname, 
                  xrange=(334000,339000), 
                  yrange=(290000,296000), 
                  zrange=(-1.01, 0.01)):
        
    ef = ElcircFile(fname)

    V = ef.readVGrid()
    H = ef.readHGrid()

    dH = ApplyOp(H, "depth", "-h*60")

    aV = AccumulateOp(V, "vpos", "vpos+1", "0") 
    aH = AccumulateOp(dH, "hpos", "hpos+1", "0") 

    rV = RestrictOp("(%f<z) & (z<%f)" % (zrange), aV)
    rH = RestrictOp("(%f<x) & (x<%f) & (%f<y) & (y<%f)" % (xrange+yrange), aH)
 
    HxV = CrossOp(rH, rV)

    Gz = ApplyOp(HxV, "z=depth*-z; addr=hpos*%s+vpos" % (V.card(),))

    return GridField(rH.getResult()), GridField(aV.getResult()), GridField(Gz.getResult())
    
  def getDomain(self, fname, xrange=(334000,339000), 
                yrange=(290000,296000), zrange=(1560, 4869)):

    ef = ElcircFile(fname)

    V = ef.readVGrid()
    H = ef.readHGrid()

    dH = ApplyOp(H, "depth", "60*(%s-h)" % (ef.h.zmsl,))

    aV = AccumulateOp(V, "vpos", "vpos+1", "0") 
    aH = AccumulateOp(dH, "column", "column+%i-b" % aV.getResult().card(), "0")
    aH.SetOffset(-1)

    rV = RestrictOp("(%f<z) & (z<%f)" % zrange, aV)
    rH = RestrictOp("(%f<x) & (x<%f) & (%f<y) & (y<%f)" % (xrange+yrange), aH)
    #rV.getResult().getAttribute("z").output()
    
    stretch = ApplyOp(rV, "z", "60*z")
    
    HxV = CrossOp(rH, stretch)
    #HxV = ApplyOp(rH, "vpos", "50")

    bottom = RestrictOp("(vpos > (b-1))", HxV)

    varaddr = ApplyOp(bottom, "addr", "column+vpos-b")
    
    return GridField(rH.getResult()), GridField(rV.getResult()), GridField(varaddr.getResult())

  def bindVar(self, grid, fname, tstep):
    ef = ElcircFile(fname)
    
    #sar = ef.getSurfReader(0, "addr")
    #surf = BindOp("surf", INT, sar, grid)
    
    #top = RestrictOp("(vpos < surf+1)", surf)
    
    ar = ef.getVariableReader(tstep, "addr")
    bind = BindOp("salt", FLOAT, ar, grid)
    #c = Condition("salt", -1, ">")
    #c.thisown = False
    #top = RestrictOp(c, bind)
    top = RestrictOp("(salt > -1)", bind)
    
    # make a copy, since the operators own their results
    return ef, bind
 
def showFrame(tstep):
  global ef, eftemp, binds, bindt, op
  off1 = ef.getVariableOffset(tstep % 96)
  #off2 = eftemp.getVariableOffset(tstep % 96)
  binds.setOffsetInt(off1)
  #bindt.setOffsetInt(off2)
  gf = op.getResult() 
  #gfo.UpdateGrid(salt3, "salt")
  gfo.UpdateScalar(gf, "salt")



