#include "decompress.h"




bool read_compressed_data(char const filename[], struct bit_stream *bit_stream) {
    unsigned char *block_buffer;
    // unsigned char *buffer;
    unsigned char *p;
    FILE *file;
    number file_size;
    number memory_block_size;
    number i;
    number bytes_read;
    number temp;
    number block_index;


    // try to open the file
    file = fopen(filename, "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", filename);
        return false;
    }

    file_size = get_file_size(file);

    // get minimum required number of bytes we need for reading the file
    memory_block_size = file_size <= settings.memory_block_size ? file_size : settings.memory_block_size;
    D(printf("memory block size = %llu\n", memory_block_size);)

    // allocate memory
    block_buffer = (unsigned char *) calloc(memory_block_size, sizeof(char));
    // buffer = (unsigned char *) calloc(settings.block_size / 8 + 1, sizeof(char));

    // create empty bit stream
    *bit_stream = create_bs(0, 0);

    block_index = 0;

    // read file til its end (-> feof = end of file)
    while(!feof(file)) {
        bytes_read = fread(block_buffer, memory_block_size, 1, file);

        D(printf("bytes_read = %llu, file_size = %llu\n", bytes_read, file_size);)

        // this happens on the last block
        if(bytes_read == 0) {
            break;
        }

        // read block into bit stream
        p = block_buffer;
        for(i = 0; i < memory_block_size; ++i) {
            // TODO: maybe improve this...gather 8 bytes and append a fully used number to bit stream instead of just 1 byte at a time
            // TODO: if doing so, swap blocks if on little endian machine

            D(printf("reading %x = %d (i = %llu)\n", *p, *p, i);)

            temp = (number) *p;
            append_num_to_bs(bit_stream, &temp, 8);

            // point to next byte
            p++;
        }

        if(block_index != 0) {

        }
        // first block => special treatment because meta data and mapper are there
        // TODO: smallest memory_block_size is 1 MB => check if meta data + mapper always fits in there (i guess yes)
        else {
            // 3 is hardcoded here (see beginning of write_data_to_bit_stream() in compress.c)
            settings.block_size = read_bs(bit_stream, 3);

        }

        // analyse bit stream and get data

    }


    fclose(file);

    return true;
}
