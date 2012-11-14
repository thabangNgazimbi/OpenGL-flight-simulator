SRCS = flight_sim.c readppm.c glm.c readoff.c

ifeq ($(shell uname),Darwin)
	TARGET = flightsim_macosx
else
	TARGET = flightsim_linux
endif

LN = ln -s
MV = mv
RM = -rm -rf

CC = cc


ifeq ($(shell uname),Darwin)
	LD_LIBS = -framework GLUT -framework OpenGL -lobjc
else
	LD_LIBS =  -lglut -lGLU -lGL
endif


CFLAGS =  -c

OBJS = $(SRCS:.c=.o)

.c.o:
	($(CC) $(CFLAGS)  $<)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)  $(LD_LIBS)

all     : $(TARGET) 

default : $(all)

clean:
	-$(RM) $(OBJS) $(TARGET) *~
