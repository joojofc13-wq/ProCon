import hashlib,pathlib,shutil,subprocess,zipfile
root=pathlib.Path(__file__).resolve().parent;out=root.parents[1]/'outputs';build=root/'build'
reader=root.parent/'llvm/llvm-mingw-20240619-ucrt-x86_64/bin/llvm-readelf.exe'
symbols=subprocess.check_output([str(reader),'--dyn-syms',str(build/'installer.elf')],text=True)
assert 'sceMsgDialogGetResult' not in symbols
assert not (build/'installer/procon_usb.prx').exists()
assert (build/'installer/procon_loader_02.prx').read_bytes()==(build/'procon_loader_02.prx').read_bytes()
files={'ProCon-0.3.pkg':build/'IV0000-PRCN00006_00-PROCONRELEASE001.pkg',
       'ProCon-0.3-LEIA-ME.md':root/'LEIA-ME.md'}
for name,source in files.items():shutil.copy2(source,out/name)
archive=out/'ProCon-0.3-codigo-revisado.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
    for f in sorted(root.rglob('*')):
        rel=f.relative_to(root)
        if f.is_file() and rel.parts[0] not in ('build','__pycache__') and rel.name not in ('diagnostic.c','requested-texts.md'):
            z.write(f,'procon-0.3/'+str(rel).replace('\\','/'))
files[archive.name]=archive
with zipfile.ZipFile(archive) as z:assert z.testzip() is None
(out/'ProCon-0.3.sha256').write_text(''.join(hashlib.sha256((out/name).read_bytes()).hexdigest()+'  '+name+'\n' for name in files),encoding='utf-8')
print('Verified:',', '.join(files))
