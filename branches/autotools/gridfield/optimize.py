   
import algebra as gf
import graph
import time
import os.path

def ReplaceContext(context, root):
  env = context.copy()
  env["var"] = a
  def f(B):
    if isinstance(B, gf.ZeroaryOperator):  # leaf op
      raise ValueError("Bind op for attribute %s not found." % (a,))
    if isinstance(B, gf.Bind):
      B.ReplaceContext(env)
    if isinstance(B, gf.Scan):
      B.ReplaceContext(env)
                                                                           
  root.map(f)
                                                                           
def ReplaceBindContext(context, attrs, root):
  env = context.copy()
  for a in attrs:
    env["var"] = a
    def f(B):
      if isinstance(B, gf.Bind) and B.attr == a:
        B.ReplaceContext(env)
                                                                           
    root.map(f)

def AllModified(root, since=time.time()):
  if root.Updated(since):
    yield root
  else:
    for c in root.Inputs():
      for x in AllModified(c, since):
        yield x

def typecheck(ops):
  pass
  # ensure input > required input
  # require each operator to reveal required input, and produced output

def optimize(ops):
  pass
  # Sharing
  # Applies/Aggregates mutate by default
  # add mutated attributes to "dirty_attrs" on each operator
  # add mutated cell arrays to "dirty_cells" on each operator
  # if Op mutates an attribute, insert a copy op if anyone else uses
  # it
# for each root, generate an id
# mark each operator in the tree with the root id

# for 
# 
