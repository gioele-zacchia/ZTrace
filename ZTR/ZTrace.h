#ifndef _ZTRACE_H_
#define _ZTRACE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include "procmem.h"
#include "elf.h"
#include "dwarf.h"



typedef struct SymbolMapEntity{
    char* fileName;
    ELFFunctionSymbol* functionSymbols;
    int programFunctionSymbolsNum;
    void* baseAddress;
    struct SymbolMapEntity* next;
    DWARFDebugInfo* debugInfo;
    bool isSelfproc;
    ELFFile* elf;
}SymbolMapEntity;

SymbolMapEntity* ZTsymbolMap;
processMapping* ZTProcMap;
int ZTProcessMappingLen;


extern void ZTInstall();
extern void ZTClean();

#endif