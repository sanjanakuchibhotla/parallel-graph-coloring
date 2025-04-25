APP_NAME = graph_color

OBJS = main.o graph.o jp.o sequential.o  # updated to use sequential.o

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -m64 -I. -fopenmp -Wno-unknown-pragmas

all: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp graph.h jp.h sequential.h  # updated header dependency

graph.o: graph.cpp graph.h
	$(CXX) $(CXXFLAGS) -c graph.cpp

jp.o: jp.cpp jp.h graph.h
	$(CXX) $(CXXFLAGS) -c jp.cpp

sequential.o: sequential.cpp sequential.h graph.h
	$(CXX) $(CXXFLAGS) -c sequential.cpp

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class
