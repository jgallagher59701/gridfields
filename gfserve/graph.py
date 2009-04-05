import pydot

def makePyDotGraph(ops):
# this function insulates the algebra from pydot's
# calling conventions.

  G = pydot.Dot(graph_name='GF_Query', 
                graph_type='graph', 
                simplify=True,
                rankdir='UD')
  nodecount = 0
  
  def addEdge(n1, n2):
    G.add_edge(pydot.Edge(str(n1), str(n2)))
    
  def addNode(id, lbl):
    s = lbl.replace("\n", " ")
    n = pydot.Node(str(id), 
                   label=str(s), 
                   style='filled',
                   color="#E0E0E0")
    G.add_node(n)
    
  for op in ops:
    nodecount += 1
    op.mkGraph(addEdge, addNode)

  if nodecount == 0:
    G.add_node(pydot.Node("No query specified"))
    
  return G

def SavePlan(ops, prefix="out", w=800, h=600):
  G = makePyDotGraph(ops)
  G.set_size("%f,%f" % (float(w)/64,float(h)/80))
  G.write_ps("%s.eps" % (prefix,), prog="dot")
