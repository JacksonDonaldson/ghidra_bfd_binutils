#include "localbufferfile.h"

typedef struct {
    char name[0x80];
    uint schema_version;
    uint root_buffer_id;
    byte key_type;
    uint schema_field_types_len;
    byte *schema_field_types;
    uint schema_field_names_len;
    char *schema_field_names;
    uint index_column;
    long long max_key;
    uint record_count;
} tabledata;

uint find_master_table(char* gbf_file_path, localbufferfile* lbf);

uint find_table_in_master_table(localbufferfile* lbf, char* table, uint master_table_offset, tabledata* table_data);

void print_tabledata(tabledata* table_data);

void free_tabledata(tabledata* table_data);


