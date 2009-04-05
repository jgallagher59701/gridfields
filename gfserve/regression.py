import gridfield as gf
import vis
import sys
import vtk
import corielib as corie
from parameters import *

def zoom(bounds, H):
  zoom = gf.Restrict("(%s<x) & (x<%s) & (%s<y) & (y<%s)" % bounds, 0, H)
  return zoom


def TestV3():
  W = corie.WetGridV3(conV3, mouthbox, "salt")
  return (W, "salt")

def TestV4():
  W = corie.WetGridV4(conV4, mouthbox, 'salt')
  return W, "salt"

def TestV5():
  W = corie.WetGridV5(fraser, mouthbox, 'salt')
  return W, "salt"
  
def main():
  W, v = TestV4()
  W.getResult()
  W.GetAttribute(v).show()
  vis.Inspect(W, v)

if __name__=='__main__':
  main()


