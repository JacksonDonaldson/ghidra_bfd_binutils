#include "localbufferfile.h"

typedef struct {
    char name[0x80];
    uint schema_version;
    uint root_buffer_id;
    byte key_type[0x80];
    byte schema_field_types[0x80];
    char schema_field_names[0x80];
    uint index_column;
    long long max_key;
    uint record_count;
} tabledata;

uint find_master_table(char* gbf_file_path, localbufferfile* lbf);

uint find_table_in_master_table(localbufferfile* lbf, char* table, uint master_table_offset);
