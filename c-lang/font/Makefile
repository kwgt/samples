#CFLAGS  := -Wall -O0 -I. -g -DSHOW_BOUNDINGBOX
CFLAGS  := -Wall -O0 -I. -g 
LDFLAGS := -lpng

CSRC    := sample.c \
           utf8.c \
           font.c \
           png_file.c \
           font/mplus_10r.c \
           font/mplus_10b.c \
           font/mplus_12r.c \
           font/mplus_12b.c \
           font/shinonome12.c \
           font/shinonome12b.c \
           font/shinonome14.c \
           font/shinonome14b.c \
           font/shinonome16.c \
           font/shinonome16b.c \
           font/ayu18.c \
           font/ayu18b.c \
           font/ayu20.c \
           font/ayu20b.c \
           font/k8x12.c

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
font/shinonome12.o: font/shinonome12.c font.h
font/shinonome12b.o: font/shinonome12b.c font.h
font/shinonome14.o: font/shinonome14.c font.h
font/shinonome14b.o: font/shinonome14b.c font.h
font/shinonome16.o: font/shinonome16.c font.h
font/shinonome16b.o: font/shinonome16b.c font.h
font/ayu18.o: font/ayu18.c font.h
font/ayu18b.o: font/ayu18b.c font.h
font/ayu20.o: font/ayu20.c font.h
font/ayu20b.o: font/ayu20b.c font.h
font/k8x12.o: font/k8x12.c font.h

clean:
	rm -f sample $(OBJS)
