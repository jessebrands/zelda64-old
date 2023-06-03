#include <zlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "patcher.h"

#define CHUNK_SIZE 4096

int decompress_zpf_file(zelda64_decompress_zpf_info_t info) {
    uint8_t in_buf[CHUNK_SIZE] = {0};
    uint8_t tmp_buf[CHUNK_SIZE] = {0};
    size_t bytes_written = 0;
    z_stream strm = {0};
    size_t have = 0;
    int ret = inflateInit(&strm);
    if (ret != Z_OK) {
        return ret;
    }
    do {
        strm.avail_in = fread(in_buf, sizeof(uint8_t), CHUNK_SIZE, info.patch_file);
        if (ferror(info.patch_file)) {
            inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0) {
            break;
        }
        strm.next_in = in_buf;
        do {
            strm.avail_out = CHUNK_SIZE;
            strm.next_out = tmp_buf;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK_SIZE - strm.avail_out;
            if (bytes_written + have > *info.buffer_size) {
                *info.out_buffer = info.realloc_func(*info.out_buffer, bytes_written + have);
                *info.buffer_size = bytes_written + have;
            }
            memcpy((*info.out_buffer) + bytes_written, tmp_buf, have);
            bytes_written += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
