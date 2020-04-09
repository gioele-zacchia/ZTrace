#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../ZTR/elf.h"

void readHeader(FILE* fp,ELFHeader* header){
    char* buffer = malloc(sizeof(ELF_MAGIC));
    fread(buffer,sizeof(ELF_MAGIC)-1,1,fp);
    char* magic = ELF_MAGIC;
    int i = 0;
    while(*(magic+i)){
        if(*(magic+i)!=*(buffer+i)){
            printf("Magic wrong\n");
            return;
        }
        i++;
    }
    //printf("Magic ok\n");


    
    fread(header,offsetof(ELFHeader,e_entry),1,fp);
    //long offset = ftell(fp);
    if(header->EI_CLASS== EI_CLASS_32 ){
        ELF32HeaderMid* hMid = malloc(sizeof(ELF32HeaderMid));
        fread(hMid,sizeof(ELF32HeaderMid),1,fp);
        header->e_entry = hMid->e_entry;
        header->e_phoff = hMid->e_phoff;
        header->e_shoff = hMid->e_shoff;
        free(hMid);
    }else{
        fseek(fp,-0x4,SEEK_CUR); //Compensate for padding
        fread(((void*)header)+offsetof(ELFHeader,e_entry),offsetof(ELFHeader,e_flags)-offsetof(ELFHeader,e_entry),1,fp);
    }
    fread(((void*)header)+offsetof(ELFHeader,e_flags),sizeof(ELFHeader)-offsetof(ELFHeader,e_flags),1,fp);
}

int readSectionHeaders(FILE* fp, ELFHeader* header, ELFSectionHeader** sections){
    *sections = malloc(sizeof(ELFSectionHeader)*header->e_shnum);
    ELFSectionHeader* cSection = *sections;
    fseek(fp,header->e_shoff,SEEK_SET);
    ELF32SectionHeader* buffer32;
    if(header->EI_CLASS == EI_CLASS_32){
        buffer32 = malloc(sizeof(ELF32SectionHeader));
    }
    for(int i = 0; i < header->e_shnum; i++){
        if(header->EI_CLASS == EI_CLASS_64){
            fread(cSection,sizeof(ELFSectionHeader),1,fp);
        }else{
            fread(buffer32,sizeof(ELF32SectionHeader),1,fp);
            cSection->sh_name = buffer32->sh_name;
            cSection->sh_type = buffer32->sh_type;
            cSection->sh_flags = buffer32->sh_flags;
            cSection->sh_addr = buffer32->sh_addr;
            cSection->sh_offset = buffer32->sh_offset;
            cSection->sh_size = buffer32->sh_size;
            cSection->sh_link = buffer32->sh_link;
            cSection->sh_info = buffer32->sh_info;
            cSection->sh_addralign = buffer32->sh_addralign;
            cSection->sh_entsize = buffer32->sh_entsize;
        }
        cSection++;
    }
    if(header->EI_CLASS == EI_CLASS_32){
        free(buffer32);
    }
    return header->e_shnum;
}

char* getStringTable(FILE* fp, ELFSectionHeader* section){
    fseek(fp,section->sh_offset,SEEK_SET);
    char* buffer = malloc(section->sh_size);
    fread(buffer,section->sh_size,1,fp);
    return buffer;
}

char* getSectionNameTable(FILE* fp, ELFHeader* header, ELFSectionHeader* sections){
    ELFSectionHeader* shstr = sections + header->e_shstrndx;
    return getStringTable(fp,shstr);
}

char* getSectionName(ELFSectionHeader* section, char* sectionNameTable){
    return sectionNameTable + section->sh_name;
}

char* getSymbolName(ELFSymbol* symbol, char* symbolNameTable){
    return symbolNameTable + symbol->st_name;
}

uint8_t getSymbolType(ELFSymbol* symbol){
    return symbol->st_info&0xf;
}

uint8_t getSymbolBind(ELFSymbol* symbol){
    return symbol->st_info>>4;
}

int readSymbolTable(FILE* fp,ELFHeader* header, ELFSectionHeader* section,ELFSymbol** buffer){
    int symbolNum = section->sh_size/section->sh_entsize;
    *buffer = malloc(sizeof(ELFSymbol)*symbolNum);
    ELF32Symbol* symbol32;
    fseek(fp,section->sh_offset,SEEK_SET);
    if(header->EI_CLASS == EI_CLASS_32){
        symbol32 = malloc(sizeof(ELF32Symbol));
    }
    for(int i = 0; i < symbolNum; i++){
        if(header->EI_CLASS == EI_CLASS_64){
            fread((*buffer)+i,sizeof(ELFSymbol),1,fp);
        }else{
            fread(symbol32,sizeof(ELF32Symbol),1,fp);
            ELFSymbol* t = (*buffer) +i;
            t->st_name = symbol32->st_name;
            t->st_value= symbol32->st_value;
            t->st_size= symbol32->st_size;
            t->st_info= symbol32->st_info;
            t->st_other= symbol32->st_other;
            t->st_shndx= symbol32->st_shndx;
        }
    }
    if(header->EI_CLASS == EI_CLASS_32){
        free(symbol32);
    }
    return symbolNum;

}

ELFFile* readElf(FILE* fp){
    ELFFile* ret = malloc(sizeof(ELFFile));

    ret->header = malloc(sizeof(ELFHeader));
    readHeader(fp,ret->header);
    ret->sectionNum = readSectionHeaders(fp,ret->header,&ret->sections);

    ret->sectionNameTable = getSectionNameTable(fp,ret->header,ret->sections);

    ret->symbolNameTable = NULL;
    ret->symbolNum = 0;
    ret->symbols = NULL;
    for(int i = 0; i < ret->sectionNum; i++){
        
        if(strcmp(getSectionName(ret->sections+i,ret->sectionNameTable),ELF_SYMBOL_NAME_SH) == 0){
            ret->symbolNameTable = getStringTable(fp,ret->sections+i);
        }
        if(strcmp(getSectionName(ret->sections+i,ret->sectionNameTable),ELF_SYMBOL_TABLE_SH) == 0){
           ret->symbolNum = readSymbolTable(fp,ret->header,ret->sections+i,&ret->symbols);
        }
    }

    if(ret->symbols == NULL){
        for(int i = 0; i < ret->sectionNum; i++){
            
            if(strcmp(getSectionName(ret->sections+i,ret->sectionNameTable),ELF_DYN_SYMBOL_NAME_SH) == 0){
                ret->symbolNameTable = getStringTable(fp,ret->sections+i);
            }
            if(strcmp(getSectionName(ret->sections+i,ret->sectionNameTable),ELF_DYN_SYMBOL_TABLE_SH) == 0){
                ret->symbolNum = readSymbolTable(fp,ret->header,ret->sections+i,&ret->symbols);
            }
        }
    }

    return ret;
}

void freeELF(ELFFile* file){
    free(file->header);
    free(file->sectionNameTable);
    free(file->sections);
    free(file->symbolNameTable);
    free(file->symbols);
    free(file);
}

ELFFile* readself(FILE** _fp){
    FILE* fp = fopen("/proc/self/exe","r");
    if(fp == 0){
        int errsv = errno;
        fprintf(stderr,"Fopen failed: Errno: %d\n",errsv);
        return NULL;
    }

    ELFFile* elf = readElf(fp);

    if(_fp != NULL){
        *_fp = fp;
    }else{
        fclose(fp);
    }

    return elf;
}


int getFunctiosForSection(ELFFile* file,char* targetSection,ELFFunctionSymbol** functionsBuffer){
    int c = 0;
    for(int i = 0; i < file->symbolNum; i++){
        ELFSymbol* symb = file->symbols+i;
        if((getSymbolType(symb) & ELF_STT_FUNC) && (strcmp(getSectionName(file->sections+symb->st_shndx,file->symbolNameTable),targetSection))){
            c++;
        }
    }
    *functionsBuffer = malloc(sizeof(ELFFunctionSymbol)*c);
    ELFFunctionSymbol* cFunc = *functionsBuffer;
    for(int i = 0; i < file->symbolNum; i++){
        ELFSymbol* symb = file->symbols+i;
        if((getSymbolType(symb) & ELF_STT_FUNC) && (strcmp(getSectionName(file->sections+symb->st_shndx,file->symbolNameTable),targetSection))){
            cFunc->name = getSymbolName(symb,file->symbolNameTable);
            cFunc->bind = getSymbolBind(symb);
            cFunc->value = (void*)symb->st_value;
            cFunc++;
        }
    }
    return c;
}

int loadSection(FILE* fp, ELFSectionHeader* section,void** buffer){
    fseek(fp,section->sh_offset,SEEK_SET);
    *buffer = malloc(section->sh_size);
    fread(*buffer,section->sh_size,1,fp);
    return section->sh_size;
}