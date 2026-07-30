#include <as/as.h>

#include <as/cxx/opcode.h>

assembler* create_new_assembler(architecture_context* as_ctx)
{
	return (assembler*)new as::assembler((as::architecture_context*)as_ctx);
}

void assembler_add_src(assembler* assembler, const char* src)
{
	((as::assembler*)assembler)->add_src(src);
}

array* assemble_unit(assembler* assembler, bool PIC, bool LargeCodeModel, unsigned syntax)
{
	return ((as::assembler*)assembler)->assemble(PIC, LargeCodeModel, (as::assembly_syntax)syntax);
}

void assembler_clear_unit(assembler* assembler)
{
	((as::assembler*)assembler)->new_uint();
}

void free_assembler(assembler* assembler)
{
	delete (as::assembler*)assembler;
}

disassembler* create_new_disassembler(architecture_context* as_ctx, unsigned syntax)
{
	return (disassembler*)new as::disassembler((as::architecture_context*)as_ctx, (as::assembly_syntax)syntax);
}

array* disassemble_text(disassembler* disassembler, const void* text, size_t text_size, uint64_t load_base_addr)
{
	return ((as::disassembler*)disassembler)->disassemble_text(text, text_size, load_base_addr);
}

array* disassemble_o(disassembler* disassembler, const array* o, size_t data_align)
{
	return ((as::disassembler*)disassembler)->disassemble_o(o->data, o->size, data_align);
}

uint64_t disassembler_find_return(disassembler* disassembler, const void* img_base, size_t max_size, uint64_t load_base_addr, int counter)
{
	return ((as::disassembler*)disassembler)->find_return(img_base, max_size, load_base_addr, counter);
}

uint64_t disassembler_find_call(disassembler* disassembler, const void* img_base, size_t max_size, uint64_t load_base_addr, int counter)
{
	return ((as::disassembler*)disassembler)->find_call(img_base, max_size, load_base_addr, counter);
}

uint64_t disassembler_find_opcode(disassembler* disassembler, const void* img_base, size_t max_size, uint64_t load_base_addr, unsigned int opcode, int counter)
{
	return ((as::disassembler*)disassembler)->find_opcode(img_base, max_size, load_base_addr, opcode, counter);
}
