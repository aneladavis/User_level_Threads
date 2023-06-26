OBJECTS1 = producer-consumer1.o threads.o synchronization.o ll_double.o
OBJECTS2 = producer-consumer2.o threads.o synchronization.o ll_double.o

pc1-clean: clean pc1
pc2-clean: clean pc2

pc1: $(OBJECTS1)
	clang -g -o pc1 $(OBJECTS1) -I.

pc2: $(OBJECTS2)
	clang -g -o pc2 $(OBJECTS2) -I.

%.o: %.c
	clang -g -c -o $@ -I. -Inet $<

clean:
	rm -f *.o pc1 pc2
