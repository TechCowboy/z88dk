
// automatically generated by m4 from headers in proto subdir


#ifndef __LIBGEN_H__
#define __LIBGEN_H__

#include <sys/compiler.h>

extern char __LIB__ *basename(char *path);
#ifndef __STDC_ABI_ONLY
extern char __LIB__ *basename_fastcall(char *path) __z88dk_fastcall;
#define basename(x) basename_fastcall(x)
#endif


extern char __LIB__ *basename_ext(char *path);
#ifndef __STDC_ABI_ONLY
extern char __LIB__ *basename_ext_fastcall(char *path) __z88dk_fastcall;
#define basename_ext(x) basename_ext_fastcall(x)
#endif

extern char __LIB__ *dirname(char *path);
#ifndef __STDC_ABI_ONLY
extern char __LIB__ *dirname_fastcall(char *path) __z88dk_fastcall;
#define dirname(x) dirname_fastcall(x)
#endif

__ZPROTO2(unsigned char,*,glob,const char *,s,const char *,pattern)
#ifndef __STDC_ABI_ONLY
extern unsigned char __LIB__ glob_callee(const char *s,const char *pattern) __smallc __z88dk_callee;
#define glob(a,b) glob_callee(a,b)
#endif

__ZPROTO2(unsigned char,*,glob_dos,const char *,s,const char *,pattern)
#ifndef __STDC_ABI_ONLY
extern unsigned char __LIB__ glob_dos_callee(const char *s,const char *pattern) __smallc __z88dk_callee;
#define glob_dos(a,b) glob_dos_callee(a,b)
#endif

__ZPROTO2(unsigned char,*,glob_fat,const char *,s,const char *,pattern)
#ifndef __STDC_ABI_ONLY
extern unsigned char __LIB__ glob_fat_callee(const char *s,const char *pattern) __smallc __z88dk_callee;
#define glob_fat(a,b) glob_fat_callee(a,b)
#endif


extern char __LIB__ *pathnice(char *path);
#ifndef __STDC_ABI_ONLY
extern char __LIB__ *pathnice_fastcall(char *path) __z88dk_fastcall;
#define pathnice(x) pathnice_fastcall(x)
#endif


#endif
