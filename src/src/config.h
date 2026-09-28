#pragma once
#include <string.h>
#include <stddef.h>
static inline int merge_config(const char *src,char *dst,size_t capacity) {
    const char *entry="/data/GoldHEN/plugins/procon_loader_02.prx\n";
    if(strstr(src,"procon_loader_02.prx")) return 1;
    size_t n=strlen(src),insert=n; bool found=false;
    for(size_t start=0;start<n;) {
        size_t end=start; while(end<n && src[end]!='\n') ++end;
        size_t left=start,right=end;
        while(left<right && (src[left]==' ' || src[left]=='\t')) ++left;
        while(right>left && (src[right-1]==' ' || src[right-1]=='\r' || src[right-1]=='\t')) --right;
        if(right-left==9 && !memcmp(src+left,"[default]",9)) {
            if(found) return -2;
            found=true; insert=end<n ? end+1 : end;
        }
        start=end<n ? end+1 : end;
    }
    const char *section=found ? "" : "[default]\n";
    size_t sep=insert && src[insert-1]!='\n' ? 1 : 0;
    size_t extra=sep+strlen(section)+strlen(entry);
    if(n+extra+1>capacity) return -1;
    memcpy(dst,src,insert); size_t p=insert;
    if(sep) dst[p++]='\n';
    memcpy(dst+p,section,strlen(section)); p+=strlen(section);
    memcpy(dst+p,entry,strlen(entry)); p+=strlen(entry);
    memcpy(dst+p,src+insert,n-insert+1);
    return 0;
}
// src and dst must not overlap.
static inline int remove_procon(const char *src,char *dst,size_t capacity) {
    size_t n=strlen(src),written=0; int removed=0;
    for(size_t start=0;start<n;) {
        size_t end=start; while(end<n && src[end]!='\n') ++end;
        size_t next=end<n ? end+1 : end;
        size_t left=start; while(left<end && (src[left]==' ' || src[left]=='\t')) ++left;
        size_t key_end=left;
        while(key_end<end && src[key_end]!='=' && src[key_end]!=' ' && src[key_end]!='\t' && src[key_end]!='\r') ++key_end;
        const char *paths[]={"/data/GoldHEN/plugins/procon_usb.prx","/data/GoldHEN/plugins/procon_loader_02.prx"};
        bool match=false;
        for(unsigned i=0;i<2;++i) if(key_end-left==strlen(paths[i]) && !memcmp(src+left,paths[i],key_end-left)) match=true;
        if(match) ++removed;
        else {
            size_t len=next-start;
            if(written+len+1>capacity) return -1;
            memcpy(dst+written,src+start,len); written+=len;
        }
        start=next;
    }
    if(written+1>capacity) return -1;
    dst[written]=0; return removed;
}
