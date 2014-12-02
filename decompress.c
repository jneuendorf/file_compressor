#include "decompress.h"




bool read_compressed_data(char const filename[], struct bit_stream *bit_stream) {
    char *block_buffer;
    char *buffer;
    char *p;
    FILE *file;
    number file_size;
    number memory_block_size;
    number i;


    // try to open the file
    file = fopen(filename, "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", filename);
        return false;
    }

    file_size = get_file_size(file);

    // get minimum required number of bytes we need for reading the file
    memory_block_size = file_size <= settings.memory_block_size ? file_size : settings.memory_block_size;

    // allocate memory
    block_buffer = (char *) calloc(memory_block_size, sizeof(char));
    buffer = (char *) calloc(settings.block_size / 8 + 1, sizeof(char));

    // create empty bit stream
    *bit_stream = create_bs(0, 0);

    // read file til its end (-> feof = end of file)
    while(!feof(file)) {
        fread(block_buffer, memory_block_size, 1, file);

        p = block_buffer;
        for(i = 0; i < memory_block_size; ++i) {
            // TODO: maybe improve this...gather 8 bytes and append a fully used number to bit stream instead of just 1 byte at a time
            append_num_to_bs(bit_stream, (number *) p, sizeof(char));
            // point to next byte
            p++;
        }
    }


    fclose(file);

    return true;
}
