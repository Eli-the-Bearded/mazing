
CFLAGS = -g

gamemazes: ldmazer etbmazer

textmazes: binary_tree sidewinder aldousbroder

test: testgrid testdistance
	./testgrid
	./testdistance
	@echo
	@echo ALL TESTS SUCCEEDED

ldmazer: ldmazer.o
	cc -g -std=c99 -I/usr/include/SDL2 -Wall -Wextra -Wno-unused-value -o $@ $^ -lSDL2 -lSDL2_image

etbmazer: etbmazer.o grid.o distance.o mazes.o
	cc -g -std=c99 -I/usr/include/SDL2 -Wall -Wextra -Wno-unused-value -o $@ $^ -lSDL2 -lSDL2_image

ldmazer.o: ldmazer.c
	cc -g -std=c99 -I/usr/include/SDL2 -Wall -Wextra -Wno-unused-value -c -o $@ $^

etbmazer.o: etbmazer.c
	cc -g -std=c99 -I/usr/include/SDL2 -Wall -Wextra -Wno-unused-value -c -o $@ $^
clean:
	rm -rf *.o testgrid testdistance core

testgrid: testgrid.o grid.o
testdistance: testdistance.o distance.o grid.o mazes.o
binary_tree: binary_tree.o grid.o mazes.o
sidewinder: sidewinder.o grid.o mazes.o
aldousbroder: aldousbroder.o distance.o grid.o mazes.o

mazes.o: distance.h grid.h mazes.h
testgrid.o: grid.h
testdistance.o: distance.h grid.h mazes.h
binary_tree.o: grid.h mazes.h
sidewinder.o: grid.h mazes.h
grid.o: grid.h mazes.h

