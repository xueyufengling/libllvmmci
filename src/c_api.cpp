#include <as/c_api.h>
#include <as/opcode.h>

#include <ppmp/semantic.h>
#include <sys/llvm/init.h>

architecture_context* host_architecture_context = nullptr;
assembler* host_assembler = nullptr;
disassembler* host_disassembler = nullptr;

// @formatter:off
__dynamic_init__(init_host_as,
{
	// 初始化汇编、反汇编相关组件
	sys::llvm::init_as(sys::llvm::host_llvm_target);
	// 初始化C API
	as::architecture_context* cxx_host_architecture_context = new as::architecture_context();
	as::assembler* cxx_host_assembler = new as::assembler(cxx_host_architecture_context);
	as::disassembler* cxx_host_disassembler = new as::disassembler(cxx_host_architecture_context, as::assembly_syntax::asm_syntax_att);
	host_architecture_context = (architecture_context*)cxx_host_architecture_context;
	host_assembler = (assembler*)cxx_host_assembler;
	host_disassembler = (disassembler*)cxx_host_disassembler;
})
// @formatter:on

assembler* create_new_assembler(architecture_context* as_ctx)
{
	return (assembler*)new as::assembler((as::architecture_context*)as_ctx);
}

void assembler_add_src(assembler* assembler, const char* src)
{
	((as::assembler*)assembler)->add_src(src);
}

c_array* assemble_unit(assembler* assembler, bool PIC, bool LargeCodeModel, unsigned syntax)
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

c_array* disassemble_text(disassembler* disassembler, const void* text, size_t text_size, uint64_t load_base_addr)
{
	return ((as::disassembler*)disassembler)->disassemble_text(text, text_size, load_base_addr);
}

c_array* disassemble_o(disassembler* disassembler, const c_array* o, size_t data_align)
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
