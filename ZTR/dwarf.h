#ifndef __DWARF_H
#define __DWARF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"
#include "LEB128.h"
#include "avltree.h"


#define DWARF_64 2
#define DWARF_32 1

#define DEBUG_LINE_SECTION ".debug_line"

//#define DEBUG_MODE


typedef struct DWARFHeaderNOPad{
    uint8_t minimum_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
}DWARFHeaderNOPad;

typedef struct DWARFFile{
    char* name;
    uint32_t directoryIndex;
    uint32_t lastModify;
    uint32_t len;
}DWARFFile;

typedef struct DWARFFileNode{
    DWARFFile* info;
    struct DWARFFileNode* next;
}DWARFFileNode;

typedef struct DWARFLineHeader{
    uint32_t unit_lenght;
    uint16_t version;
    uint64_t header_lenght;
    uint8_t minimum_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    uint8_t* standard_opcode_lenghts;
    uint8_t directories_num;
    char** include_directories;
    uint8_t file_num;
    DWARFFile** file_names;
    size_t program_start_offset;
    uint64_t base_address;
}DWARFLineHeader;

enum DWARFLineStateFlags{
    is_stmt = 0x1,
    basic_block = 0x2,
    end_sequence = 0x4,
    prologue_end = 0x8,
    epilogue_end = 0x16
};

#define    DW_LNS_copy  1
#define    DW_LNS_advance_pc  2
#define    DW_LNS_advance_line  3
#define    DW_LNS_set_file  4
#define    DW_LNS_set_column  5
#define    DW_LNS_negate_stmt  6
#define    DW_LNS_set_basic_block  7
#define    DW_LNS_const_add_pc  8
#define    DW_LNS_fixed_advance_pc  9
#define    DW_LNS_set_prologue_end  10
#define    DW_LNS_set_epilogue_begin  11
#define    DW_LNS_set_isa  12


#define DW_LNE_end_sequence 1
#define DW_LNE_set_address 2
#define DW_LNE_define_file 3



typedef struct DWARFLineState{
    void* address;
    unsigned int file;
    unsigned int line;
    unsigned int column;
    enum DWARFLineStateFlags flags;
    unsigned int isa;
    DWARFLineHeader* header;
    

}DWARFLineState;

typedef struct DWARFHeaderNode{
    DWARFLineHeader* header;
    struct DWARFHeaderNode* next;
}DWARFHeaderNode;

typedef struct DWARFDebugInfo{
    AVLNode* lineTree;
    DWARFHeaderNode* headers;
}DWARFDebugInfo;

DWARFDebugInfo* DWARFGetDebugInfo(ELFFile* elf,FILE* fp,void* baseAddr);
void DWARFClean(DWARFDebugInfo* info);
char* DWARFGetFileName(DWARFLineState* state);
void printTreeToFile(AVLNode* tree,FILE* fp);

#endif