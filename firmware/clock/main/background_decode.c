#include "background_decode.h"
#include "background_jpeg.h"
#include <string.h>
typedef struct {
    const uint8_t *data;size_t size,position;
    uint16_t *pixels;
    unsigned crop_x,crop_y,crop_width,crop_height;
    bool (*proceed)(void *);void *context;
} decode_t;
static size_t read_jpeg(JDEC *decoder,uint8_t *buffer,size_t count)
{
    decode_t *s=decoder->device;
    size_t available=s->size-s->position;if(count>available)count=available;
    if(buffer)memcpy(buffer,s->data+s->position,count);
    s->position+=count;return count;
}
static unsigned ceiling_div(unsigned a,unsigned b){return (a+b-1)/b;}
static int write_tile(JDEC *decoder,void *bitmap,JRECT *r)
{
    decode_t *s=decoder->device;if(s->proceed&&!s->proceed(s->context))return 0;
    unsigned left=r->left>s->crop_x?r->left:s->crop_x;
    unsigned top=r->top>s->crop_y?r->top:s->crop_y;
    unsigned right=(unsigned)r->right+1,bottom=(unsigned)r->bottom+1;
    if(right>s->crop_x+s->crop_width)right=s->crop_x+s->crop_width;
    if(bottom>s->crop_y+s->crop_height)bottom=s->crop_y+s->crop_height;
    if(left>=right||top>=bottom)return 1;
    unsigned x0=ceiling_div((left-s->crop_x)*BACKGROUND_WIDTH,s->crop_width);
    unsigned x1=ceiling_div((right-s->crop_x)*BACKGROUND_WIDTH,s->crop_width);
    unsigned y0=ceiling_div((top-s->crop_y)*BACKGROUND_HEIGHT,s->crop_height);
    unsigned y1=ceiling_div((bottom-s->crop_y)*BACKGROUND_HEIGHT,s->crop_height);
    unsigned stride=r->right-r->left+1;
    /* LVGL 9.4 vendor decoder emits B,G,R bytes in JD_FORMAT=0. */
    const uint8_t *rgb=bitmap;
    for(unsigned y=y0;y<y1;y++){
        unsigned sy=s->crop_y+y*s->crop_height/BACKGROUND_HEIGHT;
        for(unsigned x=x0;x<x1;x++){
            unsigned sx=s->crop_x+x*s->crop_width/BACKGROUND_WIDTH;
            const uint8_t *p=rgb+((sy-r->top)*stride+sx-r->left)*3;
            s->pixels[y*BACKGROUND_WIDTH+x]=(uint16_t)(((p[2]>>3)<<11)|((p[1]>>2)<<5)|(p[0]>>3));
        }
    }
    return 1;
}
bool background_decode_jpeg(const uint8_t *data,size_t size,uint16_t *destination,
                            void *scratch,size_t scratch_size,bool (*proceed)(void *),void *context)
{
    if(!data||size<4||size>BACKGROUND_JPEG_LIMIT||!destination||!scratch||scratch_size<8192||
       data[0]!=0xff||data[1]!=0xd8)return false;
    decode_t state={.data=data,.size=size,.pixels=destination,.proceed=proceed,.context=context};
    JDEC decoder;
    if(jd_prepare(&decoder,read_jpeg,scratch,scratch_size,&state)!=JDR_OK)return false;
    if(!decoder.width||!decoder.height||decoder.width>4096||decoder.height>4096)return false;
    state.crop_width=decoder.width;state.crop_height=decoder.height;
    if((unsigned)decoder.width*BACKGROUND_HEIGHT>(unsigned)decoder.height*BACKGROUND_WIDTH)
        state.crop_width=(unsigned)decoder.height*BACKGROUND_WIDTH/BACKGROUND_HEIGHT;
    else state.crop_height=(unsigned)decoder.width*BACKGROUND_HEIGHT/BACKGROUND_WIDTH;
    if(!state.crop_width||!state.crop_height)return false;
    state.crop_x=(decoder.width-state.crop_width)/2;state.crop_y=(decoder.height-state.crop_height)/2;
    return jd_decomp(&decoder,write_tile,0)==JDR_OK;
}
