#include <stdio.h>
#include <stdbool.h>
/*
1. Parse stdin, get path to ghidra repo, name of program
2. parse ~index.dat:
    - find the appropriate .gbf database file
3. parse .gbf file
    - find location of master table
    - find location of symbol table
    - find location of function data table
    - for each symbol in the symbol table, if it's a function match it w/ function data table and print appropriate data
*/
#define MAX_PATH 256

// Parse ~index.dat to find the appropriate .gbf database file
// Return the path to the .gbf file
void parse_index_dat(char *ghidra_path, char *program_name, char* gbf_file_path, uint gbf_file_path_size) {

    //.gpr is just the pointer to the directory. Assume they want the repo w/ the same name.
    if (strncmp(ghidra_path + strlen(ghidra_path) - 4, ".gpr", 4) == 0) {
        ghidra_path[strlen(ghidra_path) - 4] = '\0';
        strcat(ghidra_path, ".rep");
    }

    char index_dat_path[MAX_PATH];
    snprintf(index_dat_path, sizeof(index_dat_path), "%s/idata/~index.dat", ghidra_path);
    FILE *index_dat_file = fopen(index_dat_path, "r");
    if (!index_dat_file) {
        fprintf(stderr, "Error: Unable to open ~index.dat file. Is %s a ghidra repo?\n", ghidra_path);
        exit(1);
    }

    char* match = NULL;
    char index_data[MAX_PATH];
    while (fgets(index_data, sizeof(index_data), index_dat_file)) {
        if (match = strstr(index_data, program_name)) {
            break;
        }
    }
    fclose(index_dat_file);

    if(!match || match - index_data < 9){
        fprintf(stderr, "Error: Unable to find .gbf database file for program %s in %s\n", program_name, index_dat_path);
        exit(1);
    }

    // todo
    char folder_0 = *(match - 4);
    char folder_1 = *(match - 3);
    *(match-1) = '\0';

    char gbf_folder_path[MAX_PATH];
    snprintf(gbf_folder_path, sizeof(gbf_folder_path), "%s/idata/%c%c/%s/%s.gbf", ghidra_path, folder_0, folder_1, program_name, program_name);

    
    
}

int main(char ** argv, int argc) {

    char* ghidra_path, *program_name;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ghidra_path>@<program_name>\n", argv[0]);
        return 1;
    }

    // Parse ~index.dat to find the appropriate .gbf database file
    char *gbf_file = parse_index_dat(ghidra_path, program_name);

    // Parse the .gbf file to find the locations of the master table, symbol table, and function data table
    struct gbf_file *gbf = parse_gbf_file(gbf_file);
    struct master_table *master_table = find_master_table(gbf);
    struct symbol_table *symbol_table = find_symbol_table(gbf);
    struct function_data_table *function_data_table = find_function_data_table(gbf);

    // For each symbol in the symbol table, if it's a function, match it with the function data table and print appropriate data
    for (int i = 0; i < symbol_table->num_symbols; i++) {
        struct symbol *symbol = &symbol_table->symbols[i];
        if (symbol->type == FUNCTION) {
            struct function_data *function_data = find_function_data(function_data_table, symbol->name);
            printf("Function: %s\n", symbol->name);
            printf("Address: 0x%lx\n", function_data->address);
            printf("Size: %lu bytes\n", function_data->size);
            printf("Return Type: %s\n", function_data->return_type);
            printf("Parameters: %s\n", function_data->parameters);
        }
    }

    // Free allocated memory
    free(ghidra_path);
    free(program_name);
    free(gbf_file);
    free_gbf_file(gbf);
    free_master_table(master_table);
    free_symbol_table(symbol_table);
    free_function_data_table(function_data_table);

    return 0;
}