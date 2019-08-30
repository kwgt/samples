# armv8l
CFLAGS += -g -O3 -ftree-vectorize

# armv7l
#CFLAGS += -O0 -mneon -march=armv7-a

test: test.o yuv_to_rgb.o

test.o: test.c
yuv_to_rgb.o: yuv_to_rgb.c

.c.s:
	$(CC) $(CFLAGS) -S -o $@ $<

clean:
	rm -f test *.o *.s *.ppm
