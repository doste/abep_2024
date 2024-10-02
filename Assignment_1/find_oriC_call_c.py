import ctypes
import pathlib
import sys
import numpy as np
from matplotlib import pyplot as plt

class VECTOR_OF_INTS(ctypes.Structure):
    _fields_ = [("size", ctypes.c_size_t),
                ("capacity", ctypes.c_size_t),
                ("array", ctypes.POINTER(ctypes.c_long))]
    

if __name__ == '__main__':

    if len( sys.argv ) != 2:
        print("Usage: python3 find_oriC_call_c.py <salmonella_enterica_sequence.fasta>")
        exit(1)
    
    # Import the library
    lib_path = pathlib.Path("build/libfindoriC.dylib")
    dylib = ctypes.CDLL(str(lib_path.resolve()))

    # Specify return type and argument types of each C function:
    # void skew_array_by_arg(char* genome, size_t size_of_genome, long* skew_array_to_set) 
    dylib.skew_array_by_arg.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_long)]
    dylib.skew_array_by_arg.restype = None

    #char* read_genome_file(char* filename, size_t* size_of_file_to_set)
    dylib.read_genome_file.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_size_t)]
    dylib.read_genome_file.restype = ctypes.c_char_p

    #void calculate_minimum_skew_positions_and_set_vector( char *genome, size_t size_of_genome, long *skew_array,
    #                                                  size_t size_of_skew_array, struct VectorOfInts **minimum_skew_positions)
    dylib.calculate_minimum_skew_positions_and_set_vector.argtypes = [ctypes.c_char_p,
                                                        ctypes.c_size_t,
                                                        ctypes.POINTER(ctypes.c_long),
                                                        ctypes.c_size_t,
                                                        ctypes.POINTER(ctypes.POINTER(VECTOR_OF_INTS))]
    dylib.calculate_minimum_skew_positions_and_set_vector.restype = None

    #long get_lenght_of_minimum_skew_positions(char* genome, size_t size_of_genome)
    dylib.get_lenght_of_minimum_skew_positions.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
    dylib.get_lenght_of_minimum_skew_positions.restype = ctypes.c_long

    #void print_vector_of_ints(struct VectorOfInts* vec)
    dylib.print_vector_of_ints.argtypes = [ctypes.POINTER(VECTOR_OF_INTS)]
    dylib.print_vector_of_ints.restype = None

    #long get_int_at_index(struct VectorOfInts* vec, size_t index)
    dylib.get_int_at_index.argtypes = [ctypes.POINTER(VECTOR_OF_INTS), ctypes.c_size_t]
    dylib.get_int_at_index.restype = ctypes.c_long


    # read input genome file
    input_genome_file = sys.argv[1]
    size_of_genome_to_set = (ctypes.c_size_t)()
    input_genome = dylib.read_genome_file(ctypes.c_char_p(input_genome_file.encode('utf-8')), size_of_genome_to_set)

    size_of_genome = len(input_genome)

    # reserve space for the skew_array
    skew_array = (ctypes.c_long * size_of_genome)()

    # call the function with the arguments correctly casted to ctypes
    dylib.skew_array_by_arg(ctypes.c_char_p(input_genome), ctypes.c_size_t(size_of_genome), skew_array)

    # get length of vector of minimum skew indices
    minimum_skew_positions_vector_length = dylib.get_lenght_of_minimum_skew_positions(input_genome, size_of_genome)
    # and reserve memory to hold it
    minimum_skew_positions_vector = (ctypes.c_long * minimum_skew_positions_vector_length)()

    # declare vector variable that the C function calculate_minimum_skew_positions_and_set_vector will set 
    ptr_to_vector_ints_to_set = ctypes.POINTER(VECTOR_OF_INTS)()

    #void calculate_minimum_skew_positions_and_set_vector(
    #                                                   char *genome, size_t size_of_genome, long *skew_array,
    #                                                   size_t size_of_skew_array, struct VectorOfInts **minimum_skew_positions)
    dylib.calculate_minimum_skew_positions_and_set_vector(input_genome, size_of_genome, skew_array, size_of_genome, ctypes.byref(ptr_to_vector_ints_to_set))
    
    # Cast the output back to a Python list and print the results
    output_skew_array = list(skew_array)

    # to debug:
    # dylib.print_vector_of_ints(ptr_to_vector_ints_to_set)

    skew_indices = []
    for i in range(0, minimum_skew_positions_vector_length):
        skew_indices.append(dylib.get_int_at_index(ptr_to_vector_ints_to_set, i))
    
    # let's pick just one of the skew indices to mark it in the graph
    skew_index = skew_indices[0]

    plt.xlabel('Genome position', fontsize=12)
    plt.ylabel('Skew value', fontsize=12)
    plt.plot(list(range(len(output_skew_array))), output_skew_array)

    x_min = list(range(len(output_skew_array)))[skew_index]
    y_min = output_skew_array[skew_index]
    plt.plot(x_min, y_min, marker='o')

    plt.ticklabel_format(scilimits=(0, 101))
    txt = "The minimum skew value of " + str(output_skew_array[skew_index]) + " occurs at positions: "
    for i in range(0, minimum_skew_positions_vector_length):
        txt += str(dylib.get_int_at_index(ptr_to_vector_ints_to_set, i))
        txt += " "

    plt.figtext(0.5, 0.009, txt, wrap=True, horizontalalignment='center', fontsize=10)

    plt.show()
