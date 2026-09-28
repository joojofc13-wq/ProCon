#include <assert.h>
#include <stdio.h>
#include "../src/config.h"
int main() {
    char out[2048],out2[2048];
    const char *src="[default]\r\n/keep.prx=true\r\n[CUSA14409]\r\n/data/GoldHEN/plugins/procon_usb.prx\r\n/another.prx=false\r\n";
    assert(remove_procon(src,out,sizeof(out))==1);
    assert(!strcmp(out,"[default]\r\n/keep.prx=true\r\n[CUSA14409]\r\n/another.prx=false\r\n"));
    assert(remove_procon(out,out2,sizeof(out2))==0 && !strcmp(out,out2));
    assert(merge_config(out,out2,sizeof(out2))==0);
    assert(strstr(out2,"[default]\r\n/data/GoldHEN/plugins/procon_loader_02.prx\n/keep.prx=true"));
    assert(strstr(out2,"[CUSA14409]\r\n/another.prx=false"));
    assert(remove_procon(out2,out,sizeof(out))==1);
    const char *mixed="[default]\n /data/GoldHEN/plugins/procon_usb.prx = true\n# /data/GoldHEN/plugins/procon_usb.prx\n/data/GoldHEN/plugins/procon_usb.prx.other\n\t/data/GoldHEN/plugins/procon_loader_02.prx=false";
    assert(remove_procon(mixed,out,sizeof(out))==2);
    assert(!strcmp(out,"[default]\n# /data/GoldHEN/plugins/procon_usb.prx\n/data/GoldHEN/plugins/procon_usb.prx.other\n"));
    assert(remove_procon("",out,sizeof(out))==0 && !out[0]);
    assert(remove_procon("untouched",out,2)==-1);
    assert(merge_config("[default]\n[default]\n",out,sizeof(out))==-2);
    assert(merge_config("",out,sizeof(out))==0);
    assert(!strcmp(out,"[default]\n/data/GoldHEN/plugins/procon_loader_02.prx\n"));
    assert(merge_config("[CUSA14409]\n/other.prx\n",out,sizeof(out))==0);
    assert(!strcmp(out,"[CUSA14409]\n/other.prx\n[default]\n/data/GoldHEN/plugins/procon_loader_02.prx\n"));
    assert(remove_procon(out,out2,sizeof(out2))==1);
    assert(merge_config(out2,out,sizeof(out))==0);
    assert(!strstr(strstr(out,"procon_loader_02.prx")+1,"procon_loader_02.prx"));
    assert(merge_config("[default]",out,sizeof(out))==0);
    assert(!strcmp(out,"[default]\n/data/GoldHEN/plugins/procon_loader_02.prx\n"));
    assert(merge_config("",out,2)==-1);
    puts("PASS: exact-key removal, optional enable flags, CRLF, comments, unrelated entries, repeated recovery, diagnostic section, capacity bounds");
}
