#ifndef _LLVMMCI_OPCODE
#define _LLVMMCI_OPCODE

#include <llvmmci/struct.h>

#include <unordered_map>
#include <vector>

namespace llvm
{
class raw_ostream;
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
class SectionRef;
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
	llvm::object::ObjectFile* object_file_from(const void* o, size_t len);

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
	bool ignore_err = false; //遇到汇编错误时是否忽略错误尝试继续汇编下一条指令
	const char* swift_refl_seg_name = nullptr; //Swift5的反射段名称
	llvm::SourceMgr* src_ctx = nullptr; //汇编指令源码缓冲区
	llvm::MCContext* ctx = nullptr; //不同单元的上下文不能复用

	assembler(architecture_context* as_ctx);

	~assembler();

	/**
	 * @brief 为当前汇编单元添加源码
	 */
	void add_src(const char* src);

	/**
	 * @brief 编译当前汇编单元，编译完成后不清除源码，需要使用clear_uint()手动清除
	 * @param PIC Position-independent code，即是否生成位置无关代码
	 */
	array* assemble(bool PIC = true, bool LargeCodeModel = false, assembly_syntax syntax = assembly_syntax::ASM_SYNTAX_ATT);

	/**
	 * @brief 清除当前汇编单元并新建汇编单元
	 */
	void new_uint();
};

extern assembler* host_assembler;

enum symbol_flag : unsigned
{
	SF_None = 0,
	SF_Undefined = 1U << 0,      // Symbol is defined in another object file
	SF_Global = 1U << 1,         // Global symbol
	SF_Weak = 1U << 2,           // Weak symbol
	SF_Absolute = 1U << 3,       // Absolute symbol
	SF_Common = 1U << 4,         // Symbol has common linkage
	SF_Indirect = 1U << 5,       // Symbol is an alias to another symbol
	SF_Exported = 1U << 6,       // Symbol is visible to other DSOs
	SF_FormatSpecific = 1U << 7, // Specific to the object file format
								 // (e.g. section symbols)
	SF_Thumb = 1U << 8,          // Thumb symbol in a 32-bit ARM binary
	SF_Hidden = 1U << 9,         // Symbol has hidden visibility
	SF_Const = 1U << 10,         // Symbol value is constant
	SF_Executable = 1U << 11,    // Symbol points to an executable section
								  // (IR only)
};

enum symbol_type
{
	ST_Unknown, // Type not specified
	ST_Other,
	ST_Data,
	ST_Debug,
	ST_File,
	ST_Function,
};

struct obj_symbol
{
	const char* name = nullptr;
	const char* sec = nullptr; //所属的段
	uint64_t sec_offset = 0; //段内偏移量
	uint64_t value = 0;
	symbol_type type = symbol_type::ST_Unknown;
	uint64_t flags = symbol_flag::SF_None;
};

/**
 * @brief .o文件
 */
struct obj_file
{
	llvm::object::ObjectFile* obj;

	inline obj_file(architecture_context* as_ctx, const array* o) :
			obj_file(as_ctx, o->data, o->length)
	{
	}

	obj_file(architecture_context* as_ctx, const void* o, size_t len);

	~obj_file();

	inline operator bool()
	{
		return obj;
	}

	/**
	 * @brief 获取所有段
	 */
	std::vector<llvm::object::SectionRef> sections();

	/**
	 * @brief 解析指定段有地址的符号
	 */
	static void parse_sec_addr_symbols(llvm::object::ObjectFile* obj, const char* sec_name, std::unordered_map<uint64_t, obj_symbol>& sec_symbols);

	inline void parse_sec_addr_symbols(const char* sec_name, std::unordered_map<uint64_t, obj_symbol>& sec_symbols)
	{
		parse_sec_addr_symbols(obj, sec_name, sec_symbols);
	}
};

struct disassembler
{
	architecture_context* as_ctx;
	llvm::MCContext* ctx; //不同单元的上下文可以复用
	llvm::MCDisassembler* dis_asm; //可复用
	llvm::MCInstPrinter* inst_printer; //可复用

	disassembler(architecture_context* as_ctx, assembly_syntax syntax = assembly_syntax::ASM_SYNTAX_ATT);

	~disassembler();

	array* disassemble_text(const void* text, size_t len);

	array* disassemble_o(const void* o, size_t len, size_t data_align = 16);

protected:
	bool disasm_text(llvm::raw_ostream& out, const void* img_text_base, size_t total_size, std::unordered_map<uint64_t, llvmmci::obj_symbol>* sec_symbols = nullptr, uint64_t load_base_addr = 0);

	void dump_data_sec_hex(llvm::raw_ostream& out, const llvm::object::SectionRef& sec, size_t data_align);

	bool disasm_text_sec(llvm::raw_ostream& out, const llvm::object::SectionRef& sec, std::unordered_map<uint64_t, llvmmci::obj_symbol>* sec_symbols = nullptr);
};

extern disassembler* host_disassembler;

}

#endif //_LLVMMCI_OPCODE
