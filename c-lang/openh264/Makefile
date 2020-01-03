CFLAGS    += -O2 -g -I./include -fPIC
#CFLAGS    += -DBENCHMARK

# for NEON
CFLAGS    += -DENABLE_NEON

# for OpenH264
LDFLAGS   += -L./lib/aarch64-linux -lopenh264
LDFLAGS   += -Wl,-rpath,./lib/aarch64-linux

# for OpenMP
#CFLAGS    += -fopenmp
#LDFLAGS   += -lgomp

CSRC      := main.c \
             h264dec.c \
	     i420.c \
	     util.c

OBJS      := $(patsubst %.c,%.o, $(CSRC))
TARGET    := decoder

all: $(TARGET)

main.c: h264dec.h i420.h util.h
h264dec.c: h264dec.h
i420.c: i420.h
util.c: util.h

decoder: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS) ppm/*

clean-data:
	rm -f data/[0-9]*.264

decode: $(TARGET)
	env LD_LIBRARY_PATH=./lib/aarch64-linux ./$(TARGET)
