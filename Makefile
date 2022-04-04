all: main html/index.html latex/refman.pdf

hfiles=circ.h color.h graobj.h rect.h

main.o: main.cpp $(hfiles)
  @g++ -c main.cpp