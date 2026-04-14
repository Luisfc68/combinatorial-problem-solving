CXX = g++ -std=c++11
INCLUDE_DIR = /usr/include
LIB_DIR = /usr/lib/x86_64-linux-gnu/
LIBS = -lgecodedriver    -lgecodesearch  \
       -lgecodeminimodel -lgecodeint     \
       -lgecodekernel    -lgecodesupport
FILE=StreetDirectionalityProblem
CHECKER=checker

all: build checker

build: $(FILE).cpp
	$(CXX) -I$(INCLUDE_DIR) -c $(FILE).cpp
	$(CXX) -L$(LIB_DIR) -o sdp $(FILE).o $(LIBS)

checker: $(CHECKER).cpp
	$(CXX) -o sdp-checker $(CHECKER).cpp
