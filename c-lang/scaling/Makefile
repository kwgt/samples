
CFLAGS   += -I. -I../misc -Wall
#CFLAGS   += -O0 -g
CFLAGS   += -O3

LDFLAGS  += -lpng

TARGET   := aam_test

aam_test: aam_test.o aam.o png_file.o cronog.o

aam_test.o: aam_test.c aam.h png_file.h ../misc/cronog.h
aam.o: aam.c aam.h
png_file.o: png_file.c png_file.h
cronog.o: ../misc/cronog.c ../misc/cronog.h

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: ../misc/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%:
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o data/output*.*
	rm -f `find . -name ".DS_store" -or -name "._*"`
