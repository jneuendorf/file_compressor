#ifndef _DECOMPRESS
#define _DECOMPRESS

#include "global.h"
#include "compressor.h"


bool read_compressed_data(char const filename[], struct bit_stream *bit_stream);



#endif
