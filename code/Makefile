APP_NAME = jp_exec

OBJS = main.o graph.o jp.o

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -m64 -I. -fopenmp -Wno-unknown-pragmas

all: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp graph.h jp.h
	$(CXX) $(CXXFLAGS) -c main.cpp

graph.o: graph.cpp graph.h
	$(CXX) $(CXXFLAGS) -c graph.cpp

jp.o: jp.cpp jp.h graph.h
	$(CXX) $(CXXFLAGS) -c jp.cpp

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class
