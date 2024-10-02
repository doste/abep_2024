#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_rune_t.h>
#include <sys/_types/_size_t.h>

struct VectorOfInts {
	size_t size;
	size_t capacity;
	long* array;
};

struct VectorOfInts* new_vector_of_ints(void) {
	struct VectorOfInts* vec = malloc(sizeof(struct VectorOfInts));
	vec->capacity = 2;
	vec->size = 0;
	vec->array = malloc(sizeof(long) * vec->capacity);
	return vec;
}

void add_int_to_vector(struct VectorOfInts* vec, long int_to_add) {
	if(vec->size >= vec->capacity) {
		size_t new_capacity = 2 * vec->capacity;
		vec->capacity = new_capacity;
		vec->array = realloc(vec->array, new_capacity);
	}
	vec->array[vec->size] = int_to_add;
	vec->size++;
}

void print_vector_of_ints(struct VectorOfInts* vec) {
	for (size_t i = 0; i < vec->size; i++) {
		printf("%ld ", vec->array[i]);
	}
	printf("\n");
}

long get_int_at_index(struct VectorOfInts* vec, size_t index) {
	if (index > vec->size) {
		fprintf(stderr, "Error get_int_at_index: index not valid.\n");
		exit(1);
	}
	return vec->array[index];
}


// Just a 'warm-up':
void parse_genome(char* genome)
{
	size_t number_of_As, number_of_Cs, number_of_Ts, number_of_Gs;
	number_of_As = number_of_Cs = number_of_Gs = number_of_Ts = 0;
	
	char* ptr = genome;
	
	while( *ptr != '\0')
	{
		// First line starts with '>', we should skip it
		if (*ptr == '>') {
			while (*ptr != '\n') {
				ptr++;
			}
		}
		if (*ptr == 'A') {
			number_of_As++;
		} else if (*ptr == 'G') {
			number_of_Gs++;
		} else if (*ptr == 'C') {
			number_of_Cs++;
		} else if (*ptr == 'T') {
			number_of_Ts++;
		}
		ptr++;
	}
	printf("Number of As: %zu\n", number_of_As);
	printf("Number of Gs: %zu\n", number_of_Gs);
	printf("Number of Cs: %zu\n", number_of_Cs);
	printf("Number of Ts: %zu\n", number_of_Ts);
}

// First line starts with '>', we should skip it
size_t number_of_bytes_to_skip_first_line(char* genome)
{
	size_t number_of_bytes_to_skip = 0;
	char* ptr = genome;
	
	while( *ptr != '\0')
	{
		if (*ptr == '>') {
			while (*ptr != '\n') {
				number_of_bytes_to_skip++;
				ptr++;
			}
		}
		ptr++;
	}

	return number_of_bytes_to_skip;
}

// The idea of the skew algorithm was taken from:
// https://programmingforlovers.com/chapter-1-finding-replication-origins-in-bacterial-genomes/ch-1-go-code-alongs/building-a-skew-array-in-go/

// Skew takes a byte symbol as input.
// It returns an integer representing the skew of the symbol.
long skew(char symbol) {
	if (symbol == 'G') {
		return 1;
	} else if (symbol == 'C') {
		return -1;
	}
	return 0;
}


// SkewArray takes a string genome as input.
// It returns a slice of integers representing the skew of the genome at each position.
long* skew_array(char* genome, size_t size_of_genome) {
	long* the_skew_array = malloc(sizeof(long) * size_of_genome);
	the_skew_array[0] = 0;
	for (size_t i = 1; i < size_of_genome + 1; i++) {
		the_skew_array[i] = the_skew_array[i-1] + skew(genome[i-1]);
	}
	return the_skew_array;
}

void skew_array_by_arg(char* genome, size_t size_of_genome, long* skew_array_to_set) {
	skew_array_to_set[0] = 0;
	for (size_t i = 1; i < size_of_genome + 1; i++) {
		skew_array_to_set[i] = skew_array_to_set[i-1] + skew(genome[i-1]);
	}
}

// MinIntegerArray takes as input a slice of integers.
// It returns the minimum value in the slice.
long min_int_in_array(long* array, size_t size_of_array) {
	if (size_of_array == 0) {
		fprintf(stderr, "Error . Empty list given as input.\n");
		exit(1);
	}
	long min = array[0];
	for (size_t i = 0; i < size_of_array; i++) {
		if (array[i] < min ) {
			min = array[i];
		}
	}
	return min;
}


// MinimumSkew takes a string genome as input.
// It returns a slice of integers representing all integer
// indices minimizing the skew of the genome.
struct VectorOfInts* minimum_skew(char* genome, size_t size_of_genome) {
	struct VectorOfInts* indices = new_vector_of_ints();

	long* array = skew_array(genome, size_of_genome);

	long min_in_array = min_int_in_array(array, size_of_genome);
	for (size_t i = 0; i < size_of_genome; i++) {
		if (array[i] == min_in_array) {
			add_int_to_vector(indices, i);
		}
	}
	return indices;
}

char* read_genome_file(char* filename, size_t* size_of_file_to_set) {
	FILE* fptr = fopen(filename, "r");
	if (!fptr) {
		fprintf(stderr, "Error opening genome file.\n");
		exit(1);
	}

	fseek(fptr, 0L, SEEK_END);
	size_t size_of_file = ftell(fptr);
	rewind(fptr);
	
	char* buffer_for_genome = malloc(size_of_file + 1);
	
	size_t read = fread(buffer_for_genome, sizeof(char), size_of_file, fptr);
	if (read < size_of_file) {
		fprintf(stderr, "Error reading the genome file.\n");
		exit(1);
	}

	// The first line of the file is meta-data about the genome, we don't care about that,
	// so we should skip it.
	size_t to_skip = number_of_bytes_to_skip_first_line(buffer_for_genome);
	size_of_file -= to_skip;
	buffer_for_genome += to_skip;

	buffer_for_genome[size_of_file] = '\0';

	*size_of_file_to_set = size_of_file;
	return buffer_for_genome;
}

void write_skew_array_to_file(long* skew_array, size_t size_of_array_in_bytes, char* output_filename) {
	FILE* fptr_out = fopen(output_filename, "wb");
	if (!fptr_out) {
		fprintf(stderr, "Error creating output file.\n");
		exit(1);
	}
	size_t written = fwrite(skew_array, sizeof(char), size_of_array_in_bytes, fptr_out);
	if (written < size_of_array_in_bytes) {
		fprintf(stderr, "Error writing to the output file.\n");
		exit(1);
	}
}

void calculate_skew_array(char* input_filename) {
	FILE* fptr = fopen(input_filename, "r");
	if (!fptr) {
		fprintf(stderr, "Error opening genome file.\n");
		exit(1);
	}

	fseek(fptr, 0L, SEEK_END);
	size_t size_of_file = ftell(fptr);
	rewind(fptr);
	
	char* buffer_for_genome = malloc(size_of_file);
	
	size_t read = fread(buffer_for_genome, sizeof(char), size_of_file, fptr);
	if (read < size_of_file) {
		fprintf(stderr, "Error reading the genome file.\n");
		exit(1);
	}

	// The first line of the file is meta-data about the genome, we don't care about that,
	// so we should skip it.
	size_t to_skip = number_of_bytes_to_skip_first_line(buffer_for_genome);
	printf("We need to skip %zu bytes.\n", to_skip);
	size_of_file -= to_skip;
	buffer_for_genome += to_skip;
	
	printf("Genome read successfully. It has %zu nucleotides in total.\n", size_of_file);
	long* resulting_skew_array = skew_array(buffer_for_genome, size_of_file);
	struct VectorOfInts* minimum_skew_positions = minimum_skew(buffer_for_genome, size_of_file);
	printf("The minimum skew value of %ld occurs at positions:\n", resulting_skew_array[minimum_skew_positions->array[0]]);
	print_vector_of_ints(minimum_skew_positions);
}

long get_lenght_of_minimum_skew_positions(char* genome, size_t size_of_genome) {
	struct VectorOfInts* minimum_skew_positions = minimum_skew(genome, size_of_genome);
	return minimum_skew_positions->size;
}

void calculate_minimum_skew_positions_and_set_vector(
    char *genome, size_t size_of_genome, long *skew_array,
    size_t size_of_skew_array, struct VectorOfInts **minimum_skew_positions) {
	
    *minimum_skew_positions = minimum_skew(genome, size_of_genome);
	printf("The minimum skew value of %ld occurs at positions:\n", skew_array[(*minimum_skew_positions)->array[0]]);
	print_vector_of_ints(*minimum_skew_positions);
}

/* So we can build this file as a dynamic library
int main(int argc, char** argv)
{

	if (argc != 2) {
		fprintf(stderr, "Usage: ./parser <genome file to parse>\n");
		exit(1);
	}
	FILE* fptr = fopen(argv[1], "r");
	if (!fptr) {
		fprintf(stderr, "Error opening genome file.\n");
		exit(1);
	}

	
	fseek(fptr, 0L, SEEK_END);
	size_t size_of_file = ftell(fptr);
	rewind(fptr);
	
	char* buffer_for_genome = malloc(size_of_file);
	
	size_t read = fread(buffer_for_genome, sizeof(char), size_of_file, fptr);
	if (read < size_of_file) {
		fprintf(stderr, "Error reading the genome file.\n");
		exit(1);
	}

	// The first line of the file is meta-data about the genome, we don't care about that,
	// so we should skip it.
	size_t to_skip = number_of_bytes_to_skip_first_line(buffer_for_genome);
	printf("We need to skip %zu bytes.\n", to_skip);
	size_of_file -= to_skip;
	buffer_for_genome += to_skip;
	
	printf("Genome read successfully. It has %zu nucleotides in total.\n", size_of_file);
	long* resulting_skew_array = skew_array(buffer_for_genome, size_of_file);
	struct VectorOfInts* minimum_skew_positions = minimum_skew(buffer_for_genome, size_of_file);
	printf("The minimum skew value of %ld occurs at positions:\n", resulting_skew_array[minimum_skew_positions->array[0]]);
	print_vector_of_ints(minimum_skew_positions);
	
	return 0;
}
*/