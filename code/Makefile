APP_NAME = graph_color

OBJS = main.o graph.o jp.o greedy.o speculative.o spec_jp.o

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -m64 -I. -fopenmp -Wno-unknown-pragmas

all: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp graph.h jp.h greedy.h speculative.h spec_jp.h

graph.o: graph.cpp graph.h
	$(CXX) $(CXXFLAGS) -c graph.cpp

jp.o: jp.cpp jp.h graph.h
	$(CXX) $(CXXFLAGS) -c jp.cpp

greedy.o: greedy.cpp greedy.h graph.h
	$(CXX) $(CXXFLAGS) -c greedy.cpp

speculative.o: speculative.cpp speculative.h graph.h
	$(CXX) $(CXXFLAGS) -c speculative.cpp

spec_jp.o: spec_jp.cpp spec_jp.h graph.h
	$(CXX) $(CXXFLAGS) -c spec_jp.cpp

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class
