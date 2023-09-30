#include "console.h"
#include "defs.h"

#define MAX_STACK_DEPTH 10

extern char s_text[];
extern char e_text[];
extern char s_rodata[];
extern char e_rodata[];
extern char s_data[];
extern char e_data[];
extern char s_bss[];
extern char e_bss[];

int threadid()
{
	return 0;
}

void clean_bss()
{
	char *p;
	for (p = s_bss; p < e_bss; ++p)
		*p = 0;
}

void print_stackframe(void)
{
	uint64 *sp, *ra, *fp;

	// directly get sp, fp of the last stack frame
	asm volatile("mv %0, fp" : "=r"(sp));
	asm volatile("ld %0, -16(fp)" : "=r"(fp));

	for (int i = 0; i < MAX_STACK_DEPTH && fp != 0; i++) {
		if (fp != 0)
			ra = (uint64 *)*(fp - 1);
		printf("Stack frame #%d: %d Bytes [sp:%p-fp:%p]\n", i,
		       (uint64)fp - (uint64)sp, sp, fp);
		printf("ra: %p\n", ra);
		// print whole stack frame
		for (int j = 1; fp - j >= sp; j++) {
			printf("[%p-%p]: %p\n", fp - (j + 1), fp - j,
			       *(fp - j));
		}

		// move to the last frame
		uint64 *last_fp = (uint64 *)*(fp - 2);
		printf("last_fp: %p\n", last_fp);
		sp = fp;
		fp = last_fp;
	}
}

// int bar(int d)
// {
// 	int a = 1, b = 2;
// 	int c;
// 	c = a + b + d;
// 	print_stackframe();
// 	return c;
// }

// int foo(int d)
// {
// 	int a = 1, b = 2;
// 	int c;
// 	c = a + b + d;
// 	bar(3);
// 	return c;
// }

int dummymain(int argc, char *argv[])
{
	int i = 0;
	for (; i < argc; i++) {
		printf("Argument %d: %s\n", i, argv[i]);
	}
	print_stackframe();
	return 0;
}
void demo4()
{
	char *args[] = { "foo", "bar", "baz" };
	int result = dummymain(sizeof(args) / sizeof(args[0]), args);
	if (result < 0)
		panic("Demo 4");
}

void main()
{
	clean_bss();
	console_init();
	printf("\n");
	printf("hello wrold!\n");
	// foo(3);
	demo4();
	errorf("stext: %p", s_text);
	warnf("etext: %p", e_text);
	infof("sroda: %p", s_rodata);
	debugf("eroda: %p", e_rodata);
	debugf("sdata: %p", s_data);
	infof("edata: %p", e_data);
	warnf("sbss : %p", s_bss);
	errorf("ebss : %p", e_bss);
	panic("ALL DONE");
}
