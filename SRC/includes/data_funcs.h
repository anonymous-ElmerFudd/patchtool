// Copyright 2010            anonymous
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef _DATAFUNCS_H_
#define _DATAFUNCS_H_


#include "stdint.h"
#include "common_defs.h"


#define MAX_BINARYPATTERN_LENGTH		0x1000												// 4096 bytes
#define MAX_HEXSTRINGS_LENGTH			(MAX_BINARYPATTERN_LENGTH + sizeof(uint32_t) + 1)	// add extra length for mis-alignment 'padding' (if req'd)
#define DATA_PATTERN_ALIGNMENT			sizeof(uint32_t)									// curr. mask alignment is 32-bits (4 bytes)



#ifdef __cplusplus
extern "C" {
#endif



// find binary pattern in buffer
int find_data_in_buffer(u8* pInBuffer, uint32_t dwSizeOfBuffer, char* pszSearchString, char* pszMaskString, char* pszReplaceString, uint32_t dwReplaceOffset);
u64 _x_to_u64(const s8 *hex);
u8 *_x_to_u8_buffer(const s8 *hex);




#ifdef __cplusplus
}
#endif


#endif
// _DATAFUNCS_H_
