CC=clang
CFLAGS=-c -g -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function
INCLUDES=-I./Includes -I../Utils/Includes -I../DataTypes/Includes -I../Database/Includes
AR=ar
ARFLAGS=-cr
OFILES=gedcom.o gnode.o lineage.o name.o nodeutls.o readnode.o splitjoin.o writenode.o place.o date.o gnodelist.o gnodeindex.o gedpath.o rootlist.o
LIBNAME=gedcom

lib$(LIBNAME).a: $(OFILES)
	$(AR) $(ARFLAGS) lib$(LIBNAME).a $(OFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $<

clean:
	rm -f *.o *.a
