all: libopenssh.a

libopenssh.a: atomicio.o bsd-misc.o fatal.o log.o misc.o strlcat.o strlcpy.o strtonum.o vis.o xmalloc.o
	ar -rcs $@ $^

clean:
	rm -f *.a *.o 
