$! -----------------------------------------------
$! BUILD_LARN_VMS.COM — OpenVMS build script for Larn
$! For: VAX, Alpha, IA64 (legacy), x86_64
$! Requires: DEC C, CURSES.OLB
$! -----------------------------------------------
$ set noon
$ write sys$output "=== Building Larn for OpenVMS ==="

$! ----------------------
$! Detect DEC C compiler
$! ----------------------
$ cc := cc
$ if f$search("sys$system:decc$compiler.exe") .nes. "" then cc := "cc/decc"

$ write sys$output "Compiler: ''cc'"
$ cflags = "/standard=ansi89 /warnings=all /pointer_size=32 /float=ieee /prefix=all"

$! VAX needs /float=g_float instead of IEEE
$ arch = f$getsyi("arch_name")
$ if arch .eqs. "VAX" then cflags = "/standard=ansi89 /warnings=all /float=g_float /prefix=all"

$ write sys$output "CFLAGS: ''cflags'"

$! -------------------------
$! Create object directory
$! -------------------------
$ if f$search("obj.dir") .eqs. "" then create/directory obj

$! -------------------------
$! Compile all .c files
$! -------------------------
$ files = f$search("*.c")
$ if files .eqs. "" then goto NOFILES

$ loop:
$   file = files
$   base = f$parse(file,,,"name")
$   write sys$output "Compiling: ''file'"
$   'cc' 'cflags' 'file' /object=obj/''base'.obj
$   files = f$search("*.c")
$   if files .nes. "" then goto loop

$! ----------------------------
$! Link against OpenVMS curses
$! ----------------------------
$ write sys$output "Linking Larn"
$ objs = "obj:*.obj"

$! OpenVMS curses library
$ curses = "sys$library:curses.olb/lib"

$ link/exe=larn.exe 'objs', 'curses'

$ write sys$output "=== Build complete: LARN.EXE ==="
$ exit

$NOFILES:
$ write sys$output "No C source files found."
$ exit
