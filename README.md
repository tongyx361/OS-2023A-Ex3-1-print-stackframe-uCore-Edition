# OS-2023A Ex3-1:print_stackframe() uCore-Edition Report

## 题目

原文链接：[课后练习](https://www.yuque.com/xyong-9fuoz/qczol5/uzf18vbnscar3hzi#v62k1)

> 在你选择的开发和运行环境下，写一个函数 print_stackframe()，用于获取当前位置的函数调用栈信息。实现如下一种或多种功能：函数入口地址、函数名信息、参数调用参数信息、返回值信息。
>
> 可能的环境选择：
>
> - 操作系统环境：Linux、uCore、rCore、MacOS、Windows…
> - 特权级：用户态、内核态
> - 编程语言：Rust、C…
>
> 已有的参考：
>
> - [在ucore中写一个函数print_stackframe()](https://piazza.com/class/i5j09fnsl7k5x0/post/1273)：这里有多种可以在 uCore 和 rCore 上可以工作的 print_stackframe() 实现；（[piazza访问帮助](https://www.yuque.com/xyong-9fuoz/qczol5/gwv6926mfr9nnhc5#pbG6V)）
> - [在 print_stackframe() 中如何打印出被调用函数的参数列表和运行时的值](https://piazza.com/class/i5j09fnsl7k5x0/post/996#)：这是一个开放的还没有很好实现的题目；
> - [获取内核堆栈回溯信息的实用程序](https://github.com/os-module/tracer)：北京理工大学陈林峰同学给出比较完善的 Rust 语言的函数调用栈回溯工具。

## 总结

- 收获：锻炼了自学能力，学习了一些汇编级别的知识
- 损失：消耗了大量时间（共计 ~6 hr）

## 解答

### 最终实现

```C
void print_stackframe(void)
{
	uint64 *sp, *ra, *fp;
	// int i, j;

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
```

### 输出

```
hello wrold!
Argument 0: foo
Argument 1: bar
Argument 2: baz
Stack frame #0: 48 Bytes [sp:0x0000000080217f90-fp:0x0000000080217fc0]
ra: 0x000000008020043c
[0x0000000080217fb0-0x0000000080217fb8]: 0x000000008020043c
[0x0000000080217fa8-0x0000000080217fb0]: 0x0000000080217ff0
[0x0000000080217fa0-0x0000000080217fa8]: 0x0000000000000000
[0x0000000080217f98-0x0000000080217fa0]: 0x0000000000000000
[0x0000000080217f90-0x0000000080217f98]: 0x0000000000000000
[0x0000000080217f88-0x0000000080217f90]: 0x0000000000000000
last_fp: 0x0000000080217ff0
Stack frame #1: 48 Bytes [sp:0x0000000080217fc0-fp:0x0000000080217ff0]
ra: 0x00000000802004ba
[0x0000000080217fe0-0x0000000080217fe8]: 0x00000000802004ba
[0x0000000080217fd8-0x0000000080217fe0]: 0x0000000080218000
[0x0000000080217fd0-0x0000000080217fd8]: 0x00000000802010e0
[0x0000000080217fc8-0x0000000080217fd0]: 0x00000000802010d8
[0x0000000080217fc0-0x0000000080217fc8]: 0x00000000802010d0
[0x0000000080217fb8-0x0000000080217fc0]: 0x0000000000000000
last_fp: 0x0000000080218000
Stack frame #2: 16 Bytes [sp:0x0000000080217ff0-fp:0x0000000080218000]
ra: 0x000000008020000c
[0x0000000080217ff0-0x0000000080217ff8]: 0x000000008020000c
[0x0000000080217fe8-0x0000000080217ff0]: 0x0000000000000000
last_fp: 0x0000000000000000
```

### 复现

使用方法：用本仓库中的 `main.c` 替换 [GitHub - LearningOS/uCore-Tutorial-Code-2023A](https://github.com/LearningOS/uCore-Tutorial-Code-2023A/) `ch1` 分支的 `main.c`，随后运行 `make run LOG=trace` 即可

```C
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

```

## 分析

我选择的是 uCore，对应的参考为 [在ucore中写一个函数print_stackframe()](https://piazza.com/class/i5j09fnsl7k5x0/post/1273)
由于还没有学习内核态相关知识，我选择将该链接中的用户态函数作为参考，即

```C
void print_stackframe(void)
{
	uint32_t ebp = read_ebp();
	uint32_t eip = read_eip();
	int i,j;
	for(i = 0; i < STACKFRAME_DEPTH && ebp != 0; i++)
	{
		cprintf("ebp = 0x%08x, eip = 0x%08x ", ebp, eip);
		cprintf("Args = ");
		uint32_t *base = (uint32_t *)ebp;
		uint32_t *args = base + 2;
		for(j = 0; j < 4; j++)
			cprintf("0x%08x ", args[j]);
		cprintf("\n");
		print_debuginfo(eip-1);
		eip = base[1];
		ebp = base[0];
	}
}
```

```C
void print_stackframe(void) {
    uint32_t ebp = read_ebp();
    uint32_t eip = read_eip();

    int i;
    for(i = 0; i < STACKFRAME_DEPTH; i ++) {
	cprintf("ebp: 0x%08x ", ebp);
	cprintf("eip: 0x%08x ", eip);

	int j;
	cprintf("args: ");
	uint32_t *args = (uint32_t *)ebp + 2;	// 由ebp偏移两个单位得到最后入栈的参数地址
	for(j = 0; j < 4; j ++) {
	    cprintf("0x%08x ", args[j]);
	}
    cprintf("\n");
    print_debuginfo(eip - 1);

	// 分清地址和值
	eip = ((uint32_t *)ebp + 1)[0];
	ebp = ((uint32_t *)ebp)[0];
    }
}
```

然而，在 2023 年秋季学期的“操作系统”课堂上，这一参考实现并不是非常容易读懂。

### 参考实现的前置知识

- 参考实现使用的是 20 年及以前使用的 X86-32 平台上的 uCore
  - `uint32_t` 可能来自 C 标准库，也可能来自旧 uCore
  - `read_ebp` / `read_eip` / `cprintf` / `print_debuginfo` 这些函数峻来自旧 uCore
- X86 的栈帧布局（参考 [x86-64 下函数调用及栈帧原理 - 知乎](https://zhuanlan.zhihu.com/p/27339191)）与 RISC-V （参考 [RISC-V架构的函数调用规范和栈布局\_正在起飞的蜗牛的博客-CSDN博客](https://blog.csdn.net/weixin_42031299/article/details/133350533)）不同，`%ebp` - `fp` / `%eip` - `sp` 并不是严格对应关系
- 第一个栈帧中的 frame pointer（上一个栈帧的最高地址处）一般指向 `0` 地址处，故可以使用类似 `ebp != 0` 的循环条件

### 得到本实现所需的前置知识

- 当前 uCore 的平台为 RISC-V 64
- 由于当前 uCore 不可以使用 `stdint.h`，也不提供 `uint64_t` 的定义，文档中也没有相关说明，需要自行查找源文件，发现 `types.h` 中定义了 `uint64`
- 由于当前 uCore 没有提供 `read_ebp` / `read_eip`，需要自行实现读取 RISC-V 中对应寄存器的汇编代码，这里需要学习使用
  - C 的内联汇编语法（参考 ChatGPT 与 [How to Use Inline Assembly Language in C Code - Using the GNU Compiler Collection 13.0.0 (experimental 20221114) documentation](https://gcc.gnu.org/onlinedocs/gcc/extensions-to-the-c-language-family/how-to-use-inline-assembly-language-in-c-code.html)）（也可以直接编写汇编函数，但感觉不如内联汇编方便，故暂时放弃）
  - 以及 RISC-V 汇编指令（参考 [RISC-V-Reader-Chinese-v2p1](http://riscvbook.com/chinese/RISC-V-Reader-Chinese-v2p1.pdf)）
- 关键是要了解 RISC-V 的栈帧布局，经过一番搜索可以得知这属于 ABI 约定，最终在 [6.1810 / Fall 2023](https://pdos.csail.mit.edu/6.828/2023/schedule.html) 找到了比较细致的解说（参考 [5.5 Stack - MIT6.S081](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/lec05-calling-conventions-and-stack-frames-risc-v/5.5-stack) 与 [https://pdos.csail.mit.edu/6.828/2023/readings/riscv-calling.pdf](https://pdos.csail.mit.edu/6.828/2023/readings/riscv-calling.pdf)）
