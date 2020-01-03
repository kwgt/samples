CFLAGS    += -O2 -g -I./include -fPIC

#LDFLAGS   += -O2 -g -L./lib/aarch64-linux -lopenh264


CSRC      := main.c \
             h264dec.c \
	     i420.c \
	     util.c

# for MacOS
CFLAGS    += -DENABLE_AVX -mavx -mavx2 -mfma
LDFLAGS   += -L./lib/x86_64-darwin18 -lopenh264
LDFLAGS   += -Wl,-rpath,./lib/x86_64-darwin18
CFLAGS    += -I${HOME}/env/homebrew/include
CFLAGS    += -Xpreprocessor -fopenmp -DNUM_THREADS=4
LDFLAGS   += -L${HOME}/env/homebrew/lib -lomp
LDFLAGS   += -Wl,-rpath,${HOME}/env/homebrew/lib
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

update:
	install_name_tool \
	  -change "/usr/local/lib/libopenh264.5.dylib" \
	          "@rpath/libopenh264.5.dylib" \
	  decoder

decode: $(TARGET)
	env LD_LIBRARY_PATH=./lib/aarch64-linux ./$(TARGET)
