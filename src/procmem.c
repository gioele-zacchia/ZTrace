#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../ZTR/procmem.h"


enum mapPers parsePerm(char perm){
    switch(perm){
        case 'r':
            return READ;
        case 'w':
            return WRITE;
        case 'x':
            return EXECUTE;
        case 's':
            return SHARED;
        case 'p':
            return PRIVATE;
    }
    return 0;
}

void printMapping(processMapping* map){
    printf("Start address: %p\n",map->startAddress);
    printf("End address: %p\n",map->endAddress);
    printf("Perms: %d\n",map->perms);
    printf("Offset: %lx\n",map->offset);
    printf("Dev: %d:%d\n",map->majorDev,map->minorDev);
    printf("Inode: %lu\n",map->inode);
    printf("Path: %s\n",map->path);

}

int readMappings(processMapping** out){
    FILE* fp = fopen("/proc/self/maps","r");
    int numMaps = 0;
    for (char c = getc(fp); c != EOF; c = getc(fp)){
        //putchar(c); 
        if (c == '\n') 
            numMaps++;
    }
    *out = malloc(sizeof(processMapping)*numMaps);
    processMapping* target= *out;
    rewind(fp);
    for(int i = 0; i < numMaps; i++){
        char a,b,c,d;
        fscanf(fp,"%p-%p %[rwxsp-]%[rwxsp-]%[rwxsp-]%[rwxsp-]",
            &(target->startAddress),
            &(target->endAddress),
            &a,
            &b,
            &c,
            &d);
        target->perms = parsePerm(a);
        target->perms |= parsePerm(b);
        target->perms |= parsePerm(c);
        target->perms |= parsePerm(d);
        //printf("Persms: %c%c%c%c\n",a,b,c,d);
        fscanf(fp," %lx %d:%d %lu",
            &(target->offset),
            (int*)&(target->majorDev),
            (int*)&(target->minorDev),
            &(target->inode));
        int i = 0;
        for(fscanf(fp,"%c",&c);c != '\n';c = getc(fp)){
            if(isspace(c)){
                continue;
            }
            i++;
        }
        fseek(fp,-i-1,SEEK_CUR);
        target->path = malloc(sizeof(char)*(i+1));
        i = 0;
        for(fscanf(fp,"%c",&c);c != '\n';c = getc(fp)){
            if(isspace(c)){
                continue;
            }
            target->path[i] = c;
            i++;
        }
        target->path[i] = '\0';
        target++;
    }
    fclose(fp);
    return numMaps;
}

char* getRealExePath(){
    char* buffer = malloc(sizeof(char)*1024);
    int cread;
    if((cread = readlink("/proc/self/exe", buffer, 1024)) < 0){
        fprintf(stderr,"Cannot read real process path\n");
        exit(-1);
    }
    buffer[cread] = '\0';
    return buffer;
}