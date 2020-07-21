#include <stdio.h>
#include <stdlib.h>

#include "fvector.h"

int fvector(FILE* storage, char** vector)
{
	// int to hold vector size
	int v_size = 0;

	// For every line in supplied file, increment v_size
	while (!feof(storage))
		v_size++;

	// Send file pointer to beginning of file
	rewind(storage);

	// Allocate vector for length of lines
	// This is where the 15-character limit is created
	vector = calloc(v_size, sizeof(char[16]));

	// Read each line of the file into the vector
	for (int i = 0; i < v_size; i++)
		// Read each line into the vector, and if an EOF is returned, exit with code 12
		if (fscanf(storage, "%15[^\n]", vector[i]) == EOF)
			exit(12);

	// Return size of vector
	return v_size;
}
