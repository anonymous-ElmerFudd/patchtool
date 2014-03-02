// Copyright 2010       anonymous
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt





#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "stdint.h"
#include "data_funcs.h"
#include "file_functions.h"
#include "common_defs.h"



/// GLOBALS DECLARATIONS
extern uint8_t g_bDebugModeEnabled;
extern uint8_t g_bMultiPatchEnabled;
extern uint8_t g_bPatchingEnabled;




// func for find the specific 'search' pattern in a data buffer
int find_data_in_buffer(u8* pInBuffer, uint32_t dwSizeOfBuffer, char* pszSearchString, char* pszMaskString, char* pszReplaceString, uint32_t dwReplaceOffset)
{		
	u8* pCurrBuffPtr = NULL;
	u8* pSearchPattern = NULL;
	u8* pReplacePattern = NULL;	
	u8* pMaskPattern = NULL;
	uint32_t* pdwSearchBlock = NULL;
	uint32_t* pdwDataBlock = NULL;
	uint32_t* pdwMaskBlock = NULL;
	uint32_t dwSizeOfSearchString = 0;
	uint32_t dwSizeOfMaskString = 0;
	uint32_t dwSizeOfReplaceString = 0;
	uint32_t dwSizeOfSearchData = 0;
	uint32_t dwSizeOfReplaceData = 0;	
	uint32_t num_blocks = 0;
	uint32_t matches_found = 0;	
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t dwPadd = 0;
	int code_matches_found = 0;
	int retval = -1;

	


	// validate input params
	if ( (pInBuffer == NULL) || (dwSizeOfBuffer == 0) || (pszSearchString == NULL) || (pszMaskString == NULL) || (pszReplaceString == NULL) ) {
		printf("\nERROR!  Invalid input parameters, exiting!\n");
		goto exit;
	}


	__try 
	{
		// get the lengths of the 'search' and 'replace' strings
		dwSizeOfSearchString = strlen(pszSearchString);
		dwSizeOfMaskString = strlen(pszMaskString);
		dwSizeOfReplaceString = strlen(pszReplaceString);
		if ( (dwSizeOfSearchString == 0) || (dwSizeOfMaskString == 0) || ((dwSizeOfReplaceString < 1) && g_bPatchingEnabled == TRUE) ) {
			printf("\nERROR!  Invalid input parameters, exiting!\n");
			__leave;
		}

		// search and data masks MUST be same size
		if ( dwSizeOfSearchString != dwSizeOfMaskString ) {
			printf("\nERROR!  Search/Replace patterns are not the same size, exiting!!\n");
			__leave;
		}

		// calculate the actual size of the searchstring
		// converted to binary data (ie divide stringsize / 2)
		dwSizeOfSearchData = dwSizeOfSearchString / 2;
		dwSizeOfReplaceData = dwSizeOfReplaceString / 2;
		num_blocks = dwSizeOfSearchData / DATA_PATTERN_ALIGNMENT;

		// if our data is 'mis-aligned', tack on the extra 'search' byte(s) (CC)
		// and 'mask' byte(s) (00) needed to align at 32-bits, 
		// and bump up the 'num_blocks' by 1
		if ( (dwSizeOfSearchData % DATA_PATTERN_ALIGNMENT > 0) ) {
			dwPadd = (DATA_PATTERN_ALIGNMENT - (dwSizeOfSearchData % DATA_PATTERN_ALIGNMENT));
			for (i = 0; i < dwPadd; i++) {
				strcat_s(pszSearchString, MAX_HEXSTRINGS_LENGTH, "CC");
				strcat_s(pszMaskString, MAX_HEXSTRINGS_LENGTH, "00");
			}
			num_blocks+=1;
		}

		// convert the ASCII string to binary buffers
		pSearchPattern = _x_to_u8_buffer(pszSearchString);
		pMaskPattern = _x_to_u8_buffer(pszMaskString);
		pReplacePattern = _x_to_u8_buffer(pszReplaceString);
		if ( (pReplacePattern == NULL) || (pMaskPattern == NULL) || (pReplacePattern == NULL) ) {
			printf("\nERROR!  Failed to convert strings to binary data, exiting!\n");
			retval = -1;
			__leave;
		}

		// iterate through the buffer, searching for the pattern
		// (search range is our 'buffer size' - 'search string len')
		pCurrBuffPtr = pInBuffer;		
		for (i = 0; i < (dwSizeOfBuffer - dwSizeOfSearchData); i++ )
		{
			// iterate through the 32-bit 'chunks', AND
			// off the searc/data with the mask, and XOR
			// them to see if block(s) match
			matches_found = 0;
			for (j = 0; j < num_blocks; j++) {
				pdwSearchBlock = (uint32_t*)(pSearchPattern+(sizeof(uint32_t)*j));
				pdwDataBlock = (uint32_t*)(pCurrBuffPtr+(sizeof(uint32_t)*j));
				pdwMaskBlock = (uint32_t*)(pMaskPattern+(sizeof(uint32_t)*j));				
				if ( ((*pdwSearchBlock ^ *pdwDataBlock) & *pdwMaskBlock) == 0 ) 			
					matches_found++;
				else
					break;
			}			
			// if we found all matches, then 
			// break out, success!!
			if (matches_found == num_blocks)
			{	
				code_matches_found++;										
				if ( (g_bMultiPatchEnabled == FALSE) && (code_matches_found > 1) ) {
					printf("!ERROR! Multiple matches found!, exiting!\n");					
					retval = -1;
					__leave;
				}
				// if 'patching' is enabled, then copy patch to buffer, at ptr+offset
				// (verify that 'replace offset' is not beyond the buffer size!)				
				if (g_bPatchingEnabled == TRUE) {
					if ( (dwReplaceOffset+i) > dwSizeOfBuffer ) {
						printf("!ERROR! Replace Offset is out of range!!!, exiting!\n");					
						retval = -1;
						__leave;
					}
					memcpy((pCurrBuffPtr+dwReplaceOffset), pReplacePattern, dwSizeOfReplaceData);
					printf("----PATCHED AT:%.8X\n", (i+dwReplaceOffset));			
				} 
				else {
					printf("----FOUND MATCH AT:%.8X\n", (i+dwReplaceOffset));						
				}
				// status success
				retval = STATUS_SUCCESS;
			}
			pCurrBuffPtr++;
		}		
	} // end __try{}

	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		printf("\n!!ERROR!! find_data_in_buffer threw exception!!\n");
		retval = -1;
	}	

exit:
	// free any alloc'd memory
	if (pSearchPattern != NULL)
		free(pSearchPattern);

	// free any alloc'd memory
	if (pMaskPattern != NULL)
		free(pMaskPattern);

	// free any alloc'd memory
	if (pReplacePattern != NULL)
		free(pReplacePattern);
	
	return retval;
}
/**/
/**********************************************************************************************************/


u64 _x_to_u64(const s8 *hex)
{
	u64 t = 0, res = 0;
	u32 len = strlen(hex);
	char c;

	while(len--)
	{
		c = *hex++;
		if(c >= '0' && c <= '9')
			t = c - '0';
		else if(c >= 'a' && c <= 'f')
			t = c - 'a' + 10;
		else if(c >= 'A' && c <= 'F')
			t = c - 'A' + 10;
		else
			t = 0;
		res |= t << (len * 4);
	}

	return res;
}

u8 *_x_to_u8_buffer(const s8 *hex)
{
	u32 len = strlen(hex);
	s8 xtmp[3] = {0, 0, 0};

	//Must be aligned to 2.
	if(len % 2 != 0)
		return NULL;

	u8 *res = (u8 *)malloc(sizeof(u8) * len);
	u8 *ptr = res;

	while(len--)
	{
		xtmp[0] = *hex++;
		xtmp[1] = *hex++;

		*ptr++ = (u8)_x_to_u64(xtmp);
	}

	return res;
}