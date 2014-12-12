#include "decompress.h"




bool read_compressed_data(char const filename[], struct bit_stream *bit_stream) {
    // unsigned char *buffer;
    FILE *file;
    number block_index;
    number bytes_read;
    number file_size;
    number i;
    number max_nsb_bits;
    number memory_block_size;
    number temp;
    unsigned char *block_buffer;
    unsigned char *p;
    unsigned char read_error;
    unsigned char max_used_nsb_bits;
    unsigned char max_avg_idx_bits;


    // try to open the file
    file = fopen(filename, "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", filename);
        return false;
    }

    read_error = 0;
    file_size = get_file_size(file);

    // get minimum required number of bytes we need for reading the file
    memory_block_size = file_size <= settings.memory_block_size ? file_size : settings.memory_block_size;
    D(printf("memory block size = %llu\n", memory_block_size);)

    // allocate memory
    block_buffer = (unsigned char *) calloc(memory_block_size, sizeof(char));

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

        // both are char pointers!
        p = block_buffer;

        // read block into bit stream
        for(i = 0; i < memory_block_size; ++i) {
            // TODO: maybe improve this...gather 8 bytes and append a fully used number to bit stream instead of just 1 byte at a time
            // TODO: if doing so, swap blocks if on little endian machine

            D(printf("reading %x = %d (i = %llu)\n", *p, *p, i);)

            // store byte
            temp = (number) *p;
            // append byte
            append_num_to_bs(bit_stream, &temp, 8);

            // point to next byte
            p++;
        }

        // analyse bit stream and get data
        // non-first block => only compressed data
        if(block_index != 0) {

        }
        // first block => special treatment because meta data and mapper are there
        // TODO: smallest memory_block_size is 1 MB => check if meta data + mapper always fits in there (i guess yes)
        else {
            // get block size from compressed data
            // 3 is hardcoded here (see beginning of write_data_to_bit_stream() in compress.c)
            decompress_data.block_size = (read_bs(bit_stream, 3, &read_error) + 1) * 8;
            // set greatest possible NSB (block_size + 1 because from 0 to block_size)
            decompress_data.max_nsb = decompress_data.block_size + 1;
            max_nsb_bits = NEEDED_BITS(decompress_data.max_nsb);
            //
            decompress_data.num_mapper_entries = read_bs(bit_stream, max_nsb_bits, &read_error);
            // prints 2 -> correct
            decompress_data.max_used_nsb = read_bs(bit_stream, max_nsb_bits, &read_error);

            // set greatest possible permutation index
            decompress_data.max_perm_idx = binom(decompress_data.block_size, decompress_data.block_size / 2);
            // set greatest possible average permutation index (half of max. permutation index)
            decompress_data.max_avg_idx = decompress_data.max_perm_idx / 2;

            max_used_nsb_bits = NEEDED_BITS(decompress_data.max_used_nsb);
            max_avg_idx_bits = NEEDED_BITS(decompress_data.max_avg_idx);
            // read mapper

            // mapper_entry_size = max_used_nsb_bits + 2*max_avg_idx_bits + 1;
            for(nsb = 0; nsb < compress_data.max_nsb; ++nsb) {
                if(nsb_arrays_lengths[nsb] > 0) {
                    // write mapper entry:
                    // nsb
                    append_num_to_bs(bit_stream, &nsb, max_used_nsb_bits);
                    D(printf("appending nsb = %llu (%llu bits)\n", nsb, max_used_nsb_bits);)

                    // average
                    append_num_to_bs(bit_stream, &nsb_datas[nsb].avg, max_avg_idx_bits);
                    D(printf("appending average = %llu (%llu bits)\n", nsb_datas[nsb].avg, max_avg_idx_bits);)

                    // max_diff_bits
                    // D(printf("appending max_diff = %llu (%llu bits)\n", nsb_datas[nsb].max_diff, max_avg_idx_bits);)
                    append_num_to_bs(bit_stream, &nsb_datas[nsb].max_diff_bits, NEEDED_BITS(max_avg_idx_bits));
                    D(printf("appending max_diff_bits = %llu (%llu bits)\n", nsb_datas[nsb].max_diff_bits, (number) NEEDED_BITS(max_avg_idx_bits));)
                }
            }
        }

        // TODO: how do we know what nsb a compressed data block belongs to?

        block_index++;
    }


    fclose(file);

    return true;
}
