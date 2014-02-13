// Copyright 2010       anonymous
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
//
//
//
//
//  v1.0.0.0 -- initial release
//
//        


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include "common_defs.h"
#include "data_funcs.h"
#include "file_functions.h"




/////////////////////////////////////////
/// update for any changes to this code
#define PATCHTOOL_VERSION	"1.0.0.0"
/////////////////////////////////////////

///#define TOOL_DEBUG_ENABLED 1



/// *** GLOBAL DECLARATIONS ***  ///
uint8_t g_bDebugModeEnabled = FALSE;
uint8_t g_bMultiPatchEnabled = FALSE;
uint8_t g_bPatchingEnabled = TRUE;


// internal function declars
int select_string(char* pszInString);
void usage (char* pszInParam);



///////////////////////////////////////////////////////////////
/// select_string function ///////
int select_string(char* pszInString)
{
	int i = 0;
	int ret = -1;
	char* params_list[] = {
		"-action",
		"-filename",
		"-search",
		"-replace",		
		"-mask",
		"-offset",
		"-multi",
		"-debug"
	};

	// for loop to iterate through params list
	for (i = 0; i < (sizeof(params_list)/sizeof*params_list); i++)
	{
		if ( strcmp(pszInString, params_list[i]) == 0 ) {
			ret = i;
			break;
		}
	}

	return ret;
}
//
/////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//// usage function ///////
void usage (char* pszInParam)
{
	printf("\n\n************************************************\n\n");
	printf("PATCHTOOL " PATCHTOOL_VERSION " (C) 2014 by anonymous\n\n");	
	printf("************************************************\n\n");

	// display the invalid param (if specified)
	if (pszInParam != NULL)
		printf("\nParameter: \"%s\" is invalid!\n\n", pszInParam);

	printf("Usage:  PATCHTOOL:  -debug  -action  -filename  -search  -mask  -replace  \n\t\t    -offset  -multi\n\n");
	printf("\texample: <patchtool.exe  -action patch -filename lv2_kernel.elf\n\t\t-search \"11223344\" -patch \"4800\" -offset 4>\n\n");

	printf("Arguments:\n");
	printf("---------\n");	
	printf("-action:\n");
	printf("\tPATCH:\tpatch the file (single patch) ** default **\n");
	printf("\tFIND:\tfind patch location only\n\n");	
	printf("-filename:\tfull path of input file\n\n");
	printf("-search:\tsearch pattern to find\n\n");
	printf("-mask:\t\tmask pattern\t\t** optional **\n\n");
	printf("-replace:\tpatch to apply\t\t** optional **\n\n");
	printf("-offset:\toffset to apply patch\t** optional **\n\t\t(from found spot)\n\n");
	printf("-multi:\t\t** optional **\n");
	printf("\tYES:\tmultiple patches - enabled\n");
	printf("\tNO:\tmultiple patches - disabled ** default **\n\n");	
	printf("-debug:\t\t** optional **\n");;
	printf("\tYES:\tdebug info enabled\n");
	printf("\tNO:\tdebug info disabled ** default **\n\n");
	exit(-1);
}
//
///////////////////////////////////////////////////////////////


int __cdecl main(int argc, char *argv[])
{
	unsigned char* pFileBuffer = NULL;
	char szAction[MAX_PATH] ={0};
	char szInFile[MAX_PATH] = {0};
	char szSearchString[MAX_HEXSTRINGS_LENGTH] = {0};
	char szReplaceString[MAX_HEXSTRINGS_LENGTH] = {0};
	char szMaskString[MAX_HEXSTRINGS_LENGTH] = {0};		
	uint32_t args_mask = 0;
	uint32_t dwSizeOfBuffer = 0;
	uint32_t dwBytesWritten = 0;
	uint32_t dwReplaceOffset = 0;
	uint32_t dwExitStatus = EXIT_FAILURE;
	int i = 0;
	int index = 0;


		


#ifdef TOOL_DEBUG_ENABLED
	//////////////////////////////// DEFAULT ARGS FOR DEBUG TESTING FROM WITHIN VISUAL STUDIO ////////////////
	//
	//
	// default setup arguments
	strcpy_s(szAction, MAX_PATH, "find");
	strcpy_s(szInFile, MAX_PATH, "lv1ldr_org_455.self.elf");
	strcpy_s(szSearchString, MAX_BINARYPATTERN_LENGTH,  "0C000185340140801C1000813FE00283");	
//	strcpy_s(szSearchString, MAX_BINARYPATTERN_LENGTH,  "0C000185340140801C1000");	
//	strcpy_s(szMaskString, MAX_BINARYPATTERN_LENGTH,    "FFFFFFFFFFFFFFFFFFFFFF");	
	strcpy_s(szReplaceString, MAX_BINARYPATTERN_LENGTH, "60000000");
	dwReplaceOffset = 12;

	g_bDebugModeEnabled = TRUE;
	g_bMultiPatchEnabled = FALSE;
	g_bPatchingEnabled = TRUE;
	args_mask = 0x87;
		
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else

	// assure we have minimum args supplied
	if (argc < 3) {
		usage(NULL);
	}



	///////////////////////		MAIN ARG PARSING LOOP	/////////////////////////////
	//
	//
	for (i = 1; i < argc; i++)
	{
		switch ( index = select_string(argv[i]) ) {

			// "-action" argument
			case 0:
				memset(szAction, 0, MAX_PATH);
				if ( (argv[i+1] == NULL) )
					usage("-action");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 4) || (strlen(argv[i+1]) > 6) )
					usage("-action");
				if ( _stricmp(argv[i+1], "find") == 0 ) {
					strcpy_s(szAction, MAX_PATH, "find");
					g_bPatchingEnabled = FALSE;
				}
				else if ( _stricmp(argv[i+1], "patch") == 0 ) {
					strcpy_s(szAction, MAX_PATH, "patch");	
					g_bPatchingEnabled = TRUE;
				}
				else 
					usage("-action");				
				args_mask |= 0x01;
				i++;				
				break;

			// "-filename" argument
			case 1:
				memset(szInFile, 0, MAX_PATH);				
				if ( (argv[i+1] == NULL) )
					usage("-filename");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 1) || (strlen(argv[i+1]) > MAX_PATH) )
					usage("-filename");
				strcpy_s(szInFile, MAX_PATH, argv[i+1]);										
				args_mask |= 0x02;
				i++;		
				break;

			// "-search" argument
			case 2:
				memset(szSearchString, 0, MAX_BINARYPATTERN_LENGTH);				
				if ( (argv[i+1] == NULL) )
					usage("-search");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 1) || (strlen(argv[i+1]) > MAX_BINARYPATTERN_LENGTH) )
					usage("-search");
				strcpy_s(szSearchString, MAX_BINARYPATTERN_LENGTH, argv[i+1]);							
				args_mask |= 0x04;
				i++;		
				break;		

			// "-replace" argument
			case 3:
				memset(szReplaceString, 0, MAX_BINARYPATTERN_LENGTH);
				if ( (argv[i+1] == NULL) )
					usage("-replace");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) > MAX_BINARYPATTERN_LENGTH) )
					usage("-replace");
				strcpy_s(szReplaceString, MAX_BINARYPATTERN_LENGTH, argv[i+1]);												
				i++;		
				break;			

			// "-mask" argument
			case 4:
				memset(szMaskString, 0, MAX_BINARYPATTERN_LENGTH);
				if ( (argv[i+1] == NULL) )
					usage("-mask");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) > MAX_BINARYPATTERN_LENGTH) )
					usage("-mask");
				strcpy_s(szMaskString, MAX_BINARYPATTERN_LENGTH, argv[i+1]);
				args_mask |= 0x80;
				i++;		
				break;	

			// "-offset" argument
			case 5:
				dwReplaceOffset = 0;
				if ( (argv[i+1] == NULL) )
					usage("-offset");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 1) || (strlen(argv[i+1]) > 20) )
					usage("-offset");
				dwReplaceOffset = atoi(argv[i+1]);														
				i++;		
				break;					

			// "-multi" argument
			case 6:				
				if ( (argv[i+1] == NULL) )
					usage("-multi");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 2) || (strlen(argv[i+1]) > 3) )
					usage("-multi");
				if ( _stricmp(argv[i+1], "yes") == 0 )
					g_bMultiPatchEnabled = TRUE;
				else if ( _stricmp(argv[i+1], "no") == 0 )
					g_bMultiPatchEnabled = FALSE;
				else 
					usage("-multi");		
				i++;		
				break;

			// "-debug" argument
			case 7:				
				if ( (argv[i+1] == NULL) )
					usage("-debug");
				if ( (argv[i+1][0] == '-') || (strlen(argv[i+1]) < 2) || (strlen(argv[i+1]) > 3) )
					usage("-debug");
				if ( _stricmp(argv[i+1], "yes") == 0 )
					g_bDebugModeEnabled = TRUE;
				else if ( _stricmp(argv[i+1], "no") == 0 )
					g_bDebugModeEnabled = FALSE;
				else 
					usage("-debug");				
				i++;
				break;	

			default:
				printf("\nINVALID parameter specified:%s!\n", argv[i]);
				usage(NULL);
				break;

		} // end switch{}
	}
	//
	/////////////////////////////////////////////////////////////////////////////////////////

#endif

	

	printf("\n\n************************************************\n\n");
	printf("PATCHTOOL " PATCHTOOL_VERSION " (C) 2014 by anonymous\n\n");	
	printf("************************************************\n\n");	

	
	// make sure min. arg of "-action" and param specified
	if ( (args_mask & 0x07) != 0x07) {
		printf("\nError!  min. arguments not specified!\n");
		usage(NULL);
	}	
	// if 'patching' is enabled, and we specified NO replace
	// string, then error out
	if ( (g_bPatchingEnabled == TRUE) && (strlen(szReplaceString) < 1) ) {
		printf("\nError!  Patching is ENABLED, but replace string is empty!\n");
		usage(NULL);
	}	

	// if mask is not specified, then setup the default mask as all FFs
	if ( strlen(szMaskString) < 1 )		
		memset(szMaskString, 'FF', strlen(szSearchString));
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//																													//
	//											BINARY PATCHING MAIN ROUTINE											//
	//	

	// read the file in	
	if ( ReadFileToBuffer(szInFile,(uint8_t**)&pFileBuffer, 0x00, &dwSizeOfBuffer, TRUE) != STATUS_SUCCESS ) {
		printf("failed to read in file:%s, exiting...\n", szInFile);
		goto exit;
	}

	// if debug mode enabled, print out the setup params
	if (g_bDebugModeEnabled == TRUE) {	
		printf("Searching file:%s\n", szInFile);
		printf("FileSize: 0x%x bytes\n", dwSizeOfBuffer);
		printf("Search Pattern:  %s\n", szSearchString);		
		printf("Mask Pattern:    %s\n", szMaskString);
		printf("Replace Pattern: %s\n", szReplaceString);
		printf("Replace Offset:  0x%.2X\n", dwReplaceOffset);
	}	
	// go find/locate the data, and PATCH it, if enabled
	if ( find_data_in_buffer(pFileBuffer, dwSizeOfBuffer, szSearchString, szMaskString, szReplaceString, dwReplaceOffset) != STATUS_SUCCESS ) {
		printf("failed to find match(s) in file:%s, exiting!\n", szInFile);
		goto exit;
	}

	// if we found matches, and patching is enabled, then write out the new file buffer
	if ( g_bPatchingEnabled == TRUE ) {		
		if ( WriteBufferToFile(szInFile, pFileBuffer, dwSizeOfBuffer, FALSE, 0, &dwBytesWritten) != STATUS_SUCCESS) {
			printf("!ERROR! Failed to write patched buffer to file:%s, exiting!\n", szInFile);
			goto exit;			
		}
		if (dwBytesWritten != dwSizeOfBuffer) {
			printf("!ERROR! Incomplete write of patched buffer to file:%s, exiting!\n", szInFile);
			goto exit;	
		}	
	}	

	// SUCCESS!  Done!
	printf("\n<><> SUCCESS, patchtool execution complete!! <><>\n\n");
	dwExitStatus = EXIT_SUCCESS;
	 
	//																													//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
exit:	
	// free any alloc'd memory
	if (pFileBuffer != NULL)
		free(pFileBuffer);

	return dwExitStatus;
}
/**/
/********************************************************************************************************/
