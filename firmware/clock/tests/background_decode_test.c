#include "background_decode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static bool cancel(void *context){unsigned *n=context;return ++*n<2;}
int main(int argc,char **argv)
{
    assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));
    long size=ftell(f);assert(size>0&&size<=BACKGROUND_JPEG_LIMIT);rewind(f);
    uint8_t *jpeg=malloc(size);assert(jpeg&&fread(jpeg,1,size,f)==(size_t)size);fclose(f);
    uint16_t *guarded=malloc((BACKGROUND_PIXELS+2)*sizeof(*guarded));assert(guarded);
    guarded[0]=0x1234;guarded[BACKGROUND_PIXELS+1]=0x4321;
    uint8_t scratch[8192];memset(guarded+1,0xff,BACKGROUND_PIXELS*2);
    assert(background_decode_jpeg(jpeg,size,guarded+1,scratch,sizeof(scratch),NULL,NULL));
    assert(guarded[0]==0x1234&&guarded[BACKGROUND_PIXELS+1]==0x4321);
    /* Fixture is solid red. Every output pixel must be covered, including edges. */
    for(unsigned i=1;i<=BACKGROUND_PIXELS;i++)assert((guarded[i]&0xf800)>=0xe800&&(guarded[i]&0x07ff)<0x80);
    unsigned calls=0;assert(!background_decode_jpeg(jpeg,size,guarded+1,scratch,sizeof(scratch),cancel,&calls));assert(calls==2);
    assert(!background_decode_jpeg(jpeg,size/2,guarded+1,scratch,sizeof(scratch),NULL,NULL));
    assert(!background_decode_jpeg(jpeg,size,guarded+1,scratch,512,NULL,NULL));
    memset(jpeg,0,size);assert(!background_decode_jpeg(jpeg,size,guarded+1,scratch,sizeof(scratch),NULL,NULL));
    assert(guarded[0]==0x1234&&guarded[BACKGROUND_PIXELS+1]==0x4321);
    free(jpeg);free(guarded);puts("PASS JPEG crop coverage, output guards, cancellation and malformed input");
}
