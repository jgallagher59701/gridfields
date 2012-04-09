import os.path as op
import os
import ez_setup
import sys
ez_setup.use_setuptools()
#!/usr/bin/env python

from setuptools import setup, find_packages, Extension
#from distutils.core import setup, Extension

b = op.abspath(op.join('.','clib/src'))
gfdir = op.abspath(op.join('.','gridfield'))
allsources = os.listdir(b)


# c extension for accessing SELFE files
elio = [op.join(b,c) for c in allsources if op.splitext(c)[1]  in ['.c']]
elio = Extension('elio',
                  elio, 
                  include_dirs = [b],
                  )

# Ugh, does distutils provide an easy wa yto derive this 
# location from the environment a la configure?
vtkincl = '/usr/include/vtk-5.2'
#vb = op.abspath(op.join('.','src/vis'))

swig_opts = ['-c++', '-I%s' % b, '-I%s' % vtkincl, '-classic']

# c++ gridfield core extension
csources = [c for c in allsources if op.splitext(c)[1]  in ['.cc', '.cpp', '.c++', '.c']]
csources = [op.join(b,c) for c in csources]
sources = [op.join(gfdir,'core.i')] + csources
pygridfield = Extension('_core',
                       sources, 
                       include_dirs = [b],
                       libraries = ['netcdf_c++', 'netcdf'],
                       swig_opts = swig_opts
                       )

ext_modules = [elio, pygridfield]

#try:
  # only build vtk if the vtk modules are installed
#  import vtk

  # vtk-based visualization
  # not sure how to avoid recompiling and relinking the gridfield objs
  # Very wasteful, but I'm having trouble instructing python
  # to use the symbols from _gridfield.so
#  gfsources = [c for c in csources if 'output' not in c and 'netcdf' not in c and 'stuebe' not in c]
#  vsources = [op.join('gridfield','gfvis.i'), 
#              op.join(vb,'vtkGridField.cxx')] + gfsources
#  vtkgridfield = Extension('_gfvis',
#                         vsources,
#                         include_dirs = [vtkincl, vb, b],
#                         libraries = ['vtkFiltering', 'vtkCommon','vtkRendering', 'vtkIO', 'vtkCommonPythonD'],
#                         swig_opts = swig_opts
#                         )
#  ext_modules = [vtkgridfield] + ext_modules

#except ImportError:
#  print "VTK or the VTK python bindings do not appear to be installed....skipping the vis module"
#  pass

# Workaround for SWIG/C++ bug
# Specifying "-c++" above should be enough to get .cpp SWIG output, but it isn't
# http://mail.python.org/pipermail/distutils-sig/2005-November/005387.html
custom_opts = {
  'build_ext' : {
     'swig_opts':' '.join(swig_opts),
     'debug':True
  }
}
  
# Run the setup function
setup (name = 'gridfield',
       version = '0.5',
       author      = "Bill Howe",
       description = """Convenient Manipulation of Unstructured Grids""",
       ext_modules = ext_modules,
       packages = find_packages(),
       options = custom_opts
       )
