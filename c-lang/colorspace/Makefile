CFLAGS  += -g -O3
#CFLAGS  += -g

# armv8l
ifeq (,$(WITHOUT_NEON))
  CFLAGS  += -ftree-vectorize -DENABLE_NEON
endif

# armv7l
#CFLAGS  += -mneon -march=armv7-a -DENABLE_NEON

CFLAGS  += -fopenmp
LDFLAGS += -fopenmp

TARGET  := test1 test2 test3 test4

all: $(TARGET)

test1: test1.o yuv_to_rgb.o
test2: test2.o bgr_to_rgb.o
test3: test3.o bgr_to_rgb.o
test4: test4.o yuv_to_rgb.o yuv_to_rgb2.o

test.o: test.c
yuv_to_rgb.o: yuv_to_rgb.c
yuv_to_rgb2.o: yuv_to_rgb2.c
bgr_to_rgb.o: bgr_to_rgb.c

.c.s:
	$(CC) $(CFLAGS) -S -o $@ $<

clean:
	rm -f $(TARGET) *.o *.s *.ppm
