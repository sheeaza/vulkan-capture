let SessionLoad = 1
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd /opt/samba/nxf47746/project/learn-test/vulkan-texture/src
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +1 main.cpp
badd +737 render.cpp
badd +2 render.hpp
badd +1 /opt/samba/nxf47746/project/sdk/yocto/4.14-ga-wayland/sysroots/aarch64-poky-linux/usr/include/vulkan/vulkan.h
badd +558 /opt/samba/nxf47746/project/sdk/yocto/4.14-ga-wayland/sysroots/aarch64-poky-linux/usr/include/vulkan/vulkan.hpp
argglobal
silent! argdel *
edit /opt/samba/nxf47746/project/sdk/yocto/4.14-ga-wayland/sysroots/aarch64-poky-linux/usr/include/vulkan/vulkan.hpp
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd _ | wincmd |
split
1wincmd k
wincmd w
wincmd w
wincmd t
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe '1resize ' . ((&lines * 33 + 34) / 69)
exe 'vert 1resize ' . ((&columns * 135 + 135) / 271)
exe '2resize ' . ((&lines * 32 + 34) / 69)
exe 'vert 2resize ' . ((&columns * 135 + 135) / 271)
exe 'vert 3resize ' . ((&columns * 135 + 135) / 271)
argglobal
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 25740 - ((18 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
25740
normal! 07|
wincmd w
argglobal
if bufexists("render.hpp") | buffer render.hpp | else | edit render.hpp | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 2 - ((1 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
2
normal! 028|
wincmd w
argglobal
if bufexists("render.cpp") | buffer render.cpp | else | edit render.cpp | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 220 - ((23 * winheight(0) + 33) / 66)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
220
normal! 047|
wincmd w
3wincmd w
exe '1resize ' . ((&lines * 33 + 34) / 69)
exe 'vert 1resize ' . ((&columns * 135 + 135) / 271)
exe '2resize ' . ((&lines * 32 + 34) / 69)
exe 'vert 2resize ' . ((&columns * 135 + 135) / 271)
exe 'vert 3resize ' . ((&columns * 135 + 135) / 271)
tabnext 1
if exists('s:wipebuf') && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 winminheight=1 winminwidth=1 shortmess=filnxtToOFs
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
