@echo off

set flags=/Zi
set libs=user32.lib Gdi32.lib
cl %flags% main.c /link %libs%