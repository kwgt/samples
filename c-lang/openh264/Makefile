CFLAGS    += -O2 -g -I./include
LDFLAGS   += -O2 -g -L./lib -lopenh264

CSRC      := main.c \
             h264dec.c \
	     i420.c \
	     util.c
OBJS      := $(patsubst %.c,%.o, $(CSRC))
TARGET    := decoder

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

main.c: h264dec.h i420.h util.h
h264dec.c: h264dec.h
i420.c: i420.h
util.c: util.h

clean:
	rm -f $(TARGET) $(OBJS) ppm/*

clean-data:
	rm -f data/[0-9]*.264

decode: $(TARGET)
	env LD_LIBRARY_PATH=./lib ./$(TARGET)
