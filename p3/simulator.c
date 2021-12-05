/**
 * EECS 370 Project 3
 * Pipeline Simulator
 *
 * This fragment should be used to modify your project 1 simulator to simulator
 * a pipeline
 *
 * Make sure *not* to modify printState or any of the associated functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
	int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
	int instr;
	int pcPlus1;
	int readRegA;
	int readRegB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
	int instr;
	int branchTarget;
	int aluResult;
	int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
	int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
	int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; /* number of cycles run so far */
} stateType;

int
field0(int instruction)
{
    return( (instruction>>19) & 0x7);
}

int
field1(int instruction)
{
    return( (instruction>>16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int
opcode(int instruction)
{
    return(instruction>>22);
}

void
printInstruction(int instr)
{

    char opcodeString[10];

    if (opcode(instr) == ADD) {
        strcpy(opcodeString, "add");
    } else if (opcode(instr) == NOR) {
        strcpy(opcodeString, "nor");
    } else if (opcode(instr) == LW) {
        strcpy(opcodeString, "lw");
    } else if (opcode(instr) == SW) {
        strcpy(opcodeString, "sw");
    } else if (opcode(instr) == BEQ) {
        strcpy(opcodeString, "beq");
    } else if (opcode(instr) == JALR) {
        strcpy(opcodeString, "jalr");
    } else if (opcode(instr) == HALT) {
        strcpy(opcodeString, "halt");
    } else if (opcode(instr) == NOOP) {
        strcpy(opcodeString, "noop");
    } else {
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
        field2(instr));
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
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

int
main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    stateType newState;
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

    /* read the entire machine-code file into instr memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {
        if (sscanf(line, "%d", state.instrMem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
    }
    //initialize dataMem
    for (int i = 0; i < state.numMemory; i++)
    {
        state.dataMem[i] = state.instrMem[i];
    }

    printf("%d mempry words\n",state.numMemory);
    printf("\tinstruction memory:\n");
    for (int i = 0; i < state.numMemory; i++){
        printf("\t\tinstrMem[ %d ] ", i);
        printInstruction(state.instrMem[i]);
    }

    // initialize pc and all registers to 0.
    state.pc = 0;
    for (int i = 0; i < NUMREGS; i++)
    {
        state.reg[i] = 0;
    }
    
    // Initialize the instruction field in all pipeline registers to the noop instruction (0x1c00000).
    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;


    // run()
    while (1) {

    printState(&state);

    /* check for halt */
    if (opcode(state.MEMWB.instr) == HALT) {
    printf("machine halted\n");
    printf("total of %d cycles executed\n", state.cycles);
    exit(0);
    }

    newState = state;
    newState.cycles++;

    /* --------------------- IF stage --------------------- */
    newState.IFID.instr = state.instrMem[state.pc];
    newState.IFID.pcPlus1 = state.pc + 1;
    newState.pc++;

    /* --------------------- ID stage --------------------- */
    newState.IDEX.instr = state.IFID.instr;
    newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
    // lw immediately followed by a dependency: we only cares about field0 & field1
    if (opcode(state.IDEX.instr)==LW && (field1(state.IDEX.instr)==field0(newState.IDEX.instr)||field1(state.IDEX.instr)==field1(newState.IDEX.instr)))
    {
        newState.IDEX.instr = NOOPINSTRUCTION;
        newState.pc = state.pc;
        newState.IFID = state.IFID;
    }
    else {
        newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
        newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
        newState.IDEX.offset = convertNum(field2(state.IFID.instr));
    }

    /* --------------------- EX stage --------------------- */
    newState.EXMEM.instr = state.IDEX.instr;
    newState.EXMEM.branchTarget = state.IDEX.offset + state.IDEX.pcPlus1;
    int currRegA = state.IDEX.readRegA;
    int currRegB = state.IDEX.readRegB;

    /* data hazard */
    // 1. check WBEND
    if (opcode(state.WBEND.instr) == LW)
    {
        // field1 value is changed
        if (field0(newState.EXMEM.instr) == field1(state.WBEND.instr))
        {
            currRegA = state.WBEND.writeData;
        }
        if (field1(newState.EXMEM.instr) == field1(state.WBEND.instr))
        {
            currRegB = state.WBEND.writeData;
        }
    }
    if (opcode(state.WBEND.instr) == ADD || opcode(state.WBEND.instr) == NOR)
    {
        // field2 value is changed
        if (field0(newState.EXMEM.instr) == field2(state.WBEND.instr))
        {
            currRegA = state.WBEND.writeData;
        }
        if (field1(newState.EXMEM.instr) == field2(state.WBEND.instr))
        {
            currRegB = state.WBEND.writeData;
        }
    }
    
    // 2. check MEMWB
    if (opcode(state.MEMWB.instr) == LW)
    {
        // field1 value is changed
        if (field0(newState.EXMEM.instr) == field1(state.MEMWB.instr))
        {
            currRegA = state.MEMWB.writeData;
        }
        if (field1(newState.EXMEM.instr) == field1(state.MEMWB.instr))
        {
            currRegB = state.MEMWB.writeData;
        }
    }
    if (opcode(state.MEMWB.instr) == ADD || opcode(state.MEMWB.instr) == NOR)
    {
        // field2 value is changed
        if (field0(newState.EXMEM.instr) == field2(state.MEMWB.instr))
        {
            currRegA = state.MEMWB.writeData;
        }
        if (field1(newState.EXMEM.instr) == field2(state.MEMWB.instr))
        {
            currRegB = state.MEMWB.writeData;
        }
    }

    // 3. check EXMEM
    if (opcode(state.EXMEM.instr) == LW)
    {
        // field1 value is changed
        if (field0(newState.EXMEM.instr) == field1(state.EXMEM.instr))
        {
            currRegA = state.EXMEM.aluResult;
        }
        if (field1(newState.EXMEM.instr) == field1(state.EXMEM.instr))
        {
            currRegB = state.EXMEM.aluResult;
        }
    }
    if (opcode(state.EXMEM.instr) == ADD || opcode(state.EXMEM.instr) == NOR)
    {
        // field2 value is changed
        if (field0(newState.EXMEM.instr) == field2(state.EXMEM.instr))
        {
            currRegA = state.EXMEM.aluResult;
        }
        if (field1(newState.EXMEM.instr) == field2(state.EXMEM.instr))
        {
            currRegB = state.EXMEM.aluResult;
        }
    }

    // update newState.EXMEM.aluresult
    switch (opcode(newState.EXMEM.instr))
    {
    case ADD:
        newState.EXMEM.aluResult = currRegA + currRegB;
        break;
    case NOR:
        newState.EXMEM.aluResult = ~(currRegA | currRegB);
        break;
    case LW:
        newState.EXMEM.aluResult = currRegA + state.IDEX.offset;
        break;
    case SW:
        newState.EXMEM.aluResult = currRegA + state.IDEX.offset;
        break;
    case BEQ:
        newState.EXMEM.aluResult = (currRegA == currRegB);
        break;
    default:
        break;
    }
    
    // update newState.EXMEM.readRegB
    if (opcode(newState.EXMEM.instr) != NOOP)
    {
        newState.EXMEM.readRegB = currRegB;
    }

    /* --------------------- MEM stage --------------------- */
    newState.MEMWB.instr = state.EXMEM.instr;
    int opcode_mem = opcode(newState.MEMWB.instr);

    if (opcode_mem == LW)
    {
        newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
    }
    if (opcode_mem == SW)
    {
        newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
    }
    if (opcode_mem == ADD || opcode_mem == NOR)
    {
        newState.MEMWB.writeData = state.EXMEM.aluResult;
    }
    if (opcode_mem == BEQ)
    {
        // if the BEQ should be taken, change the instructions in previous pipelines to noop
        if (state.EXMEM.aluResult == 1)
        {
            newState.pc = state.EXMEM.branchTarget;
            newState.IFID.instr = NOOPINSTRUCTION;
            newState.IDEX.instr = NOOPINSTRUCTION;
            newState.EXMEM.instr = NOOPINSTRUCTION;
        }
    }

    /* --------------------- WB stage --------------------- */
    newState.WBEND.instr = state.MEMWB.instr;
    newState.WBEND.writeData = state.MEMWB.writeData;

    int opcode_wb = opcode(newState.WBEND.instr);

    if (opcode_wb == LW)
    {
        newState.reg[field1(newState.WBEND.instr)] = state.MEMWB.writeData;
    }
    if (opcode_wb == ADD || opcode_wb == NOR)
    {
        newState.reg[field2(newState.WBEND.instr)] = state.MEMWB.writeData;
    }
    
    state = newState; /* this is the last statement before end of the loop.
        It marks the end of the cycle and updates the
        current state with the values calculated in this
        cycle */
    }

    return(0);
}
