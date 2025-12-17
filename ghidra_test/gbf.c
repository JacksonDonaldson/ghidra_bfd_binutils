#include <stdio.h>
#include <stdlib.h>
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

    byte node_type = master_table[0];
    if (node_type != 0x01) {
        fprintf(stderr, "Error: expected master to be node type LONGKEY_VAR_REC_NODE \n");
        exit(1);
    }

    uint record_count = readint(master_table, 1);

    uint record_base_offset = 13;
    for(int i = 0; i < record_count; i++){
        long long key = readlong(master_table, record_base_offset + i * 13);
        uint rec_offset = readint(master_table, record_base_offset + i * 13 + 8);
        byte ind_flag = master_table[record_base_offset + i * 13 + 12];

        if(ind_flag != 0){
            //the record has been stored within a chained DBBuffer at rec_offset
            byte* record = master_table + rec_offset;
            uint table_name_len = readint(record, 0);
            if(table_name_len != strlen(table_name)){
                continue;
            }
            if(memcmp(record + 4, table_name, table_name_len) != 0){
                continue;
            }
            //we've found the table of interest. Let's actually fill out tabledata.
            
            //todo


        }
    }

    free(master_table);
}