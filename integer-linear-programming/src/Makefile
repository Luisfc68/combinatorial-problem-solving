
all: build

FILE=StreetDirectionalityProblem
CHECKER=checker

all: build checker

build:
	$(CCC) -c $(CCFLAGS) $(FILE).cpp -o $(FILE).o
	$(CCC) $(CCFLAGS) $(FILE).o -o sdp $(CCLNFLAGS)
	rm $(FILE).o

checker: $(CHECKER).cpp
	$(CCC) -o sdp-checker $(CHECKER).cpp

################################

SYSTEM     	  = x86-64_linux
LIBFORMAT     = static_pic
CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio201/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio201/concert

################

CCC = g++

CCOPT = -std=c++11 -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread -ldl

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 