#include <llvmmci/mci.h>

#include <llvmmci/cxx/opcode.h>
#include <llvmmci/cxx/linker.h>

#include <llvmmci/cxx/init_order.h>

architecture_context* host_architecture_context = nullptr;
thread_local assembler* host_assembler = nullptr;
disassembler* host_disassembler = nullptr;
dynamic_linker* global_dynamic_linker = nullptr;

__init_module__(export)
{
	host_architecture_context = (architecture_context*)llvmmci::host_architecture_context;
	host_assembler = (assembler*)llvmmci::host_assembler;
	host_disassembler = (disassembler*)llvmmci::host_disassembler;
	global_dynamic_linker = (dynamic_linker*)llvmmci::global_dynamic_linker;
}

assembler* create_new_assembler(architecture_context* as_ctx)
{
	return (assembler*)new llvmmci::assembler((llvmmci::architecture_context*)as_ctx);
}

void assembler_add_src(assembler* assembler, const char* src)
{
	((llvmmci::assembler*)assembler)->add_src(src);
}

array* assemble_unit(assembler* assembler, bool PIC, bool LargeCodeModel, unsigned syntax)
{
	return ((llvmmci::assembler*)assembler)->assemble(PIC, LargeCodeModel, (llvmmci::assembly_syntax)syntax);
}

void assembler_clear_unit(assembler* assembler)
{
	((llvmmci::assembler*)assembler)->new_uint();
}

void free_assembler(assembler* assembler)
{
	delete (llvmmci::assembler*)assembler;
}

dynamic_lib_target* dynamic_link_target(dynamic_linker* linker, const char* lib_name)
{
	return (dynamic_lib_target*)((llvmmci::dynamic_linker*)linker)->link_target(lib_name);
}

void dynamic_link_o(dynamic_lib_target* lib_ctx, array* o)
{
	((llvmmci::dynamic_lib_target*)lib_ctx)->add_o(o);
}

void* dynamic_symbol_lookup(dynamic_lib_target* lib_ctx, const char* sym_name)
{
	return ((llvmmci::dynamic_lib_target*)lib_ctx)->lookup(sym_name);
}

void free_dynamic_lib(dynamic_lib_target* lib_ctx)
{
	delete (llvmmci::dynamic_lib_target*)lib_ctx;
}

disassembler* create_new_disassembler(architecture_context* as_ctx, unsigned syntax)
{
	return (disassembler*)new llvmmci::disassembler((llvmmci::architecture_context*)as_ctx, (llvmmci::assembly_syntax)syntax);
}

array* disassemble_text(disassembler* disassembler, const void* text, size_t len)
{
	return ((llvmmci::disassembler*)disassembler)->disassemble_text(text, len);
}

array* disassemble_o(disassembler* disassembler, const array* o, size_t data_align)
{
	return ((llvmmci::disassembler*)disassembler)->disassemble_o(o->data, o->length, data_align);
}
