#include "../ZTR/LEB128.h"

uint32_t decodeULEB128(uint8_t* data,size_t* offset){
    *offset = 1;
    uint32_t result = 0;
    int shift = 0;
    while (1) {
        uint8_t byte = *data;
        result |= (byte&0x7F) << shift;
        if ((byte&0x80) == 0)
            break;
        shift += 7;
        data++;
        *offset = *offset + 1;
    }
    return result;
}

/*void testLEB(){
    uint8_t buffer[] = {2,127,0x80,1,1+0x80,1,2+0x80,1,57+0x80,100};
    uint32_t results[] = {2,127,128,129,130,12857};
    long unsigned int offset = 0;
    int i = 1;
    uint32_t result = decodeULEB128(buffer,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;
    result =decodeULEB128(buffer+1,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;
    result =decodeULEB128(buffer+2,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;
    result = decodeULEB128(buffer+4,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;
    result =decodeULEB128(buffer+6,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;
    result =decodeULEB128(buffer+8,&offset);
    printf("Test: %d, expected: %d, got:%d offset:%d\n",i,results[i-1],result,offset);
    i++;

}*/