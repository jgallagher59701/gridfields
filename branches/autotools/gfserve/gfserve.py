import guppy.heapy.RM
from pkg_resources import require
require("gridfield>0.2")

import SimpleXMLRPCServer as serverlib
import xmlrpclib as xrl
import gridfield.algebra as algebra
import sys
import os
import socket
import config
from gridfield.vis import ToVTK

#config = object()
#config.logfile = '/tmp/gfserve.log'

import logging

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename=config.logfile,
                    filemode='w')
def Info(s):
  print s
  logging.info(s)

def debug(s):
  logging.debug(s)

def warning(s):
  logging.warning(s)

import gfrun

class GridFieldServer(serverlib.SimpleXMLRPCServer):

  def _dispatch(self, method, params):
    try:
      func = getattr(self, 'export_' + method)
    except AttributeError:
      raise Exception('method %s is not supported' % method)
    else:
      return func(*params)
   
  def server_bind(self):
    self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    self.socket.bind(self.server_address)
 
  def __init__(self, address):
    serverlib.SimpleXMLRPCServer.__init__(self, address)
    # querystring -> query
    self.query_cache = {}

    # query -> filename
    self.disk_cache = {}

    # query -> GridField
    self.mem_cache = {}

    self.filecount = 0

    self.reqid = 0

  #def init(request):
  #  qry = algebra.queryFromDict(request)

  #  # check cache in case of prior 
  #  # initialization, optimization, compilation, etc.
  #  id = str(qry)
  #  try:
  #    Q = self.query_cache[id]
  #  except KeyError:
  #    qry.init()
  #    self.query_cache[id] = qry
  #    Q = qry

  #  return True

  def _nextfilename(self):
    self.filecount += 1
    return str(self.filecount) + ".gf"
  
  def serve_forever(self):
    self.quit = 0
    while not self.quit:
      self.handle_request()

  def export_kill(self):
    self.quit = 1
    #self.kill_frame_buffer()
    self.server_close()
    return 1
  
#  def export_bind(self, context, filter, size):
#    g = gridfield.ZeroGrid(size)
# need an array method that can interpret a string
#    a = gridfield.Array("addr", INT, filter, size)
#    gf = gridfield.GridField(g, 0, a)
#    bind = algebra.Bind(context, "value", gf, "addr")
#
#    answer = bind.getResult()
#    valattr = answer.getAttribute("value")
# and an array method that can return data as a string
#    s = valattr.getValsAsString()
#    return Binary(s)

  def export_generate_data_product(self, name, args={}, imgtype="gif"):
    Info("generating " + name + "...")
    f = file(name)

    # using unique request ids  
    # (server is synchronous, so no concurrency probs)
    outfn = str(self.reqid) + "_response." + imgtype
    self.reqid += 1

    gfrun.Run(f, args, outfn)
    rf = file(outfn)
    imgdata = rf.read()
    bin = xrl.Binary(imgdata)
    f.close()
    rf.close()
    os.remove(outfn)
    Info("returning image")
    return bin

  def export_execute(self, request):
    qry = algebra.queryFromDict(request)
    qry.map(algebra.attachclass)
    Info("Evaluating: %s" % (qry,))
    s = xrl.dumps((qry,))
    f = file(config.xmldump, "w")
    f.write(s)
    f.close()

    id = str(qry)
    try:
      fn = self.disk_cache[id]
    except KeyError:
      fn = self._nextfilename()
      save = algebra.Store(fn, qry)
      save.init()
      #save.execute()
      result = save.getResult()
      if result.IsAttribute(0, "salt"):
        ToVTK(save, "salt", capture=True)
      #result.output()
      self.disk_cache[id] = fn
    
    f = file(fn)
    data = f.read()
    f.close()
    bin = xrl.Binary(data)
    Info("Returning gridfield data")
    return bin

     
quit = 0
   
def main():
  try: 
    server = GridFieldServer((config.host, config.port))
    #server.register_introspection_functions()
    Info("Starting gridfield server...")
    server.serve_forever()
  except KeyboardInterrupt:
    server.export_kill()

if __name__ == "__main__":
  main()
