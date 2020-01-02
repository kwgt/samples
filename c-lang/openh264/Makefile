CFLAGS    += -O2 -g -I./include -fPIC

#LDFLAGS   += -O2 -g -L./lib/aarch64-linux -lopenh264


CSRC      := main.c \
             h264dec.c \
	     i420.c \
	     util.c

# for darwin
CFLAGS    += -DENABLE_AVX -mavx -mavx2 -mfma
LDFLAGS   += -O3 -g -L./lib/x86_64-darwin18 -lopenh264
LDFLAGS   += -Wl,-rpath,lib/x86_64-darwin18
CSRC      += avx.c

OBJS      := $(patsubst %.c,%.o, $(CSRC))
TARGET    := decoder

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

main.c: h264dec.h i420.h util.h
h264dec.c: h264dec.h
i420.c: i420.h avx.h
util.c: util.h
avx.c: avx.h

clean:
	rm -f $(TARGET) $(OBJS) ppm/*

clean-data:
	rm -f data/[0-9]*.264

decode: $(TARGET)
	env LD_LIBRARY_PATH=./lib/aarch64-linux ./$(TARGET)
