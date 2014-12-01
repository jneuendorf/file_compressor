#ifndef _COMPRESS
#define _COMPRESS

#include "global.h"
#include "compressor.h"


void read_data(FILE *file, struct nsb_data *nsb_datas, number num_blocks, number **nsb_order_p, number *nsb_arrays_lengths, number **nsb_permutations);


void calc_rem_data(struct nsb_data *nsb_datas, number *max_used_nsb, number *nsb_arrays_lengths);


void write_data_to_bit_stream(struct bit_stream *bit_stream, struct nsb_data *nsb_datas, number num_blocks, number max_used_nsb, number *nsb_order, number *nsb_arrays_lengths, number *nsb_offsets);


bool write_data_to_file(char const filename[], struct bit_stream *bit_stream);

#endif
