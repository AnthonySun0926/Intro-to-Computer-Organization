/**
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int);

void getOpcode(stateType * state);
void Add(stateType * state);
void Nor(stateType * state);
void Lw(stateType * state);
void Sw(stateType * state);
void Beq(stateType * state);
void Jalr(stateType * state);


int
main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    //initialize regeisters to be 0.
    for (int i = 0; i < NUMREGS; i++)
    {
        state.reg[i] = 0;
    }

    int cnt = 0;
    //execute instructions.
    while (state.pc < state.numMemory)
    {
        printState(&state);
        cnt++;
    //exact opcode from instrcution.
    int opcode = (state.mem[state.pc] >> 22) & 0b111;
    switch (opcode)
    {
    case 0b000:
        //add
        Add(&state);
        break;
    case 0b001:
        //nor
        Nor(&state);
        break;
    case 0b010:
        //lw
        Lw(&state);
        break;
    case 0b011:
        //sw
        Sw(&state);
        break;
    case 0b100:
        //beq
        Beq(&state);
        break;
    case 0b101:
        //jalr
        Jalr(&state);
        break;
    case 0b110:
        //halt
        {
           state.pc++;
           goto endwhile;
           break; 
        }
    default:
        //noop
        break;
    }
        state.pc++;
    }
    
    endwhile: printf("machine halted\ntotal of %d instructions executed\nfinal state of machine:\n",cnt);
    printState(&state);
    return(0);
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}



void Add(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    int destR = state->mem[state->pc] & 0b111;
    //regA + regB = destR
    state->reg[destR] = state->reg[regA] + state->reg[regB];
}

void Nor(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    int destR = state->mem[state->pc] & 0b111;
    //!(regA | regB) = destR
    state->reg[destR] = ~(state->reg[regA] | state->reg[regB]);
}

void Lw(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    int offset = state->mem[state->pc] & 0xFFFF;
    //reg[regB] = mem[regA + offset]
    state->reg[regB] = state->mem[state->reg[regA] + convertNum(offset)];
}

void Sw(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    int offset = state->mem[state->pc] & 0xFFFF;
    // mem[regA + offset] = reg[regB]
    state->mem[state->reg[regA] + convertNum(offset)] = state->reg[regB] ;
}

void Beq(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    int offset = state->mem[state->pc] & 0xFFFF;
    //if regA = regB, branch to address PC+1+offsetField.
    if (state->reg[regA] == state->reg[regB])
    {
        state->pc = state->pc + convertNum(offset);
    }
}

void Jalr(stateType * state){
    int regA = (state->mem[state->pc] >> 19) & 0b111;
    int regB = (state->mem[state->pc]  >> 16) & 0b111;
    //store PC+1 into regB
    state->reg[regB] = state->pc + 1;
    //branch to the address contained in regA
    state->pc = state->reg[regA]-1;
}
