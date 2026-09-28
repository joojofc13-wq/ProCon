import argparse,os,pathlib,subprocess
p=argparse.ArgumentParser();p.add_argument('--sdk',required=True);p.add_argument('--llvm',required=True)
a=p.parse_args();root=pathlib.Path(__file__).resolve().parent
sdk=pathlib.Path(a.sdk).resolve();llvm=pathlib.Path(a.llvm).resolve()
build=root/'build';build.mkdir(exist_ok=True)
env=dict(os.environ,OO_PS4_TOOLCHAIN=str(sdk))
def run(*args): subprocess.run([str(x) for x in args],cwd=root,env=env,check=True)
run(llvm/'clang++.exe','-Wall','-Wextra','-Werror',root/'tests/config_test.cpp','-o',build/'config_test.exe')
run(build/'config_test.exe')
run(llvm/'clang++.exe',root/'tests/usb_command_test.cpp','-o',build/'usb_test.exe')
run(build/'usb_test.exe')
run(llvm/'clang++.exe','-static','-O2','-Wall','-Wextra','-Werror',root/'tests/bridge_test.cpp','-o',build/'bridge_test.exe')
run(build/'bridge_test.exe')
run(llvm/'clang++.exe','-static','-O2','-Wall','-Wextra','-Werror',root/'tests/jump_test.cpp','-o',build/'jump_test.exe')
run(build/'jump_test.exe')
run(llvm/'clang++.exe','-static','-O2','-Wall','-Wextra','-Werror',root/'tests/controllers_test.cpp','-o',build/'controllers_test.exe')
run(build/'controllers_test.exe')
run(llvm/'clang++.exe','-static','-O2','-Wall','-Wextra','-Werror',root/'tests/keyboard_mouse_test.cpp','-o',build/'keyboard_mouse_test.exe')
run(build/'keyboard_mouse_test.exe')
objects=[]
for name in ('src/usb_test.cpp','vendor/crtprx.c'):
    src=root/name;obj=build/(src.stem+'.o');objects.append(obj)
    extra=['-fno-exceptions','-fno-rtti'] if src.suffix=='.cpp' else []
    run(llvm/('clang++.exe' if src.suffix=='.cpp' else 'clang.exe'),*extra,'--target=x86_64-pc-freebsd12-elf','-fPIC','-fno-builtin','-fno-stack-protector',
        '-nostdinc','-isystem',sdk/'include','-isystem',sdk/'include/orbis/_types','-I'+str(root/'vendor'),
        '-Wall','-Wextra','-Werror','-O1','-c',src,'-o',obj)
exports=['plugin_load','plugin_unload','g_pluginName','g_pluginDesc','g_pluginAuth','g_pluginVersion']
run(llvm/'ld.lld.exe',*objects,'-o',build/'diagnostic.elf','-m','elf_x86_64','-pie',
    '--script',sdk/'link.x','-e','_init','--eh-frame-hdr',
    *['--export-dynamic-symbol='+name for name in exports],'-L'+str(sdk/'lib'),'-lkernel')
symbols=subprocess.check_output([str(llvm/'llvm-readelf.exe'),'--dyn-syms',str(build/'diagnostic.elf')],text=True)
imports=[line.split()[-1] for line in symbols.splitlines() if ' UND ' in line and len(line.split())>7]
allowed={'sceKernelSendNotificationRequest','sceKernelGetFsSandboxRandomWord','sceKernelLoadStartModule','sceKernelDlsym','sceKernelGetProcessTime','sceKernelUsleep','scePthreadCreate','sceKernelMprotect'}
assert set(imports)==allowed,imports
for name in exports: assert any(line.endswith(' '+name) and ' UND ' not in line for line in symbols.splitlines()),name
(build/'imports-verified.txt').write_text(symbols,encoding='utf-8')
run(sdk/'bin/windows/create-fself.exe','-in='+str(build/'diagnostic.elf'),
    '-out='+str(build/'diagnostic.oelf'),'--lib='+str(build/'procon_loader_02.prx'),'--paid','0x3800000000000011')
print('Diagnostic built: kernel-only imports and required plugin exports verified.')
