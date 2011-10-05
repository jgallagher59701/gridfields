import gridfield.vis as vis
import gridfield.selfe as corie
import gridfield.core as gridfield
import gridfield.algebra as algebra
import os.path
import vtk
from time import time

import sys

def main():
  if len(sys.argv) < 2:
    print "Usage:"
    print "python gfrun <dataproduct.dp> [<output_filename.ext> = %s]\n    where '.ext' is one of %s" % (vis.DEFAULT_OUTPUT, vis.SUPPORTED_FORMATS)
    sys.exit(1)

  if len(sys.argv) < 3:
    outfn = vis.DEFAULT_OUTPUT
  else:
    outfn = sys.argv[2]
  
  dpfn = sys.argv[1]
  f = file(dpfn)
  script = f.read()

  body, env = BindArguments(script, {})

  start = time()
  GenerateDataProduct(body, env, outfn)

  print "Completed in ", time() - start, " seconds."

def BindArguments(script, new_arguments):
  parts = script.split("#BEGIN")
  if len(parts) > 2:
    raise ValueError("Parsing error: expected script of the form: \n " \
                     "(header, '#BEGIN', script body) or \n" \
                     "(script body)")
  if len(parts) == 1:
    body = parts[0]
    arguments = {}
  else:
    body = parts[1]
    exec parts[0]
    try:
      arguments.update(new_arguments)
    except NameError:
      raise NameError("The script header, if provided, must define an 'arguments' dictionary")
  return body, arguments


def GenerateDataProduct(body, args, outfn=None, renWin=None, screenlock=None):
  '''For offscreen rendering, set up a virtual frame buffer using Xvfb'''  

  #renWin = vtkLocal.vtkXOpenGLOffScreenRenderWindow()
  if renWin == None:
    renWin = vtk.vtkRenderWindow()
    try:
      w = args["width"]
      h = args["height"]
      renWin.SetSize(int(w), int(h))
    except KeyError:
      renWin.SetSize(500, 500)
      

  dpr = vis.DataProductRenderer(renWin, screenlock)    
 
  def dummy(a):
    pass
  env = {'AnimateScalar' : dpr.AnimateScalar, 
            'Draw' : dpr.Draw, 
            'Window' : dpr.Window, 
            'AddPane' : dpr.AddPane, 
            'ShowPlan' : dummy,
            '_vis' : dpr}
  env.update(algebra.__dict__)
  env.update(gridfield.__dict__)
  env.update({"arguments" : args})

  # Execute the data product as python code in a controlled environment
  exec body in env

  # save it as an image
  if outfn: 
    # Display the data product
    dpr.Render()
    # animations must be gifish
    (path, ext) = os.path.splitext(outfn)
    if ext == '.gif':
      dpr.MakeAnimation(path)  
    else:
      dpr.CaptureImage(outfn)


def Run(scriptfile, args={}, outfn=None, renWin=None):
 script = scriptfile.read()
 body, env = BindArguments(script, args)
 GenerateDataProduct(body, env, outfn, renWin)


if __name__ == "__main__":
  main()
