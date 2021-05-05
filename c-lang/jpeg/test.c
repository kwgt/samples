#include "util.h"
#include "jpeg.h"

int
main(int argc, char* argv[])
{
  int err;

  jpeg_decoder_t* dec;
  jpeg_encoder_t* enc;

  void* jpg1;
  size_t size;

  void* raw;
  size_t wd;
  size_t ht;

  void* jpg2;

  FILE* fp;

  dec  = NULL;
  enc  = NULL;
  raw  = NULL;
  jpg1 = NULL;
  jpg2 = NULL;
  fp   = NULL;

  do {
    err = read_file("DSC_0215_small.JPG", &jpg1, &size);
    if (err) {
      fprintf(stderr, "read_file() failed [err=%d]\n", err);
      break;
    }

    err = jpeg_decoder_new(&dec);
    if (err) {
      fprintf(stderr, "jpeg_decoder_new() failed [err=%d]\n", err);
      break;
    }

    err = jpeg_decoder_decode(dec, jpg1, size, &raw, &wd, &ht);
    if (err) {
      fprintf(stderr, "jpeg_decoder_decode() failed [err=%d]\n", err);
      break;
    }


    write_ppm("output.ppm", wd, ht, raw, NULL);

    err = jpeg_encoder_new(wd, ht, 0, &enc);
    if (err) {
      fprintf(stderr, "jpeg_encoder_new() failed [err=%d]\n", err);
      break;
    }

    err = jpeg_encoder_encode(enc, raw, &jpg2, &size);
    if (err) {
      fprintf(stderr, "jpeg_encoder_encode() failed [err=%d]\n", err);
      break;
    }

    fp = fopen("output.jpg", "wb");
    if (fp == NULL) {
      fprintf(stderr, "fopen() failed [err=%d]\n", err);
      break;
    }

    fwrite(jpg2, size, 1, fp);
  } while (0);

  if (dec != NULL) jpeg_decoder_destroy(dec);
  if (enc != NULL) jpeg_encoder_destroy(enc);
  if (raw != NULL) free(raw);
  if (jpg1 != NULL) free(jpg1);
  if (jpg2 != NULL) free(jpg2);
  if (fp != NULL) fclose(fp);

  return 0;
}
