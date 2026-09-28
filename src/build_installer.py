"""Package the plugin and installer."""
import argparse,os,pathlib,shutil,subprocess
p=argparse.ArgumentParser(); p.add_argument('--sdk',required=True);p.add_argument('--llvm',required=True)
a=p.parse_args(); root=pathlib.Path(__file__).resolve().parent
sdk=pathlib.Path(a.sdk).resolve(); llvm=pathlib.Path(a.llvm).resolve()
build=root/'build'; package=build/'installer'; package.mkdir(parents=True,exist_ok=True)
env=dict(os.environ,OO_PS4_TOOLCHAIN=str(sdk),DOTNET_ROLL_FORWARD='Major')
def run(*args,cwd=root): subprocess.run([str(x) for x in args],cwd=cwd,env=env,check=True)
run(llvm/'clang++.exe','--target=x86_64-pc-freebsd12-elf','-fPIC','-funwind-tables','-fno-exceptions',
    '-fno-rtti','-nostdinc','-isystem',sdk/'include','-isystem',sdk/'include/orbis/_types',
    '-Wall','-Wextra','-Werror','-O0','-c',root/'src/installer.cpp','-o',build/'installer.o')
run(llvm/'ld.lld.exe',build/'installer.o','-o',build/'installer.elf','-m','elf_x86_64','-pie',
    '--script',sdk/'link.x','--eh-frame-hdr','-L'+str(sdk/'lib'),'-lc','-lkernel',
    '-lSceSysmodule','-lSceCommonDialog','-lSceMsgDialog',sdk/'lib/crt1.o')
binpath=sdk/'bin/windows'
run(binpath/'create-fself.exe','-in='+str(build/'installer.elf'),'-out='+str(build/'installer.oelf'),
    '--eboot',package/'eboot.bin','--paid','0x3800000000000011')
for directory in ('sce_sys/about','sce_module'): (package/directory).mkdir(parents=True,exist_ok=True)
for rel in ('sce_sys/about/right.sprx','sce_module/libc.prx','sce_module/libSceFios2.prx'):
    shutil.copy2(sdk/'samples/dialogs'/rel,package/rel)
shutil.copy2(root/'icon0.png',package/'sce_sys/icon0.png')
shutil.copy2(build/'procon_loader_02.prx',package/'procon_loader_02.prx')
tool=binpath/'PkgTool.Core.exe';sfo=package/'sce_sys/param.sfo'; content='IV0000-PRCN00006_00-PROCONRELEASE001'
run(tool,'sfo_new',sfo)
entries={'APP_TYPE':('Integer',4,'1'),'APP_VER':('Utf8',8,'00.03'),'ATTRIBUTE':('Integer',4,'0'),
    'CATEGORY':('Utf8',4,'gd'),'CONTENT_ID':('Utf8',48,content),'DOWNLOAD_DATA_SIZE':('Integer',4,'0'),
    'SYSTEM_VER':('Integer',4,'0'),'TITLE':('Utf8',128,'ProCon'),
    'TITLE_ID':('Utf8',12,'PRCN00006'),'VERSION':('Utf8',8,'00.03')}
for key,(kind,size,value) in entries.items(): run(tool,'sfo_setentry',sfo,key,'--type',kind,'--maxsize',size,'--value',value)
files=sorted(str(f.relative_to(package)).replace('\\','/') for f in package.rglob('*') if f.is_file() and f.suffix!='.gp4')
run(binpath/'create-gp4.exe','-out','installer.gp4','--content-id='+content,'--files',' '.join(files),cwd=package)
run(tool,'pkg_build','installer.gp4',str(build),cwd=package)
run(tool,'pkg_validate',build/(content+'.pkg'))
print('Installer package validated:',build/(content+'.pkg'))
