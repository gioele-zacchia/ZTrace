#include "../ZTR/ZTrace.h"


void printToFiles(SymbolMapEntity* map){
    SymbolMapEntity* cVal = map;
    char tFile[2014];
    int i = 0;
    while(cVal){
        sprintf(tFile,"debugData_%d.csv",i);
        FILE* fp = fopen(tFile,"w");
        fprintf(fp,"FILE: %s\n",cVal->fileName);
        for(int c = 0; c < cVal->programFunctionSymbolsNum; c++){
            fprintf(fp,"%s,%p\n",cVal->functionSymbols[c].name,cVal->functionSymbols[c].value);
        }
        if(cVal->debugInfo){
                sprintf(tFile,"DwarfData_%d.csv",i);
                FILE* fp = fopen(tFile,"w");
                printTreeToFile(cVal->debugInfo->lineTree,fp);
                fclose(fp);
        }
        fclose(fp);
        i++;
        cVal = cVal->next;
    }
}


SymbolMapEntity* getMapFile(void* addr,SymbolMapEntity* root){
    if(root == NULL){
        return NULL;
    }
    uint64_t aT,aB;

    if(BUILD_64){
        aT = (uint64_t)addr&0xFF0000000000;
        aB = (uint64_t)root->baseAddress&0xFF0000000000;
    }else{
        aT = (uint64_t)addr&0xFF<<(3*8);
        aB = (uint64_t)root->baseAddress&0xFF<<(3*8);
    }
    if(aT == aB){
        return root;
    }else{
        return getMapFile(addr,root->next);
    }

}


void printTrace(void* addr){
    SymbolMapEntity* entity = getMapFile(addr,ZTsymbolMap);
    if(entity == NULL){
        fprintf(stderr,"    Call to unknown library [Address:%p]\n",addr);
        return;
    }
    uint64_t cBest = UINT64_MAX;
    ELFFunctionSymbol* cFunc = NULL;
    for(int i = 0; i < entity->programFunctionSymbolsNum; i++){
        if((addr - entity->functionSymbols[i].value)>0 && (addr -  entity->functionSymbols[i].value)<cBest){
            cBest = addr - entity->functionSymbols[i].value;
            cFunc = &entity->functionSymbols[i];
        }
    }
    if(cFunc == NULL || strlen(cFunc->name) < 1){
        if(entity->isSelfproc){
            fprintf(stderr,"    Unknown function call to file \e[1;33m%s\e[0m [Address: %p]\n",entity->fileName,addr);
        }else{
            fprintf(stderr,"    Unknown function call to library \e[1;33m%s\e[0m [Address: %p]\n",entity->fileName,addr);
        }
        return;
    }
    if(entity->isSelfproc && entity->debugInfo){
        DWARFLineState* debugInfo = AVLFindClosest(entity->debugInfo->lineTree,(unsigned long long)addr);
        char* fName = cFunc->name;
        uint64_t offset = addr-cFunc->value;
        char* fileName = DWARFGetFileName(debugInfo);
        uint32_t line = debugInfo->line;
        fprintf(stderr,"    Function call to \e[31m%s+0x%lx\e[0m \e[1;33m(%s:%d)\e[0m [Address:%p]\n",fName,offset,fileName,line,addr);
        //fprintf(stderr,"    Function call to &s+0x%x (%s:%d) [Address:%p]\n",cFunc->name,addr - cFunc->value,DWARFGetFileName(debugInfo),debugInfo->line,addr);
        return;
    }
    if(entity->isSelfproc){
        fprintf(stderr,"    Function call to \e[31m%s+0x%lx\e[0m [Address: %p]\n",cFunc->name,addr - cFunc->value,addr);
    }else{
        fprintf(stderr,"    Library call to library \e[1;33m%s\e[0m : \e[31m%s+0x%lx\e[0m [Address: %p]\n",entity->fileName,cFunc->name,addr-cFunc->value,addr);
    }
}


void sigSEGVHandler(int signum, siginfo_t *info, void *ptr){
    //printf("Ho acchiappato il segfault\n");
    fprintf(stderr,"\e[1;35mSegfault for addr: %p\e[0m\n",info->si_addr);
    if(info->si_code == SEGV_MAPERR){
        fprintf(stderr,"Address not mapped to object\n");
    }else if(info->si_code == SEGV_ACCERR){
        fprintf(stderr,"Invalid permissions for mapped object\n");
    }else if(info->si_code == SEGV_BNDERR){
        fprintf(stderr,"Failed address bound checks\n");
    }else if(info->si_code == SEGV_PKUERR){
        fprintf(stderr,"Access was denied by memory protection keys\n");
    }else{
        fprintf(stderr,"Unknow code %d\n",info->si_code);
    } 
    void *bt[1024];
    int bt_size = backtrace(bt, 1024);
    for(int i = 2; i < bt_size; i++){
        printTrace(bt[i]);
    }
    fprintf(stderr,"End stack trace\n");
    signal(signum,SIG_DFL);
    raise(signum);
}


void installSignalHandler(){
    struct sigaction* act;
    //struct sigaction* oldAct;
    act = malloc(sizeof(sigaction));
    memset(act,0,sizeof(sigaction));

    act->sa_sigaction = sigSEGVHandler;
    act->sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV,act,NULL);
}


/*int getFunctionSymbolsFromPath(char* path, ELFFunctionSymbol** target, DWARFDebugInfo** debug){
    printf("TODO! Load symbols for: %s\n",path);
}*/

void insertDebugInfo(SymbolMapEntity* entity,char* procPath){
    //printf("Loading debug info for file: %s, at address: %p\n",entity->fileName,entity->baseAddress);
    FILE* fp = fopen(entity->fileName,"r");
    ELFFile* elf = readElf(fp);
    
    entity->programFunctionSymbolsNum = getFunctiosForSection(elf,".text",&entity->functionSymbols);
    for(int i = 0; i < entity->programFunctionSymbolsNum; i++){
        ELFFunctionSymbol* cSymb = entity->functionSymbols +i;
        cSymb->value = (void*)(cSymb->value+(uint64_t)entity->baseAddress);
    }

    if(strcmp(procPath,entity->fileName) == 0){
        entity->debugInfo = DWARFGetDebugInfo(elf,fp,entity->baseAddress);
        entity->isSelfproc = true;
    }else{
        entity->debugInfo = NULL;
        entity->isSelfproc = false;
    }

    entity->elf = elf;

    fclose(fp);
    
}

SymbolMapEntity* createSymbolMap(processMapping* _map, int mappingLen,char* procPath){
    SymbolMapEntity* head=NULL;
    for(int i = 0;i < mappingLen; i++){
        processMapping* map = _map +i;
        if(map->path[0] != '/') continue;
        if(head == NULL){
            head = malloc(sizeof(SymbolMapEntity));
            head->baseAddress = map->startAddress;
            head->fileName = map->path;
            head->next = NULL;
            insertDebugInfo(head,procPath);
            continue;
        }
        SymbolMapEntity* cEnt = NULL;
        bool foundSelf = false;
        do{
            if(cEnt){
                cEnt = cEnt->next;
            }else{
                cEnt = head;
            }
            if(strcmp(cEnt->fileName,map->path) == 0){
                foundSelf = true;
                break;
            }
        }while(cEnt->next);
        if(!foundSelf){
            SymbolMapEntity*t = cEnt->next;
            cEnt->next = malloc(sizeof(SymbolMapEntity));
            cEnt->next->baseAddress = map->startAddress;
            cEnt->next->fileName = map->path;
            insertDebugInfo(cEnt->next,procPath);
            cEnt->next->next = t;
        }
    }
    
    return head;

}

void ZTInstall(){
    processMapping* memMap;
    int mappingLen = readMappings(&memMap);

    char* procPath = getRealExePath();

    SymbolMapEntity* symbolMaps = createSymbolMap(memMap,mappingLen,procPath);


    ZTProcMap = memMap;
    ZTProcessMappingLen = mappingLen;

    free(procPath);

    ZTsymbolMap = symbolMaps;
    installSignalHandler();
    #ifdef DEBUG_MODE
        printToFiles(ZTsymbolMap);
    #endif
}


void ZTClean(){
    signal(SIGSEGV,NULL);
    SymbolMapEntity* entity = ZTsymbolMap;
    while(entity){
        if(entity->debugInfo){
            DWARFClean(entity->debugInfo);
        }
        free(entity->functionSymbols);
        freeELF(entity->elf);
        SymbolMapEntity* l = entity;
        entity = entity->next;
        free(l);
    }
    for(int i =0 ; i < ZTProcessMappingLen; i++){
        free(ZTProcMap[i].path);
    }
    free(ZTProcMap);
}
