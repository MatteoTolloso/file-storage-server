
SHELL           = /bin/bash
CC		=  gcc
AR              =  ar
CFLAGS	        +=  -std=c99 -Wall -Werror -g
ARFLAGS         =  rvs
INCLUDES	= -I. -I ./includes
LDFLAGS 	= -L.
OPTFLAGS	= -O3 
LIBS            = -lpthread

# aggiungere qui altri targets
TARGETS		= 	toup_server  \
		  toup_client  \
		  

.PHONY: all clean cleanall testtoup
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

clean		: 
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~ *.a ./cs_sock ./toup?.txt
