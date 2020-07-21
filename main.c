#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "fvector.h"

typedef bool WORD[16];

// Declare registers & memory
struct ITSIAC
{
	// Registers
	uint16_t ACC;	// Accumulator
	uint8_t PSIAR;	// Primary Storage Instruction Address Register
	uint8_t SAR;	// Storage Address Register
	uint16_t SDR;	// Storage Data Register
	uint16_t TMPR;	// TeMPorary Register
	uint8_t CSIAR;	// Control Storage Instruction Address Register
	uint8_t MIR;	// MicroInstruction Register

	WORD memory[256];
};

/*
 * ITSIAC rules:
 * - one of the operands in any arithmetic operation must be in the accumulator (ACC), the other must be in primary storage
 * - the ITSIAC does not allow direct SAR to SDR transfer
 */

int main() {

	// Declare ITSIAC instance & zero registers
	struct ITSIAC machine =
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0
	};
	// Control storage is a file called control in the project root
	FILE* c_storage = fopen("control_storage", "r+");
	// Primary storage is a file called primary_storage in the project root
	FILE* p_storage = fopen("primary_storage", "r");
	// Buffer to hold current control storage opcode
	unsigned int opcode = 0x0;
	// Buffer to hold current control storage instruction arguments
	char* cs_args[2] = {calloc(6, sizeof(char)), calloc(6, sizeof(char))};

	// char** that holds all the primary storage lines
	char** ps_vector;
	// Pass ps_vector to method fvector(FILE*, char**) to fill it with instructions, getting total count of lines
	int ps_size = fvector(p_storage, ps_vector);

	// Read and execute control storage instructions
	// while loop that terminates when there are no more instructions
	while (1)
	{

		// If end of file, exit
		if (feof(c_storage))
		{
			printf("\nEND OF CONTROL STORAGE\n");
			return 100;
		}
		// Read next control storage opcode to opcode buffer
		fscanf(c_storage, "%X", &opcode);
		// Clear argument buffer
		cs_args[0] = calloc(6, sizeof(char));
		cs_args[1] = calloc(6, sizeof(char));
		// Print current instruction
		// TODO remove
		printf("%.2X\n", opcode);
		// Switch between actions depending on current instruction
		interpret_opcode:
		switch (opcode)
		{
			// 00 : MOVE
			case 0x00:
			{
				// Get arguments for instruction
				fscanf(c_storage, " %s %s", cs_args[0], cs_args[1]);
				uint16_t* destination;
				uint16_t* source;
				// Set destination of MOVE
				if (strcmp(cs_args[0], "ACC"))
					destination = &machine.ACC;
				if (strcmp(cs_args[0], "PSIAR"))
					destination = (uint16_t*) &machine.PSIAR;
				if (strcmp(cs_args[0], "TMPR"))
					destination = &machine.TMPR;
				if (strcmp(cs_args[0], "SAR"))
					destination = (uint16_t*) &machine.SAR;
				if (strcmp(cs_args[0], "SDR"))
					destination = &machine.SDR;
				// Set source of move
				if (strcmp(cs_args[1], "ACC"))
					source = &machine.ACC;
				if (strcmp(cs_args[1], "PSIAR"))
					source = (uint16_t*) &machine.PSIAR;
				if (strcmp(cs_args[1], "TMPR"))
					source = &machine.TMPR;
				if (strcmp(cs_args[1], "SDR"))
					source = &machine.SDR;
				break;
			}
			// 01 : INCREMENT
			case 0x01:
				machine.ACC++;
				break;
			// 02 : ADD
			case 0x02:
				{
					// Get register to add to ACC
					char* reg = calloc(6, sizeof(char));
					fscanf(c_storage, "%s", reg);
					// Depending on which register, add value to ACC
					if (strcmp(reg, "ACC"))			machine.ACC += machine.ACC;
					else if (strcmp(reg, "TMPR"))	machine.ACC += machine.TMPR;
					else if (strcmp(reg, "CSIAR"))	machine.ACC += machine.CSIAR;
					else if (strcmp(reg, "PSIAR"))	machine.ACC += machine.PSIAR;
				}
				break;
			// 03 : SUB
			case 0x03:
				{
					// Get register to add to ACC
					char* reg = calloc(6, sizeof(char));
					fscanf(c_storage, "%s", reg);
					// Depending on which register, add value to ACC
					if (strcmp(reg, "ACC"))			machine.ACC -= machine.ACC;
					else if (strcmp(reg, "TMPR"))	machine.ACC -= machine.TMPR;
					else if (strcmp(reg, "CSIAR"))	machine.ACC -= machine.CSIAR;
					else if (strcmp(reg, "PSIAR"))	machine.ACC -= machine.PSIAR;
				}
				break;
			// 04 : SET
			case 0x04:
				{
					// Get register to set
					char* reg = calloc(6, sizeof(char));
					// Get value to set register to
					uint16_t val = 0x0;
					fscanf(c_storage, "%s %hX", reg, &val);
					// Depending on which register, add value to ACC
					if (strcmp(reg, "ACC"))			machine.ACC = val;
					else if (strcmp(reg, "TMPR"))	machine.TMPR = val;
					else if (strcmp(reg, "CSIAR"))	machine.CSIAR = val;
					else if (strcmp(reg, "PSIAR"))	machine.PSIAR = val;
				}
			// 10 : READ
			case 0x10:
				fscanf(p_storage, "%s", ps_instruction);
				// ADD
				if (strcmp(ps_instruction, "ADD") == 0)
					machine.CSIAR = 10;
				// HALT
				if (strcmp(ps_instruction, "HALT") == 0)
				{
					opcode = 0xF0;
					goto interpret_opcode;
				}
				break;
			// 12 : SKIP
			case 0x12:
				if (machine.ACC == 0x0) machine.CSIAR += 2;
				else					machine.CSIAR += 1;
				break;
			// F0 : HALT
			case 0xF0:
				printf("-----------------------\n");
				printf("ACC:   %.16X\n", machine.ACC);
				printf("TMPR:  %.16X\n", machine.TMPR);
				printf("CSIAR:         %.8X\n", machine.CSIAR);
				printf("PSIAR:         %.8X\n", machine.PSIAR);
				printf("SAR:           %.8X\n", machine.SAR);
				printf("SDR:   %.16X\n", machine.SDR);
				printf("MIR:           %.8X\n", machine.MIR);
				printf("END OF JOB\n");
				return 0;
			// FF : Dummy comment
			case 0xFF:
				// Consume rest of line
				fscanf(c_storage, "%*[^\n]");
				break;
			// Unrecognized opcode
			default:
				printf("UNRECOGNIZED OPCODE %.2X\n", opcode);
				return 101;
		}
	}
	return 0;
}
