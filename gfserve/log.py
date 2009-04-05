import time
import xml.sax.saxutils
logfile = "/tmp/gridfield_log.xml"

f = file(logfile, "a")

global indent 
indent = 0
tab = 2 

def makeAttr(p,v):
  return "%s=%s" % (p, xml.sax.saxutils.quoteattr(str(v)))

params = ["previous", "left", "right", "_physicalop", "_Result"]
def param(p):
 if p in params: return False
 else: return True

def OpToTag(op):
  tag = "gf:%s" % (str(op.__class__).split(".")[-1])
  return tag

def opentag(op):
  global indent 
  tag = OpToTag(op)
  attrs = op.__dict__
  attrstr = ' '.join([makeAttr(p,v) for p,v in attrs.iteritems() if param(p) ])
  #opener = "<%s>\n" % (tag,)
  opener = "%s<%s %s>\n" % (indent*" ", tag, attrstr)
  f.write(opener)
  indent += tab
  exectime = "%s<start>%s</start>\n" % (indent*" ", time.time())
  f.write(exectime)
  
def closetag(op):
  global indent
  exectime = "%s<end>%s</end>\n" % (indent*" ", time.time())
  f.write(exectime)
  
  tag = OpToTag(op)
  closer = "%s</%s>\n" % (indent*" ", tag)
  #closer = "</%s>\n" % (tag,)
  f.write(closer)
  indent -= tab
