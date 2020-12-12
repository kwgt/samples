CFLAGS  := -O0 -I. -g -DSHOW_BOUNDINGBOX
LDFLAGS := -lpng

CSRC    := sample.c \
           utf8.c \
           font.c \
           png_file.c \
           font/mplus_10r.c \
           font/mplus_10b.c \
           font/mplus_12r.c \
           font/mplus_12b.c

OBJS    := $(CSRC:.c=.o)

test: $(OBJS) 
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f test $(OBJS)
