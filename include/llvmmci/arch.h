#ifndef _CXXSP_ARCH
#define _CXXSP_ARCH

// ***** CPU架构 *****

#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#define __ARCH_ALPHA__
#endif

//x86 64位
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64)
#define __ARCH_X86_64__
#endif

//x86 32位
#if defined(__i386__) || defined(__i386) || defined(i386) || defined(_M_IX86) || defined(_M_I86) || defined(__IA32__) || defined(__X86__) || defined(_X86_)
#define __ARCH_X86_32__
#endif

#if defined(__ARCH_X86_32__) || defined(__ARCH_X86_64__)
#define __ARCH_X86__
#endif

#if (defined(__arm__) || defined(_M_ARM))
#define __ARCH_ARM__
//ARM 64位
#if defined(__aarch64__) || defined(_M_ARM64)
#define __ARCH_AARCH_64__
#else
//ARM 32位
#define __ARCH_AARCH_32__
#endif
#endif

#if defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__ppc64__) || defined(__PPC__) || defined(__PPC64__) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(_M_PPC)
#define __ARCH_POWERPC_64__
#endif

// ***** 操作系统 *****

#if defined (_WIN32)
#define __OS_WIN__
#if defined (_WIN64)
#define __OS_WIN64__
#else
#define __OS_WIN32__
#endif

#endif

#if defined(__unix__) || defined(__unix) || defined(unix)
#define __OS_UNIX__
#endif

#define __OS_PAGE_SIZE__ ((unsigned int)0x1000)

// 操作系统保留内存，所有系统均保留第一页（地址0~4095）
#define __OS_RESERVED_ADDR__ ((void*)0x0FFF)

#endif //_CXXSP_ARCH
