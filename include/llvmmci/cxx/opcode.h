#ifndef _LLVMMCI_OPCODE
#define _LLVMMCI_OPCODE

#include <llvmmci/struct.h>

namespace llvm
{
//架构信息
class Triple;
class Target;
class MCRegisterInfo;
class MCAsmInfo;
class MCInstrInfo;
class MCTargetOptions;
class MCSubtargetInfo;

//上下文
class MCContext;

//汇编
class MCAsmBackend;
class MCCodeEmitter;
class MCAsmParser;
class MCTargetAsmParser;
class SourceMgr;
class raw_svector_ostream;
class MCStreamer;

//反汇编
namespace object
{
class ObjectFile;
}
class MCDisassembler;
class MCInstPrinter;
}

namespace llvmmci
{
struct assembler;

enum assembly_syntax : unsigned
{
	ASM_SYNTAX_ATT = 0,
	ASM_SYNTAX_INTEL = 1,
};

struct architecture_context
{
	const llvm::Triple* triple; //架构三元组信息
	const llvm::Target* target_arch; //架构
	const llvm::MCTargetOptions* options;
	const llvm::MCAsmInfo* asm_info;
	const llvm::MCRegisterInfo* reg_info;
	const llvm::MCInstrInfo* inst_info;
	const llvm::MCSubtargetInfo* subtarget_info; //指定CPU和架构特性

	architecture_context(const char* target_arch_triple, const char* target_cpu, const char* feature);

	architecture_context(const char* target_arch_triple) :
			architecture_context(target_arch_triple, "generic", "")
	{
	}

	architecture_context();

	/**
	 * 基于当前的架构和options创建一个新的上下文，不同汇编单元上下文各自独立。
	 * MCContext包含各个汇编单元独有的数据，汇编时不同单元的代码不能复用，但反汇编可以复用。
	 */
	llvm::MCContext* new_context(const llvm::SourceMgr* Mgr = nullptr, bool DoAutoReset = true, const char* Swift5ReflSegmentName = nullptr);

	//汇编

	llvm::MCAsmBackend* new_asm_backend();

	llvm::MCCodeEmitter* new_code_emitter(llvm::MCContext* ctx);

	llvm::MCStreamer* new_object_streamer(llvm::MCContext* ctx, llvm::MCAsmBackend* backend, llvm::raw_svector_ostream* o_stream, llvm::MCCodeEmitter* code_emitter);

	llvm::MCAsmParser* new_asm_parser(llvm::MCContext* ctx, llvm::SourceMgr* src_ctx, llvm::MCStreamer* o_streamer, unsigned CB = 0);

	llvm::MCTargetAsmParser* new_target_asm_parser(llvm::MCAsmParser* parser);

	//反汇编
	llvm::object::ObjectFile* object_file_from(void* o, size_t len);

	inline llvm::object::ObjectFile* object_file_from(array* o)
	{
		return object_file_from(o->data, o->length);
	}

	llvm::MCDisassembler* new_disassembler(llvm::MCContext* ctx);

	llvm::MCInstPrinter* new_inst_printer(assembly_syntax dialect);
};

extern architecture_context* host_architecture_context;

/**
 * @brief 汇编文件单元，该单元内的指令和状态是互相关联的
 * 		  不可复用，不同的汇编单元需要分别创建assembler实例
 */
struct assembler
{
	architecture_context* as_ctx;
	llvm::SourceMgr* src_ctx; //汇编指令源码缓冲区
	llvm::MCContext* ctx; //不同单元的上下文不能复用

	assembler(architecture_context* as_ctx, bool DoAutoReset = true, const char* Swift5ReflSegmentName = nullptr);

	~assembler();

	void add_src(const char* src);

	array* assemble(bool PIC = true, bool LargeCodeModel = false, assembly_syntax syntax = assembly_syntax::ASM_SYNTAX_ATT);
};

struct disassembler
{
	architecture_context* as_ctx;
	llvm::MCContext* ctx; //不同单元的上下文可以复用
	llvm::MCDisassembler* dis_asm; //可复用
	llvm::MCInstPrinter* inst_printer; //可复用

	disassembler(architecture_context* as_ctx, assembly_syntax syntax = assembly_syntax::ASM_SYNTAX_ATT);

	~disassembler();

	array* disassemble_text(void* text, size_t len);
};

extern disassembler* host_att_disassembler;

}

#endif //_LLVMMCI_OPCODE
