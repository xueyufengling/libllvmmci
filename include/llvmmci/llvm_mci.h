#ifndef _LLVMMCI_MCI
#define _LLVMMCI_MCI

/**
 * 本库依赖llvm进行汇编和反汇编，编译时需要在msys2中使用命令 pacman -S mingw-w64-ucrt-x86_64-llvm 以安装llvm
 * llvm依赖zlib，安装 pacman -S mingw-w64-ucrt-x86_64-zlib
 * 运行时则可直接调用导出函数，所有llvm相关模块被静态链接进了本so库
 */

#include <stdint.h>

// 导出的C接口兼容层
extern "C"
{
struct array
{
	size_t length;
	unsigned char data[1]; //数据起始地址，长度为length
};

__declspec(dllexport) extern array* alloc_array(size_t length);

__declspec(dllexport) extern void free_array(array* arr);

struct assembly_context;
struct assembly_unit;

__declspec(dllexport) extern assembly_context* host_assembly_context();

__declspec(dllexport) extern assembly_unit* create_new_asm_unit(assembly_context* as_ctx, bool DoAutoReset, const char* Swift5ReflSegmentName);

__declspec(dllexport) extern void asm_unit_add_src(assembly_unit* unit, const char* src);

__declspec(dllexport) extern array* assemble_asm_unit(assembly_unit* unit, bool PIC, bool LargeCodeModel, unsigned syntax);

__declspec(dllexport) extern void free_asm_unit(assembly_unit* unit);

struct dynamic_linker;
struct dynamic_lib_context;

__declspec(dllexport) extern dynamic_linker* global_dynamic_linker();

__declspec(dllexport) extern dynamic_lib_context* dynamic_link_target(dynamic_linker* linker, const char* lib_name);

__declspec(dllexport) extern void dynamic_link_o(dynamic_lib_context* lib_ctx, array* o);

__declspec(dllexport) extern void* dynamic_symbol_lookup(dynamic_lib_context* lib_ctx, const char* sym_name);

__declspec(dllexport) extern void free_dynamic_lib(dynamic_lib_context* lib_ctx);
}

#define __wrap_array_as_membuffer__(arr_ptr) llvm::MemoryBuffer::getMemBuffer(llvm::StringRef((const char*)arr_ptr->data, arr_ptr->length), "", false)

#endif //_LLVMMCI_MCI
