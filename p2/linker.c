/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	int offset;
};

struct RelocationTableEntry {
	int offset;
	char inst[7];
	char label[7];
	int file;
};

struct FileData {
	int textSize;
	int dataSize;
	int symbolTableSize;
	int relocationTableSize;
	int textStartingLine; // in final executable
	int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry     symTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
	int textSize;
	int dataSize;
	int symTableSize;
	int relocTableSize;
};

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr; 
	int i, j;

	if (argc <= 2) {
		printf("error: usage: %s <obj file> ... <output-exe-file>\n",
				argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData files[MAXFILES];

  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; i++) {
		inFileString = argv[i+1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		int sizeText, sizeData, sizeSymbol, sizeReloc;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&sizeText, &sizeData, &sizeSymbol, &sizeReloc);

		files[i].textSize = sizeText;
		files[i].dataSize = sizeData;
		files[i].symbolTableSize = sizeSymbol;
		files[i].relocationTableSize = sizeReloc;

		// read in text section
		int instr;
		for (j = 0; j < sizeText; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = atoi(line);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < sizeData; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = atoi(line);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		int addr;
		for (j = 0; j < sizeSymbol; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < sizeReloc; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!
	
	


	/* combine files to CombinedFiles */
	struct CombinedFiles combinedFile;
	// get the total textSize of all files
	int total_textSize = 0;
	int total_dataSize = 0;
	for (int n = 0; n < argc - 2; n++){
		total_textSize += files[n].textSize;
		total_dataSize += files[n].dataSize;
	}

	for (int k = 0; k < argc - 2; k++){
		// combine TEXT
		for (int i = 0; i < files[k].textSize; i++)
		{
			combinedFile.text[combinedFile.textSize + i] = files[k].text[i];
		}
		// combine DATA
		for (int i = 0; i < files[k].dataSize; i++)
		{
			combinedFile.data[combinedFile.dataSize + i] = files[k].data[i];
		}

		// combine symbol table
		for (int i = 0; i < files[k].symbolTableSize; i++)
		{
			if (files[k].symbolTable[i].location == 'U') continue;

			if (!strcmp(files[k].symbolTable[i].label,"Stack"))
			{
				printf("Stack defined by object file.\n");
				exit(1);//Stack defined by obj file
			}

			for (int p = 0; p < combinedFile.symTableSize; p++)
			{
				if (!strcmp(combinedFile.symTable[p].label,files[k].symbolTable[i].label))
				{
					printf("Duplicated defined global label.\n");
					exit(1);//duplicated defined global label
				}
			}
			strcpy(combinedFile.symTable[combinedFile.symTableSize].label,files[k].symbolTable[i].label);
			combinedFile.symTable[combinedFile.symTableSize].location = files[k].symbolTable[i].location;
			if (files[k].symbolTable[i].location == 'T')
			{
				combinedFile.symTable[combinedFile.symTableSize].offset = files[k].symbolTable[i].offset + combinedFile.textSize;
			}
			else {
				combinedFile.symTable[combinedFile.symTableSize].offset = files[k].symbolTable[i].offset + combinedFile.dataSize + total_textSize;
			}
			combinedFile.symTableSize++;
		}
		files[k].textStartingLine = combinedFile.textSize;
		files[k].dataStartingLine = combinedFile.dataSize;
		combinedFile.textSize += files[k].textSize;
		combinedFile.dataSize += files[k].dataSize;
	}


	/* go through each reloc entry in reloc table of each file */
	for (int i = 0; i < argc - 2; i++)
	{
		for (int j = 0; j < files[i].relocationTableSize; j++)
		{
			/* for reloc entry of files[i].relocTable[j] */
			// .fill
			if (!strcmp(files[i].relocTable[j].inst,".fill")){
				// reloc local symbolic address
				if (files[i].relocTable[j].label[0]<='z'&&files[i].relocTable[j].label[0]>='a')
				{
					// local label defined in T
					if ((files[i].data[files[i].relocTable[j].offset]&0xFFFF) < files[i].textSize)
					{
						files[i].data[files[i].relocTable[j].offset] += files[i].textStartingLine;
					}
					// local label defined in D
					else {
						files[i].data[files[i].relocTable[j].offset] += (total_textSize - files[i].textSize) + files[i].dataStartingLine;
					}
				}
				// reloc global symbolic address: find it in combined symbol table; get the difference of offset.
				else {
					int symIndex = 0;
					while (strcmp(combinedFile.symTable[symIndex].label,files[i].relocTable[j].label)&&symIndex<combinedFile.symTableSize) symIndex++;
					if (symIndex == combinedFile.symTableSize) {
						if (!strcmp(files[i].relocTable[j].label,"Stack")){
							files[i].data[files[i].relocTable[j].offset] = total_dataSize + total_textSize;
						}
						else {
							printf("Undefined global label used in .fill: %s\n",files[i].relocTable[j].label);
							exit(1);// undefined global label.
						}
					}
					else {
						int local_symIndex = 0;
						while (strcmp(files[i].symbolTable[local_symIndex].label,files[i].relocTable[j].label)&&local_symIndex<files[i].symbolTableSize) local_symIndex++;
						if (local_symIndex == files[i].symbolTableSize) exit(1);
						if (files[i].symbolTable[local_symIndex].location == 'D')
						{
							files[i].data[files[i].relocTable[j].offset] += combinedFile.symTable[symIndex].offset - (files[i].textSize + (files[i].symbolTable[local_symIndex].offset & 0xFFFF));
						}
						else // 'U'/'T'
						{
							files[i].data[files[i].relocTable[j].offset] += combinedFile.symTable[symIndex].offset - (files[i].symbolTable[local_symIndex].offset & 0xFFFF);
						}
						
					}
				}
			}
			// lw/sw
			else {
				// reloc local
				if (files[i].relocTable[j].label[0]<='z'&&files[i].relocTable[j].label[0]>='a'){
					// local label defined in T
					if ((files[i].text[files[i].relocTable[j].offset]&0xFFFF) < files[i].textSize)
					{
						files[i].text[files[i].relocTable[j].offset] += files[i].textStartingLine;
					}
					// local label defined in D
					else {
						files[i].text[files[i].relocTable[j].offset] += (total_textSize - files[i].textSize) + files[i].dataStartingLine;
					}
				}
				// reloc global
				else {
					int symIndex = 0;
					while (strcmp(combinedFile.symTable[symIndex].label,files[i].relocTable[j].label)&&symIndex<combinedFile.symTableSize) symIndex++;
					if (symIndex == combinedFile.symTableSize){
						if (!strcmp(files[i].relocTable[j].label,"Stack")){
							files[i].text[files[i].relocTable[j].offset] += total_dataSize + total_textSize;
						}
						else {
							printf("Undefined global label used in lw/sw: %s\n",files[i].relocTable[j].label);
							exit(1);// undefined global label.
						}
					}
					else {
						int local_symIndex = 0;
						while (strcmp(files[i].symbolTable[local_symIndex].label,files[i].relocTable[j].label)&&local_symIndex<files[i].symbolTableSize) local_symIndex++;
						if (local_symIndex == files[i].symbolTableSize) exit(1);
						if (files[i].symbolTable[local_symIndex].location == 'D')
						{
							files[i].text[files[i].relocTable[j].offset] += combinedFile.symTable[symIndex].offset - (files[i].textSize + (files[i].symbolTable[local_symIndex].offset& 0xFFFF)) ;
						}
						else // 'U'/'T'
						{
							files[i].text[files[i].relocTable[j].offset] += combinedFile.symTable[symIndex].offset - (files[i].symbolTable[local_symIndex].offset & 0xFFFF);
						}
					}
				}
			}
		}
	}
	int curr_text = 0;
	int curr_data = 0;
	for (int k = 0; k < argc - 2; k++){
		// combine TEXT
		for (int i = 0; i < files[k].textSize; i++)
		{
			combinedFile.text[curr_text + i] = files[k].text[i];
		}
		// combine DATA
		for (int i = 0; i < files[k].dataSize; i++)
		{
			combinedFile.data[curr_data + i] = files[k].data[i];
		}
		curr_text += files[k].textSize;
		curr_data += files[k].dataSize;
	}

	
	// for (int i = 0; i <  total_textSize; i++)
	// {
	// 	printf("%d\n",combinedFile.text[i]);
	// }

	// for (int i = 0; i <  total_dataSize; i++)
	// {
	// 	printf("%d\n",combinedFile.data[i]);
	// }


	for (int i = 0; i < total_textSize; i++)
	{
		fprintf(outFilePtr,"%d\n",combinedFile.text[i]);
	}
	for (int i = 0; i < total_dataSize; i++)
	{
		fprintf(outFilePtr,"%d\n",combinedFile.data[i]);
	}
	
} // main
