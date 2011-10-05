import algebra as gf

def gfV(context):
  V = gf.Scan(context, "V")
  ordV = gf.Accumulate("vpos", "vpos+1", "0", 0, V)
  return ordV

def gfH(context):
  H = gf.Scan(context, "H")
  ordH = gf.Accumulate("hpos", "hpos+1", "0", 0, H)
  return ordH

def zoom(bounds, H):
  zoom = gf.Restrict("(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds, 0, H)
  return zoom

def BindSome(context, attrs, X, ptr_attribute="addr"):
  if not attrs:
    raise ValueError("No attributes provided.")

  if isinstance(attrs, str):
    attrs = [attrs]
  
  context["var"] = attrs[0]
  B = gf.Bind(context, attrs[0], 0, X, ptr_attribute)
  for a in attrs[1:]:
    context["var"] = a
    B = gf.Bind(context, a, 0, B, ptr_attribute)
    
  return B 

def WetGridV3(context, bounds, vars):
  H = gf.Scan(context, "H")
  V = gf.Scan(context, "V")

  dH = gf.Apply("depth=60*(4825.1-h)", 0, H)
                                                                    
  aV = gf.Accumulate("vpos", "vpos+1", "0", 0, V)
  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, dH, -1)
                                                                 
  rH = zoom(bounds, aH)

  rV = gf.Restrict("(1560<z) & (z<4866)", 0, aV)
  #rV = Restrict("vpos=%s" % (fV.getResult().Card(0)-1,), aV)
  V = gf.Restrict("vpos=60", 0, aV)
                                                                    
  HxV = gf.Cross(rH, rV)
                                                                    
  wetgrid = gf.Restrict("vpos > (b-1)", 0, HxV)

  varaddr = gf.Apply("z=z*60; addr=column+vpos-b", 0, wetgrid)
   
  B = gf.BindSome(context, vars, varaddr)

  trim = gf.Restrict("%s>-98" % (var,), 0, B)
  vis.Inspect(trim,var)
  
def WetGridV4(context, bounds, var):
  H = gfH(context)
  V = gfV(context)

  rH = zoom(bounds, H)

  HxV = gf.Cross(rH, V)
  addr = gf.Apply("addr=%s*hpos + vpos" % (V.getResult().Card(0),), 0, HxV)

  B = BindSome(context, var, addr)

def WetGridV5(context, bounds, var, zcond="vpos>0"):
  H = gfH(context)
  V = gfV(context)
  V0 = gf.Sift(0,V)
  rV = gf.Restrict(zcond, 0, V0)
  aH = gf.Accumulate("column", "column+%i-b" % V.getResult().Card(0), "0", 0, H, -1)
  #aaH = gf.Apply("h=(h>130)*130 + (h<130)*h", 0, aH)
  rH = zoom(bounds, aH)

  contextsurf = context.copy()
  contextsurf["var"] = "surf"
  surfH = gf.Bind(contextsurf, "surf", 0, rH, "hpos")

  HxV = gf.Cross(surfH, rV)
  wetgrid = gf.Restrict("(vpos>b+1) & (b>0)", 0, HxV)
  addr = gf.Apply("addr=column - b + vpos", 0, wetgrid)
  B = BindSome(context, var, addr)
  #X = gf.Apply("foo=u*2", 0, B)
  Z = gf.Apply("z=(vpos>16)*z*(h-surf)*60 + (vpos<17)*z*60", 0, B)
  return Z
#  T = gf.Scan(context, "T")

