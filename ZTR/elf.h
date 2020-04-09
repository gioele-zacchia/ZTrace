#ifndef __ELF_H
#define __ELF_H

#include <stdint.h>

#if defined(__LP64__) || defined(_LP64)
# define BUILD_64   1
#else
# define BUILD_64 0
#endif

#define EI_CLASS_32 1
#define EI_CLASS_64 2

#define ELF_SYMBOL_NAME_SH ".strtab"
#define ELF_SYMBOL_TABLE_SH ".symtab"

#define ELF_DYN_SYMBOL_NAME_SH ".dynstr"
#define ELF_DYN_SYMBOL_TABLE_SH ".dynsym"

#define ELF_STT_FUNC 2
#define ELF_MAGIC "\x7F""ELF"

#define offsetof(st, m) \
    ((size_t)&(((st *)0)->m))



typedef struct ELFHeader{
    uint8_t EI_CLASS;
    uint8_t EI_DATA;
    uint8_t EI_VERSION;
    uint8_t EI_OSABI;
    uint8_t EI_ABIVERSION;
    uint8_t EI_PAD[7];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
}ELFHeader;

typedef struct ELF32HeaderMid{
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
}ELF32HeaderMid;

typedef struct ELFSectionHeader{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
}ELFSectionHeader;

typedef struct ELF32SectionHeader{
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
}ELF32SectionHeader;

typedef struct ELFSymbol{
    uint32_t st_name;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint32_t st_size;
}ELFSymbol;

typedef struct ELF32Symbol{
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;

}ELF32Symbol;

typedef struct ELFFile{
    ELFHeader* header;
    ELFSectionHeader* sections;
    unsigned int sectionNum;
    ELFSymbol* symbols;
    unsigned int symbolNum;
    char* sectionNameTable;
    char* symbolNameTable;
}ELFFile;

typedef struct ELFFunctionSymbol{
    char* name;
    void* value;
    uint8_t bind;
}ELFFunctionSymbol;

void readHeader(FILE* fp,ELFHeader* header);
int readSectionHeaders(FILE* fp, ELFHeader* header, ELFSectionHeader** sections);
char* getStringTable(FILE* fp, ELFSectionHeader* section);
char* getSectionNameTable(FILE* fp, ELFHeader* header, ELFSectionHeader* sections);
char* getSectionName(ELFSectionHeader* section, char* sectionNameTable);
char* getSymbolName(ELFSymbol* symbol, char* symbolNameTable);
uint8_t getSymbolType(ELFSymbol* symbol);
uint8_t getSymbolBind(ELFSymbol* symbol);
int readSymbolTable(FILE* fp,ELFHeader* header, ELFSectionHeader* section,ELFSymbol** buffer);
ELFFile* readElf(FILE* fp);
void freeELF(ELFFile* file);
ELFFile* readself();
int getFunctiosForSection(ELFFile* file,char* targetSection,ELFFunctionSymbol** functionsBuffer);
int loadSection(FILE* fp, ELFSectionHeader* section,void** buffer);
#endif