CFLAGS  := -Wall -O0 -I. -g -DSHOW_BOUNDINGBOX
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

sample: $(OBJS) 
	$(CC) -o $@ $^ $(LDFLAGS)

utf8.o : utf8.c utf8.h
font.o : font.c font.h
png_file.o : png_file.c
font/mplus_10r.o: font/mplus_10r.c font.h
font/mplus_10b.o: font/mplus_10b.c font.h
font/mplus_12r.o: font/mplus_12r.c font.h
font/mplus_12b.o: font/mplus_12b.c font.h

clean:
	rm -f sample $(OBJS)
