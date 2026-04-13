#ifndef _LLVMMCI_MCI
#define _LLVMMCI_MCI

/**
 * 本库依赖llvm进行汇编、反汇编、链接、加载，编译时需要在msys2中使用命令 pacman -S mingw-w64-ucrt-x86_64-llvm 以安装llvm
 * llvm依赖zlib，安装 pacman -S mingw-w64-ucrt-x86_64-zlib
 * 运行时则可直接调用导出函数，所有llvm相关模块被静态链接进了本so库
 */

#include <stdint.h>
#include <llvmmci/struct.h>

// 导出的C接口兼容层
extern "C"
{
struct architecture_context;
struct assembler;
struct disassembler;

__declspec(dllexport) extern architecture_context* host_architecture_context();

__declspec(dllexport) extern assembler* create_new_assembler(architecture_context* as_ctx, bool DoAutoReset, const char* Swift5ReflSegmentName);

__declspec(dllexport) extern void assembler_add_src(assembler* unit, const char* src);

__declspec(dllexport) extern array* assemble_unit(assembler* unit, bool PIC, bool LargeCodeModel, unsigned syntax);

__declspec(dllexport) extern void free_assembler(assembler* unit);

__declspec(dllexport) extern disassembler* create_new_disassembler(architecture_context* as_ctx, unsigned syntax);

__declspec(dllexport) extern disassembler* host_att_disassembler();

__declspec(dllexport) extern array* disassemble_text(disassembler* disassembler, const void* text, size_t len);

__declspec(dllexport) extern array* disassemble_o(disassembler* disassembler, const array* o, size_t data_align);

struct dynamic_linker;
struct dynamic_lib_target;

__declspec(dllexport) extern dynamic_linker* global_dynamic_linker();

__declspec(dllexport) extern dynamic_lib_target* dynamic_link_target(dynamic_linker* linker, const char* lib_name);

__declspec(dllexport) extern void dynamic_link_o(dynamic_lib_target* lib_ctx, array* o);

__declspec(dllexport) extern void* dynamic_symbol_lookup(dynamic_lib_target* lib_ctx, const char* sym_name);

__declspec(dllexport) extern void free_dynamic_lib(dynamic_lib_target* lib_ctx);
}

#endif //_LLVMMCI_MCI
