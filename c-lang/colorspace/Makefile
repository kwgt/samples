CFLAGS  += -g -O3

# armv8l
CFLAGS  += -ftree-vectorize -DENABLE_NEON

# armv7l
#CFLAGS  += -mneon -march=armv7-a -DENABLE_NEON

CFLAGS  += -fopenmp
LDFLAGS += -fopenmp

test: test.o yuv_to_rgb.o

test.o: test.c
yuv_to_rgb.o: yuv_to_rgb.c

.c.s:
	$(CC) $(CFLAGS) -S -o $@ $<

clean:
	rm -f test *.o *.s *.ppm
