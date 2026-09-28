#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/CommonDialog.h>
#include <orbis/MsgDialog.h>
#include "config.h"
static char error[256];
static char config[65536],merged[66000],cleaned[65536];
static int show(const char *text) {
    if(sceMsgDialogInitialize()<0) return 0;
    OrbisMsgDialogParam param={}; OrbisMsgDialogUserMessageParam msg={};
    param.baseParam.size=sizeof(param.baseParam);
    param.baseParam.magic=(uint32_t)(ORBIS_COMMON_DIALOG_MAGIC_NUMBER+(uintptr_t)&param.baseParam);
    param.size=sizeof(param); param.mode=ORBIS_MSG_DIALOG_MODE_USER_MSG;
    msg.buttonType=ORBIS_MSG_DIALOG_BUTTON_TYPE_OK;
    msg.msg=text; param.userMsgParam=&msg;
    if(sceMsgDialogOpen(&param)<0) { sceMsgDialogTerminate(); return false; }
    uint64_t end=sceKernelGetProcessTime()+180000000ULL;
    while(sceMsgDialogUpdateStatus()!=ORBIS_COMMON_DIALOG_STATUS_FINISHED && sceKernelGetProcessTime()<end) sceKernelUsleep(20000);
    int selected=sceMsgDialogUpdateStatus()==ORBIS_COMMON_DIALOG_STATUS_FINISHED ? 1 : 0;
    sceMsgDialogClose(); sceMsgDialogTerminate(); return selected;
}
static bool fail(const char *step,int r) {
    snprintf(error,sizeof(error),"%s\nError: 0x%08x",step,(unsigned)r); return false;
}
static bool write_all(int fd,const void *data,size_t size) {
    const char *p=(const char*)data;
    while(size) { int r=sceKernelWrite(fd,p,size); if(r<=0) return fail("Write",r); p+=r; size-=r; }
    return true;
}
static bool write_file(const char *path,const void *data,size_t size,bool exclusive=false) {
    int fd=sceKernelOpen(path,exclusive ? 0xa01 : 0x601,0777);
    if(fd<0) return fail(path,fd);
    bool ok=write_all(fd,data,size);
    int sync=sceKernelFsync(fd),close=sceKernelClose(fd);
    if(!ok) return false;
    if(sync<0 || close<0) return fail("Synchronization",sync<0 ? sync : close);
    return true;
}
static bool backup(const char *path,const char *suffix) {
    int fd=sceKernelOpen(path,0,0);
    if((unsigned)fd==0x80020002) return true;
    if(fd<0) return fail(path,fd);
    char name[256]; snprintf(name,sizeof(name),"%s.procon-%s.bak",path,suffix);
    int out=sceKernelOpen(name,0xa01,0777);
    if(out<0) { sceKernelClose(fd); return fail(name,out); }
    char block[4096]; bool ok=true; int n;
    while((n=sceKernelRead(fd,block,sizeof(block)))>0) if(!write_all(out,block,n)) { ok=false; break; }
    if(n<0) ok=fail("Read for backup",n);
    int synced=sceKernelFsync(out); int closed=sceKernelClose(out); sceKernelClose(fd);
    return ok && (synced<0 ? fail("Backup sync",synced) : closed<0 ? fail("Backup close",closed) : true);
}
static bool remove_old_plugin() {
    const char *path="/data/GoldHEN/plugins/procon_usb.prx";
    char suffix[48];
    snprintf(suffix,sizeof(suffix),"retired-%llu",(unsigned long long)sceKernelGetProcessTime());
    if(!backup(path,suffix)) return false;
    int r=sceKernelUnlink(path);
    if(r<0 && (unsigned)r!=0x80020002) return fail("Delete old plugin (configuration already disabled)",r);
    return true;
}
static bool install(bool diagnostic) {
    const char *ini="/data/GoldHEN/plugins.ini",*prx="/data/GoldHEN/plugins/procon_loader_02.prx";
    int fd=sceKernelOpen(ini,0,0); size_t used=0;
    if(fd<0 && (unsigned)fd!=0x80020002) return fail("Open plugins.ini",fd);
    if(fd>=0) {
        int n=0;
        while(used<sizeof(config)-1 && (n=sceKernelRead(fd,config+used,sizeof(config)-1-used))>0) used+=n;
        char extra=0; int tail=n<0 ? n : sceKernelRead(fd,&extra,1); sceKernelClose(fd);
        if(tail!=0) return fail("plugins.ini too large or unreadable",tail);
    }
    config[used]=0;
    if(strlen(config)!=used) return fail("plugins.ini contains unexpected data",-1);
    int removed=remove_procon(config,cleaned,sizeof(cleaned));
    if(removed<0) return fail("Could not preserve plugins.ini",removed);
    if(diagnostic) {
        int result=merge_config(cleaned,merged,sizeof(merged));
        if(result) return fail("Ambiguous configuration; plugin not installed",result);
    } else {
        if(!removed) return true;
        memcpy(merged,cleaned,strlen(cleaned)+1);
    }
    int r=sceKernelMkdir("/data/GoldHEN",0777);
    if(r<0 && (unsigned)r!=0x80020011) return fail("Create GoldHEN",r);
    r=sceKernelMkdir("/data/GoldHEN/plugins",0777);
    if(r<0 && (unsigned)r!=0x80020011) return fail("Create plugins",r);
    char suffix[40]; snprintf(suffix,sizeof(suffix),"recovery-%llu",(unsigned long long)sceKernelGetProcessTime());
    if(!backup(ini,suffix)) return false;
    const char *tmpini="/data/GoldHEN/plugins.ini.procon-recovery-new";
    if(!diagnostic) {
        if(!write_file(tmpini,merged,strlen(merged))) return false;
        r=sceKernelRename(tmpini,ini); return r<0 ? fail("Disable ProCon",r) : true;
    }
    fd=sceKernelOpen("/app0/procon_loader_02.prx",0,0);
    if(fd<0) return fail("Open plugin in PKG",fd);
    static char binary[262144]; size_t bytes=0; int n=0;
    while(bytes<sizeof(binary) && (n=sceKernelRead(fd,binary+bytes,sizeof(binary)-bytes))>0) bytes+=n;
    sceKernelClose(fd);
    if(n<0 || bytes<32 || bytes==sizeof(binary)) return fail("Read plugin from PKG",n);
    if(!backup(prx,suffix)) return false;
    const char *tmpprx="/data/GoldHEN/plugins/procon_loader_02.prx.new";
    if(!write_file(tmpprx,binary,bytes) || !write_file(tmpini,merged,strlen(merged))) return false;
    r=sceKernelRename(tmpprx,prx); if(r<0) return fail("Install PRX",r);
    r=sceKernelRename(tmpini,ini); if(r<0) return fail("Enable configuration (PRX already copied)",r);
    return true;
}
static int run_installer() {
    if(sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG)<0 || sceCommonDialogInitialize()<0) return 1;
    if(show("ProCon 0.3\nEXPERIMENTAL USB INPUT SUPPORT\nmade by @nickyrmldy\n\nClose games and disable GoldHEN plugins while updating.\nPress OK and wait for installation confirmation.\nThen close this app and enable plugins in GoldHEN.\n\n> Keep the DualShock connected for ProCon to work")!=1) return 1;
    if(!install(false)) {
        char screen[600]; snprintf(screen,sizeof(screen),"Could not disable ProCon!\nDisable plugins in GoldHEN\n\n%s",error); show(screen); return 1;
    }
    if(!remove_old_plugin()) {
        char screen[600]; snprintf(screen,sizeof(screen),"The old plugin was disabled, but it could not be completely removed.\nNew plugin not installed.\n\n%s\nTake a photo of this screen.",error); show(screen); return 1;
    }
    if(install(true)) show("ProCon 0.3\nPlugin installed successfully.\n\n1. Close this app using the PS button.\n2. Enable plugins in GoldHEN.\n3. Connect supported USB input devices.\n4. Keep the DualShock connected and launch a game.\n\nNew input drivers are experimental and untested on PS4.\nThis app will remain open until you close it.");
    else { char screen[600]; snprintf(screen,sizeof(screen),"Plugin not installed\n%s",error); show(screen); }
    return 0;
}

int main() {
    run_installer();
    for(;;) sceKernelUsleep(1000000);
}
