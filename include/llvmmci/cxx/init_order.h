#ifndef _LLVMMCI_INITORDER
#define _LLVMMCI_INITORDER

//初始化顺序
#define __opcode_init_order__ 101
#define __linker_init_order__ 102
#define __export_init_order__ 103

#define __init_module__(name) __attribute__((constructor(__##name##_init_order__), used)) static void __init_##name()

#endif //_LLVMMCI_INITORDER
