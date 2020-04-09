#ifndef __PROCMEM_H
#define __PROCMEM_H
enum mapPers{
    READ = 1,
    WRITE = 2,
    EXECUTE = 4,
    SHARED = 8,
    PRIVATE = 16
};

typedef struct processMapping{
    void* startAddress;
    void* endAddress;
    enum mapPers perms;
    unsigned long offset;
    unsigned char majorDev;
    unsigned char minorDev;
    unsigned long inode;
    char* path;
}processMapping;

enum mapPers parsePerm(char perm);
void printMapping(processMapping* map);
int readMappings(processMapping** out);
char* getRealExePath();
#endif