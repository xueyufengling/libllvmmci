#include <llvmmci/llvm_mci.h>

#include <malloc.h>
#include <llvmmci/opcode.h>
#include <llvmmci/linker.h>

array* alloc_array(size_t length)
{
	array* arr = (array*)malloc(length + sizeof(size_t));
	arr->length = length;
	return arr;
}

void free_array(array* arr)
{
	free(arr);
}

assembly_context* host_assembly_context()
{
	return (assembly_context*)llvmmci::host_assembly_context;
}

assembly_unit* create_new_asm_unit(::assembly_context* as_ctx, bool DoAutoReset, const char* Swift5ReflSegmentName)
{
	return (assembly_unit*)((llvmmci::assembly_context*)as_ctx)->new_asm_unit(DoAutoReset, Swift5ReflSegmentName);
}

void asm_unit_add_src(assembly_unit* unit, const char* src)
{
	((llvmmci::assembly_unit*)unit)->add_src_buffer(src);
}

array* assemble_asm_unit(assembly_unit* unit, bool PIC, bool LargeCodeModel, unsigned syntax)
{
	return ((llvmmci::assembly_unit*)unit)->assemble(PIC, LargeCodeModel, (llvmmci::assembly_syntax)syntax);
}

void free_asm_unit(assembly_unit* unit)
{
	delete (llvmmci::assembly_unit*)unit;
}

dynamic_linker* global_dynamic_linker()
{
	return (dynamic_linker*)llvmmci::global_dynamic_linker;
}

dynamic_lib_context* dynamic_link_target(dynamic_linker* linker, const char* lib_name)
{
	return (dynamic_lib_context*)((llvmmci::dynamic_linker*)linker)->link_target(lib_name);
}

void dynamic_link_o(dynamic_lib_context* lib_ctx, array* o)
{
	((llvmmci::dynamic_lib_context*)lib_ctx)->add_o(o);
}

void* dynamic_symbol_lookup(dynamic_lib_context* lib_ctx, const char* sym_name)
{
	return ((llvmmci::dynamic_lib_context*)lib_ctx)->lookup(sym_name);
}

void free_dynamic_lib(dynamic_lib_context* lib_ctx)
{
	delete (llvmmci::dynamic_lib_context*)lib_ctx;
}
