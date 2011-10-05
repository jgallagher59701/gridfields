

# ========= Changes unlikely to be needed below this point =============

IWRAP      = $(INTERFACE:.i=_wrap.i)
ICXXSRCS   = $(IWRAP:.i=.cxx)
IOBJS      = $(IWRAP:.i=.o)

MODULE = $(basename $(INTERFACE))

# object lists
CXXBASES     = $(basename $(CXXSRCS))
CBASES     = $(basename $(CSRCS))

ifdef CXXSRCS
  CXXOBJS  = $(addsuffix .o, $(CXXBASES))
  CXXOBJ_RULE  = cxxobjects
else
  CXXOBJ_RULE  = 
endif

ifdef CSRCS
  COBJS  = $(addsuffix .o, $(CBASES))
  COBJ_RULE  = cobjects
else
  COBJ_RULE  = 
endif
ALLOBJS = $(CXXOBJS) $(COBJS) $(OBJS)
POBJS  = $(addprefix $(OBJPATH), $(ALLOBJS))

CBASES     = $(basename $(CSRCS))
COBJS      = $(addsuffix .o, $(CBASES))

# Language specific vars
ifeq ($(LANGUAGE), python)
  LANGUAGE_INCLUDE= -DHAVE_CONFIG_H -I$(PYTHON_INCL) -I$(PYTHON_CONF)
endif

# rules
all: sharedlibs

wrappers: $(INTERFACE)
	$(SWIG) -classic -Wall -c++ -$(LANGUAGE) $(CINCLUDES) $(CXXINCLUDES) $(SWIGOPT) $(INTERFACE)

cobjects: $(CSRCS)
	$(CC) -c $(CCSHARED) $(CFLAGS) $(CSRCS) $(CINCLUDES) $(LANGUAGE_INCLUDE)

wrapperobj:
	$(CXX) -c $(CFLAGS) $(ICXXSRCS) $(CXXINCLUDES) $(LANGUAGE_INCLUDE)

cxxobjects: $(CXXSRCS)
	$(CXX) -c -O3 $(CCSHARED) $(CFLAGS) $(ICXXSRCS) $(CXXSRCS) $(CXXINCLUDES) $(LANGUAGE_INCLUDE)

sharedlibs: wrappers wrapperobj $(COBJ_RULE) $(CXXOBJ_RULE)
	$(CXXSHARED) $(POBJS) $(IOBJS) $(PYTHON_DLNK) $(CXXLIBS) -o _$(MODULE).so

clean: 
	rm -f *.o *.so *_wrap* *~ .~* core  mypython *.pyc $(MODULE).py
