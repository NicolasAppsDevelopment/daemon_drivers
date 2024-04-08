/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_common.h"
#include "sensirion_config.h"

UShort SensirionCommon::sensirion_common_bytes_to_UShort(const Byte* bytes) {
    return (UShort)bytes[0] << 8 | (UShort)bytes[1];
}

UInt SensirionCommon::sensirion_common_bytes_to_UInt(const Byte* bytes) {
    return (UInt)bytes[0] << 24 | (UInt)bytes[1] << 16 |
           (UInt)bytes[2] << 8 | (UInt)bytes[3];
}

int16_t SensirionCommon::sensirion_common_bytes_to_int16_t(const Byte* bytes) {
    return (int16_t)sensirion_common_bytes_to_UShort(bytes);
}

int32_t SensirionCommon::sensirion_common_bytes_to_int32_t(const Byte* bytes) {
    return (int32_t)sensirion_common_bytes_to_UInt(bytes);
}

float SensirionCommon::sensirion_common_bytes_to_float(const Byte* bytes) {
    union {
        UInt u32_value;
        float float32;
    } tmp;

    tmp.u32_value = sensirion_common_bytes_to_UInt(bytes);
    return tmp.float32;
}

void SensirionCommon::sensirion_common_UInt_to_bytes(const UInt value, Byte* bytes) {
    bytes[0] = value >> (UInt)24;
    bytes[1] = value >> (UInt)16;
    bytes[2] = value >> (UInt)8;
    bytes[3] = value;
}

void SensirionCommon::sensirion_common_UShort_to_bytes(const UShort value, Byte* bytes) {
    bytes[0] = value >> (UShort)8;
    bytes[1] = value;
}

void SensirionCommon::sensirion_common_int32_t_to_bytes(const int32_t value, Byte* bytes) {
    bytes[0] = value >> (int32_t)24;
    bytes[1] = value >> (int32_t)16;
    bytes[2] = value >> (int32_t)8;
    bytes[3] = value;
}

void SensirionCommon::sensirion_common_int16_t_to_bytes(const int16_t value, Byte* bytes) {
    bytes[0] = value >> (int16_t)8;
    bytes[1] = value;
}

void SensirionCommon::sensirion_common_float_to_bytes(const float value, Byte* bytes) {
    union {
        UInt u32_value;
        float float32;
    } tmp;
    tmp.float32 = value;
    sensirion_common_UInt_to_bytes(tmp.u32_value, bytes);
}

void SensirionCommon::sensirion_common_copy_bytes(const Byte* source, Byte* destination,
                                 UShort data_length) {
    UShort i;
    for (i = 0; i < data_length; i++) {
        destination[i] = source[i];
    }
}
