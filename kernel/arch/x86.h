/* RedWill OS kernel - i386 protected mode, VBE framebuffer, polled PS/2, PC-speaker sound */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#ifndef HOST
static inline void outb(u16 p, u8 v) { __asm__ volatile("outb %0,%1" ::"a"(v), "Nd"(p)); }
static inline void outw(u16 p, u16 v) { __asm__ volatile("outw %0,%1" ::"a"(v), "Nd"(p)); }
static inline u8 inb(u16 p) { u8 v; __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(p)); return v; }
void *memset(void *d, int c, unsigned n) { void *r = d; __asm__ volatile("rep stosb" : "+D"(d), "+c"(n) : "a"(c) : "memory"); return r; }
void *memcpy(void *d, const void *s, unsigned n) { void *r = d; __asm__ volatile("rep movsb" : "+D"(d), "+S"(s), "+c"(n)::"memory"); return r; }
#else
#include <string.h>
#include <stdlib.h>
static void outb(u16 p, u8 v) { (void)p; (void)v; }
static void outw(u16 p, u16 v) { (void)p; (void)v; }
static u8 inb(u16 p) { (void)p; return 0; }
#endif

