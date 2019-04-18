#
#   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
#   CHECKED IT OUT USING THE COMMAND CHECKOUT.
#
#
CFLAGS = /usr/bin/gcc -D_INTEL -Wall -O3 -flto -g -I./include
LIBS = -lm -lpthread
LOCALMODULE = tb2sac.o compare.o sacproc.o progbar.o swap.o

main: $(LOCALMODULE)
	$(CFLAGS) -o tb2sac $(LOCALMODULE) $(LIBS)

.c.o:
	$(CFLAGS) -c $<

# Clean-up rules
clean:
	rm -f a.out core *.o *.obj *% *~;

clean_bin:
	rm -f tb2sac;
