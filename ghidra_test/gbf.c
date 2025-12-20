#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "localbufferfile.h"
#include "gbf.h"

int readint(byte * buffer, int offset){
    return (buffer[offset] << 24) | (buffer[offset+1] << 16) | (buffer[offset+2] << 8) | buffer[offset+3];
}
long long readlong(byte * buffer, int offset){
    return (long long)buffer[offset] << 56 | (long long)buffer[offset+1] << 48 | (long long)buffer[offset+2] << 40 | (long long)buffer[offset+3] << 32 |
           (long long)buffer[offset+4] << 24 | (long long)buffer[offset+5] << 16 | (long long)buffer[offset+6] << 8 | (long long)buffer[offset+7];
}

uint find_master_table(char* gbf_file_path, localbufferfile* lbf) {
    create_localbufferfile(gbf_file_path, lbf);

    byte * master_buf = get_buffer(lbf, 1);

    if (master_buf[0] != 0x09) {
        fprintf(stderr, "Error: Expected chained buffer byte\n");
        exit(1);
    }
    uint size = readint(master_buf, 1);
    if(size < 9){
        fprintf(stderr, "Error: parm size too small to contain master table offset\n");
        exit(1);
    }
    byte version = master_buf[5];
    if(version != 0x01){
        fprintf(stderr, "Error: Unsupported parm version %02x\n", version);
        exit(1);
    }

    uint master_table_offset = readint(master_buf, 6);

    free(master_buf);
    return master_table_offset;
}

//if table_name is found, fills out tabledata
uint find_table_in_master_table(localbufferfile* lbf, char* table_name, uint master_table_offset, tabledata * data) {
    // Read the master table
    byte *master_table = get_buffer(lbf, master_table_offset + 1);

    uint target_len = strlen(table_name);
    if (target_len >= 0x80){
        fprintf(stderr, "Error: table name too long\n");
        exit(1);
    }

    byte node_type = master_table[0];
    if (node_type != 0x01) {
        fprintf(stderr, "Error: expected master to be node type LONGKEY_VAR_REC_NODE \n");
        exit(1);
    }

    uint record_count = readint(master_table, 1);
    // printf("master table has %u records\n", record_count);
    uint record_base_offset = 13;
    for(int i = 0; i < record_count; i++){
        long long key = readlong(master_table, record_base_offset + i * 13);
        (void)key; // unused
        uint rec_offset = readint(master_table, record_base_offset + i * 13 + 8);
        byte ind_flag = master_table[record_base_offset + i * 13 + 12];
        // printf("record %d: key=%016llx, offset=%08x, ind_flag=%02x\n", i, key, rec_offset, ind_flag);

        if(ind_flag == 0){
            //the record has been stored within a chained DBBuffer at rec_offset
            byte* record = master_table + rec_offset;
            uint table_name_len = readint(record, 0);
            // printf("processing table %.*s at %08x\n", table_name_len, record + 4, rec_offset);
            if(table_name_len != target_len){
                continue;
            }
            if(memcmp(record + 4, table_name, table_name_len) != 0){
                continue;
            }
            //we've found the table of interest. Let's actually fill out tabledata.
            memcpy(data->name, table_name, target_len);
            data->name[target_len] = '\0';
            
            record += table_name_len + 4;
            printf("Found table: %s at %08x\n", table_name, rec_offset);
            data->schema_version = readint(record, 0);
            data->root_buffer_id = readint(record, 4);
            data->key_type = record[8];
            data->schema_field_types_len = readint(record, 9);
            if (data->schema_field_types_len > 0) {
                data->schema_field_types = malloc(data->schema_field_types_len);
                memcpy(data->schema_field_types, record + 13, data->schema_field_types_len);
            }
            record = record + data->schema_field_types_len + 13;
            data->schema_field_names_len = readint(record, 0);
            if (data->schema_field_names_len > 0) {
                data->schema_field_names = malloc(data->schema_field_names_len);
                memcpy(data->schema_field_names, record + 4, data->schema_field_names_len);
            }
            record = record + data->schema_field_names_len + 4;
            data->index_column = readint(record, 0);
            data->max_key = readlong(record, 4);
            data->record_count = readint(record, 12);
            free(master_table);
            return 0;
        }
    }

    free(master_table);
    return 1;
}

void print_tabledata(tabledata * data) {
    printf("tabledata:\n");
    printf("  name: %s\n", data->name);
    printf("  schema version: %u\n", data->schema_version);
    printf("  root buffer id: %u\n", data->root_buffer_id);
    printf("  key type: %u\n", data->key_type);
    printf("  schema field types length: %u\n", data->schema_field_types_len);
    printf("  schema field names length: %u\n", data->schema_field_names_len);
    printf("  index column: %u\n", data->index_column);
    printf("  max key: %lld\n", data->max_key);
    printf("  record count: %u\n", data->record_count);
}
void free_tabledata(tabledata * data) {
    if (data->schema_field_types_len > 0) {
        free(data->schema_field_types);
    }
    if (data->schema_field_names_len > 0) {
        free(data->schema_field_names);
    }

}