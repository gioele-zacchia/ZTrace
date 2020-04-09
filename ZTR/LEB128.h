#ifndef __LEB128__H
#define __LEB128__H

#include <stdint.h>
#include <stdio.h>

uint32_t decodeULEB128(uint8_t* data,size_t* offset);
void testLEB();

#endif