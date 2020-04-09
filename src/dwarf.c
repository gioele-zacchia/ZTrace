#include "../ZTR/dwarf.h"


void DWARFResetState(DWARFLineState* state, DWARFLineHeader* header){
    state->address = (void*)header->base_address;
    state->file = 1;
    state->line = 1;
    state->column = 0;
    state->flags = header->default_is_stmt?is_stmt:0;
    state->isa = 0;
    state->header = header;
}

DWARFLineState* getDefaultState(DWARFLineHeader* header){
    DWARFLineState* state = malloc(sizeof(DWARFLineState));
    DWARFResetState(state,header);
    return state;
}

void DWARFPushState(DWARFLineState* state,AVLNode** root){
    #ifdef DEBUG_MODE
        printf("Saving state\n");
    #endif
    DWARFLineState* nState = malloc(sizeof(DWARFLineState));
    memcpy(nState,state,sizeof(DWARFLineState));
    //BSTAppend(root,nState,nState->address);
    AVLInsertInPlace(root,(unsigned long long)nState->address,nState);
}

size_t DWARFExecuteOpcode(uint8_t* opcode, DWARFLineHeader* header, DWARFLineState* state,AVLNode** lineTree){
    if(*opcode == 0x0){
        size_t offset = 1;
        size_t pOffset = 0;
        uint32_t operationSize = decodeULEB128(opcode+offset,&pOffset);
        offset += pOffset; 
        if(operationSize == 0){
            fprintf(stderr,"Erorr decording extended opcode at address: %p\n",opcode);
            return 0;
        }
        uint8_t ext_opcode = *(opcode+offset);
        offset++;

        uint32_t operands[30];
        int i = 0; 
        while(offset < (operationSize+pOffset+1)){
            if(i >= 30){
                fprintf(stderr,"Operand size overflow\n");
                return 0;
            }
            size_t tOffset = 0;
            operands[i] = decodeULEB128(opcode+offset,&tOffset);
            offset += tOffset;
            i++;
        }

        if(ext_opcode == DW_LNE_end_sequence){
            #ifdef DEBUG_MODE
                printf("Opcode DW_LNE_end_sequence\n");
            #endif
            state->flags |= end_sequence;
            DWARFPushState(state,lineTree);
            DWARFResetState(state,header);
            
        }else if(ext_opcode == DW_LNE_set_address){
            uint64_t address = 0;
            uint8_t* ptr = (uint8_t*)&address;
            for(int i = 0; i <sizeof(void*);i++){
                *ptr = (uint8_t)(operands[i]&0xFF);
                ptr++;
            }
            #ifdef DEBUG_MODE
                printf("Opcode DW_LNE_set_address, Address: %p\n",address);
            #endif
            state->address = (void*)(address + header->base_address);
        }else if(ext_opcode == DW_LNE_define_file){
            #ifdef DEBUG_MODE
                printf("Opcode DW_LNE_define_file, not yet implemented\n");
            #endif
            printf("Opcode DW_LNE_define_file, not yet implemented\n");
            return 0;
        }else{
            #ifdef DEBUG_MODE
                printf("Cannot decode extend opcode %d at address %p\n",ext_opcode,opcode);
            #endif
            return offset;
        }
        return offset;
    }
    if(*opcode >= header->opcode_base){
        int32_t adjsOcode = *opcode - header->opcode_base;
        int32_t addressIncrement = (adjsOcode/header->line_range)*header->minimum_instruction_length;
        int32_t lineIncrement =(adjsOcode%header->line_range) + header->line_base;
        #ifdef DEBUG_MODE
            printf("Spcial opcode : %d Address increment: %d line Increment: %d\n",*opcode,addressIncrement,lineIncrement);
        #endif
        state->address += addressIncrement;
        state->line += lineIncrement;
        DWARFPushState(state,lineTree);
        state->flags &= ~basic_block;
        state->flags &= ~prologue_end;
        state->flags &= ~epilogue_end;
        return 1;
    }
    uint32_t operands[10];
    size_t offset = 1;
    for(int i = 0; i < header->standard_opcode_lenghts[*opcode-1]; i++){
        if(i >= 10){
            fprintf(stderr,"Operand size overflow\n");
            return 0 ;
        }
        size_t tOffset = 0;
        operands[i] = decodeULEB128(opcode+offset,&tOffset);
        offset += tOffset; 
    }
    if(*opcode == DW_LNS_copy){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_copy \n");
         #endif
        DWARFPushState(state,lineTree);
        state->flags &= ~basic_block;
        state->flags &= ~prologue_end;
        state->flags &= ~epilogue_end;
    }
    else if(*opcode == DW_LNS_advance_pc){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_advance_pc, afvancing by %d\n",operands[0]*header->minimum_instruction_length);
         #endif
        state->address += operands[0]*header->minimum_instruction_length;
    }
    else if(*opcode == DW_LNS_advance_line){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_advance_line, andancing of%d\n",operands[0]);
        #endif
        state->line += operands[0];
    }
    else if(*opcode == DW_LNS_set_file){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_file, new file: %d\n",operands[0]);
        #endif
        state->file = operands[0];
    }
    else if(*opcode == DW_LNS_set_column){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_column, column: %d\n",operands[0]);
         #endif
        state->column = operands[0];
    }
    else if(*opcode == DW_LNS_negate_stmt){
        state->flags ^= is_stmt;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_negate_stmt, new is_stmt: %d\n",(state->flags&is_stmt != 0));
        #endif
    }
    else if(*opcode == DW_LNS_set_basic_block){
        state->flags |= basic_block;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_basic_block, new basci_block %d\n",(state->flags&basic_block != 0));
         #endif
    }
    else if(*opcode == DW_LNS_const_add_pc){
        int32_t adjsOcode = 0xFF - header->opcode_base;
        int32_t addressIncrement = (adjsOcode/header->line_range)*header->minimum_instruction_length;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_const_add_pc, incrementing address by 0x%x\n",addressIncrement);
        #endif
        state->address+=addressIncrement;
    }
    else if(*opcode == DW_LNS_fixed_advance_pc){
        uint16_t operand = *(opcode+1);
        offset = 3;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_fixed_advance_pc, Increment by %sd\n",operand);
         #endif
        state->address += operand;
    }
    else if(*opcode == DW_LNS_set_prologue_end){
        state->flags |= prologue_end;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_prologue_end, new prologue_end %d\n",(state->flags&prologue_end != 0));
         #endif
    }
    else if(*opcode == DW_LNS_set_epilogue_begin){
        state->flags |= epilogue_end;
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_epilogue_begin, new epilogue_end %d\n",(state->flags&epilogue_end != 0));
         #endif
    }
    else if(*opcode == DW_LNS_set_isa){
        #ifdef DEBUG_MODE
            printf("Opcode DW_LNS_set_isa, new ISA: %d\n",operands[0]);
         #endif
        state->isa = operands[0];
    }else{
        #ifdef DEBUG_MODE
            printf("Invalid opcode: %sx, at address: %p\n",*opcode,opcode);
         #endif
        return 0;
    }
    return offset;

}




void DWARFPrintState(DWARFLineState* state){
    printf("------------------\n");
    printf("Address: %p file:%d\n",state->address,state->file);
    printf("Line: %d column:%d\n",state->line,state->column);
    printf("Falgs: %x Isa:%d\n",state->flags,state->isa);
    printf("------------------\n");
}

AVLNode* DWARFDecode(DWARFLineHeader* header, void* data,AVLNode* tree){
    DWARFLineState* state = getDefaultState(header);
    void* pc = data + header->program_start_offset;
    size_t offset;
    while(pc < (data+header->unit_lenght+sizeof(uint32_t))){
        if((offset = DWARFExecuteOpcode(pc,header,state,&tree))){
            pc += offset;
        }else{
            fprintf(stderr,"DWARF Opcode execution failed\n");
            return tree;
        }
        #ifdef DEBUG_MODE
            DWARFPrintState(state);
        #endif
    }
    free(state);
    return tree;

}



DWARFLineHeader* parseDebugHeader(void* rawData, int len){
    DWARFLineHeader* header = malloc(sizeof(DWARFLineHeader));
    FILE* fr = fmemopen(rawData,len,"r");
    fread(header,sizeof(uint32_t),1,fr);
    //int dataType = DWARF_32;
    if(header->unit_lenght == 0xffffffff){
        //dataType = DWARF_64;
        fread(header,offsetof(DWARFLineHeader,header_lenght),1,fr);
    }else{
        uint16_t version;
        uint32_t header_lenght;
        fread(&version,sizeof(uint16_t),1,fr);
        fread(&header_lenght,sizeof(uint32_t),1,fr);
        header->version = version;
        header->header_lenght = header_lenght;
    }
    DWARFHeaderNOPad* mid = malloc(sizeof(DWARFHeaderNOPad));
    fread(mid,sizeof(DWARFHeaderNOPad),1,fr);

    header->minimum_instruction_length = mid->minimum_instruction_length;
    header->default_is_stmt = mid->default_is_stmt;
    header->line_base = mid->line_base;
    header->line_range = mid->line_range;
    header->opcode_base = mid->opcode_base;

    free(mid);

    //fread(&(header->minimum_instruction_length),offsetof(DWARFLineHeader,opcode_base)-offsetof(DWARFLineHeader,header_lenght),1,fr);
    //printf("Unint lenght: %d Version: %d, Header Lenght: %ld\n",header->unit_lenght,header->version,header->header_lenght);
    //printf("minimum_instruction_length: %d default_is_stmt: %d, line_base: %d\n",header->minimum_instruction_length,header->default_is_stmt,header->line_base);
    header->standard_opcode_lenghts = malloc(sizeof(uint8_t)*(header->opcode_base-1));
    uint8_t* t = header->standard_opcode_lenghts;
    for(int i = 0; i < (header->opcode_base-1); i++){
        *t = fgetc(fr);
        t++;
    }
    //printf("Current offset: %d\n",ftell(fr));
    char* baseStrAddr = rawData + ftell(fr);
    int i = 0;
    while(1){
        if(*baseStrAddr == 0){
            i++;
            if(*(baseStrAddr+1)==0){
                break;
            }
        }
        baseStrAddr++;
    }
    header->directories_num = i;
    header->include_directories = malloc(sizeof(char*)*header->directories_num);
    //printf("Num strings: %d\n",i);
    baseStrAddr = rawData + ftell(fr);
    for(int i = 0; i < header->directories_num; i++){
        size_t cStrLen = strlen(baseStrAddr);
        header->include_directories[i] = malloc(sizeof(char)*(cStrLen+1));
        strcpy(header->include_directories[i],baseStrAddr);
        baseStrAddr += cStrLen+1;
    }
    
    DWARFFileNode* head = NULL;
    DWARFFileNode* currentNode = NULL;

    void* cPointer = baseStrAddr+1;
    int itemCount = 0;
    while(1){
        if(*(char*)cPointer == 0x0){
            break;
        }
        int nameLen = strlen(cPointer);
        DWARFFile* cFile = malloc(sizeof(DWARFFile));
        cFile->name = malloc(sizeof(char)*(nameLen+1));
        memcpy(cFile->name,cPointer,nameLen+1);
        cPointer += nameLen+1;
        size_t offset = 0;
        cFile->directoryIndex = decodeULEB128(cPointer,&offset);
        cPointer += offset;
        cFile->lastModify  = decodeULEB128(cPointer,&offset);
        cPointer += offset; 
        cFile->len = decodeULEB128(cPointer,&offset);
        cPointer += offset; 

        DWARFFileNode* cNode = malloc(sizeof(DWARFFileNode));
        cNode->info = cFile;
        cNode->next = NULL;

        if(head == NULL){
            head = cNode;
            currentNode = cNode;
        }else{
            currentNode->next = cNode;
            currentNode = cNode;
        }
        itemCount++;
    }
    header->file_num = itemCount;
    header->file_names = malloc(sizeof(DWARFFile*)*itemCount);
    for(int i = 0; i < itemCount; i++){
        header->file_names[i] = head->info;
        DWARFFileNode* t = head;
        head = head->next;
        free(t);
    }

    header->program_start_offset = (cPointer+1) - rawData;
    header->base_address = 0x0;
    fclose(fr);
    return header;
}

void freeDWARFLineHeader(DWARFLineHeader* header){
    for(int i = 0; i < header->directories_num; i++){
        free(header->include_directories[i]);
    }
    free(header->include_directories);
    for(int i = 0; i < header->file_num; i++){
        free(header->file_names[i]->name);
        free(header->file_names[i]);
    }
    free(header->file_names);
    free(header);
}

void dumbPrint(AVLNode* tree){
    if(tree->left != NULL){
        dumbPrint(tree->right);
    }
    DWARFPrintState(tree->content);
    free(tree->content);
    if(tree->right != NULL){
        dumbPrint(tree->right);
    }
}

void DWARFFreeArray(DWARFHeaderNode* node){
    if(node){
        DWARFFreeArray(node->next);
        free(node);
    }
}

void printLineInfoToFile(DWARFLineState* state,FILE *fp){
    fprintf(fp,"%s: [%d:%d] - %p\n",state->header->file_names[state->file-1]->name,state->line,state->column,state->address);
}

void printTreeToFile(AVLNode* tree,FILE* fp){
    if(tree->left){
        printTreeToFile( tree->left,fp);
    }
    printLineInfoToFile(tree->content,fp);
    if(tree->right){
        printTreeToFile(tree->right,fp);
    }
    return;
}

DWARFDebugInfo* DWARFGetDebugInfo(ELFFile* elf,FILE* fp,void* baseAddr){
    ELFSectionHeader* debugLineSection = NULL;
    for(int i = 0; i < elf->sectionNum; i++){
        if(strcmp(getSectionName(elf->sections+i,elf->sectionNameTable),DEBUG_LINE_SECTION) == 0){
            debugLineSection = elf->sections+i;
            break;
        }
    }
    if(debugLineSection == NULL){
        fprintf(stderr,"\e[35mCannot find debug line section. Did you compile with -g?\n\e[0m");
        return NULL;
    }
    //printf("Found debug section: %p\n",debugLineSection);
    void* rawDebugData;
    int debugDataLen = loadSection(fp,debugLineSection,&rawDebugData);
    int cOffset = 0;
    //int i = 0;

    DWARFDebugInfo* ret = malloc(sizeof(DWARFDebugInfo));

    ret->headers = NULL;
    ret->lineTree = NULL;

    DWARFHeaderNode* cHeader = ret->headers;
    while(cOffset < debugDataLen){
            #ifdef DEBUG_MODE
                printf("Analyzing debug header %d at offset: %d\n",i,cOffset);
            #endif
            DWARFLineHeader* dHeader = parseDebugHeader(rawDebugData+cOffset,debugDataLen);
            dHeader->base_address = (uint64_t)baseAddr;
            if(ret->headers == NULL){
                ret->headers = malloc(sizeof(DWARFHeaderNode));
                ret->headers->header = dHeader;
                cHeader = ret->headers;
            }else{
                DWARFHeaderNode* t = malloc(sizeof(DWARFHeaderNode));
                cHeader->next = t;
                cHeader = t; 
            }
            #ifdef DEBUG_MODE
                printf("Start offset: %d, header len:%d\n",dHeader->program_start_offset,dHeader->header_lenght);
            #endif

            ret->lineTree = DWARFDecode(dHeader,rawDebugData+cOffset,ret->lineTree);
            cOffset += dHeader->unit_lenght+4;

    }
    return ret;

}

void DWARFClean(DWARFDebugInfo* info){
    AVLFree(info->lineTree,1);
    DWARFFreeArray(info->headers);
    free(info);
}

char* DWARFGetFileName(DWARFLineState* state){
    return state->header->file_names[state->file-1]->name;
}

