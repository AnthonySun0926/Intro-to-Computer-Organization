/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<stdint.h>

#define MAXLINELENGTH 1000
#define MAXMEMORY 65536 

struct label_info
{
    char label_name[7];
    int address;
};

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int indexLabel(struct label_info *, int, char*);

int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    int opc, ra, rb, destR, offset;
    int decimalCode;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    struct label_info label_address[MAXMEMORY];
    int labelIndex = 0, array_index = 0;
    while (readAndParse(inFilePtr,label,opcode,arg0,arg1,arg2))
    {
        if (indexLabel(label_address,array_index,label)!=-1) exit(1);//duplicate label
        
        if (strlen(label)!=0)
        {
            strcpy(label_address[array_index].label_name,label);
            label_address[array_index].address = labelIndex;
            array_index++;
        }
        labelIndex++;
    }

    // for (size_t i = 0; i < array_index; i++)
    // {
    //     // test pass 1
    //     printf("%s , %d \n",label_address[i].label_name,label_address[i].address);
    // }
    
    rewind(inFilePtr);

    int PC = 0;
    while (readAndParse(inFilePtr,label,opcode,arg0,arg1,arg2))
    {
        if (!strcmp(opcode, "add")||!strcmp(opcode,"nor")) 
        {
            if(!strcmp(opcode,"add")) opc = 0b000;
            else opc = 0b001;
            ra = atoi(arg0);//printf("%d\n",ra);
            rb = atoi(arg1);//printf("%d\n",rb);
            destR = atoi(arg2);
            decimalCode = (opc<<22) + (ra<<19) + (rb<<16) + destR;
        }
        
        /* l-Type */
        else if (!strcmp(opcode, "lw")||!strcmp(opcode, "sw")||!strcmp(opcode, "beq"))
        {
            if (!strcmp(opcode, "lw"))
            {
                opc = 0b010;
            }
            else if (!strcmp(opcode, "sw"))
            {
                opc = 0b011;
            }
            else opc = 0b100;
            
            ra = atoi(arg0);
            rb = atoi(arg1);

            if (isNumber(arg2))
            {
                // the offset is number.
                offset = atoi(arg2);
            }
            else {
                // the offset is a symbolic label.
                offset = indexLabel(label_address,array_index,arg2);
                if (offset==-1) exit(1);
                if (opc == 0b100)
                {
                    offset = offset - PC -1;
                }
                
            }
            if (offset<-32768 || offset>32767)
            {
                // offsetFields that don’t fit in 16 bits.
                exit(1);
            }
            
            decimalCode = (opc<<22) + (ra<<19) + (rb<<16) + (0xFFFF & offset);
        }
        
        else if (!strcmp(opcode, "jalr"))
        {
            opc = 0b101;
            ra = atoi(arg0);
            rb = atoi(arg1);
            decimalCode = (opc<<22) + (ra<<19) + (rb<<16);
        }
        
        else if (!strcmp(opcode, "halt")||!strcmp(opcode, "noop"))
        {
            if(!strcmp(opcode, "halt")) opc = 0b110;
            else opc = 0b111;
            decimalCode = opc<<22;
        }
        
        else if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
            {
                decimalCode = atoi(arg0);
            }
            else {
                decimalCode = indexLabel(label_address,array_index,arg0);
                if (decimalCode==-1) exit(1);
            }
        }
        else exit(1);// Unrecognized opcodes.

        PC++;
        // test for second pass
        // printf("%d\n",decimalCode);
        fprintf(outFilePtr,"%d\n",decimalCode);
    }
    
    // /* here is an example for how to use readAndParse to read a line from
    //     inFilePtr */
    // if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //     /* reached end of file */
    // }
    

    // /* this is how to rewind the file ptr so that you start reading from the
    //     beginning of the file */
    // rewind(inFilePtr);

    // /* after doing a readAndParse, you may want to do the following to test the
    //     opcode */
    // if (!strcmp(opcode, "add")) {
    //     /* do whatever you need to do for opcode "add" */
    // }

    return(0);
}

// return the address of target label. If label is not existed, return -1;
int indexLabel(struct label_info * memory, int len, char* target){
    for (int i = 0; i < len; i++)
    {
        if (!strcmp(target, memory[i].label_name))
        {
            return memory[i].address;
        }
    }
    return -1;
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if successfully read
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    char *ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}


