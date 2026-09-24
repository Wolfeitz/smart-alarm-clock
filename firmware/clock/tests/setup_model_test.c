#include "setup_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    setup_request_t r;
    const char *s="{\"tag\":42,\"url\":\"http://192.168.1.232:8123\",\"token\":\"synthetic-token\",\"light\":\"\"}";
    assert(setup_parse(s,strlen(s),&r)&&r.tag==42&&!r.entity[0]&&!strcmp(r.token,"synthetic-token"));
    assert(!setup_parse(s,strlen(s)-1,&r)&&!r.token[0]);
    const char *bad[]={"{}","[]","{\"tag\":1.5}","{\"tag\":0}","{\"tag\":4294967296}","{\"tag\":42,\"url\":\"http://host\",\"token\":\"valid\\u0000hidden\",\"light\":\"\"}","{\"tag\":42,\"url\":\"http://host/path\",\"token\":\"synthetic\",\"light\":\"\"}"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!setup_parse(bad[i],strlen(bad[i]),&r)&&!r.token[0]);
    char large[1801];memset(large,'x',sizeof(large));assert(!setup_parse(large,sizeof(large),&r));
    puts("PASS bounded credential setup parser, tags, truncation, endpoint and decoded-NUL rejection");
}
