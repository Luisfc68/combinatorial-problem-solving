CXX = g++ -std=c++11
INCLUDE_DIR = /usr/include
FILE=StreetDirectionalityProblem
CHECKER=checker

all: build checker

build: $(FILE).cpp
	$(CXX) -I$(INCLUDE_DIR) -c $(FILE).cpp
	$(CXX) -o sdp $(FILE).o $(LIBS)

checker: $(CHECKER).cpp
	$(CXX) -o sdp-checker $(CHECKER).cpp
