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

#ifndef SENSIRION_COMMON_H
#define SENSIRION_COMMON_H

#include "sensirion_config.h"
#include "../types.h"

#define NO_ERROR 0
#define NOT_IMPLEMENTED_ERROR 31

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

#define SENSIRION_COMMAND_SIZE 2
#define SENSIRION_WORD_SIZE 2
#define SENSIRION_NUM_WORDS(x) (sizeof(x) / SENSIRION_WORD_SIZE)
#define SENSIRION_MAX_BUFFER_WORDS 32

/**
* SensirionCommon - Sensirion driver base class for data type convertion
* @see https://github.com/Sensirion/raspberry-pi-i2c-stc3x 
*/
class SensirionCommon
{
    public:
    /**
    * sensirion_common_bytes_to_int16_t() - Convert an array of bytes to an int16_t
    *
    * Convert an array of bytes received from the sensor in big-endian/MSB-first
    * format to an int16_t value in the correct system-endianness.
    *
    * @param bytes An array of at least two bytes (MSB first)
    * @return      The byte array represented as int16_t
    */
    static int16_t sensirion_common_bytes_to_int16_t(const Byte* bytes);

    /**
     * sensirion_common_bytes_to_int32_t() - Convert an array of bytes to an int32_t
     *
     * Convert an array of bytes received from the sensor in big-endian/MSB-first
     * format to an int32_t value in the correct system-endianness.
     *
     * @param bytes An array of at least four bytes (MSB first)
     * @return      The byte array represented as int32_t
     */
    static int32_t sensirion_common_bytes_to_int32_t(const Byte* bytes);

    /**
     * sensirion_common_bytes_to_UShort() - Convert an array of bytes to an
     * UShort
     *
     * Convert an array of bytes received from the sensor in big-endian/MSB-first
     * format to an UShort value in the correct system-endianness.
     *
     * @param bytes An array of at least two bytes (MSB first)
     * @return      The byte array represented as UShort
     */
    static UShort sensirion_common_bytes_to_UShort(const Byte* bytes);

    /**
     * sensirion_common_bytes_to_UInt() - Convert an array of bytes to an
     * UInt
     *
     * Convert an array of bytes received from the sensor in big-endian/MSB-first
     * format to an UInt value in the correct system-endianness.
     *
     * @param bytes An array of at least four bytes (MSB first)
     * @return      The byte array represented as UInt
     */
    static UInt sensirion_common_bytes_to_UInt(const Byte* bytes);

    /**
     * sensirion_common_bytes_to_float() - Convert an array of bytes to a float
     *
     * Convert an array of bytes received from the sensor in big-endian/MSB-first
     * format to an float value in the correct system-endianness.
     *
     * @param bytes An array of at least four bytes (MSB first)
     * @return      The byte array represented as float
     */
    static float sensirion_common_bytes_to_float(const Byte* bytes);

    /**
     * sensirion_common_UInt_to_bytes() - Convert an UInt to an array of
     * bytes
     *
     * Convert an UInt value in system-endianness to big-endian/MBS-first
     * format to send to the sensor.
     *
     * @param value Value to convert
     * @param bytes An array of at least four bytes
     */
    static void sensirion_common_UInt_to_bytes(const UInt value, Byte* bytes);

    /**
     * sensirion_common_UShort_to_bytes() - Convert an UShort to an array of
     * bytes
     *
     * Convert an UShort value in system-endianness to big-endian/MBS-first
     * format to send to the sensor.
     *
     * @param value Value to convert
     * @param bytes An array of at least two bytes
     */
    static void sensirion_common_UShort_to_bytes(const UShort value, Byte* bytes);

    /**
     * sensirion_common_int32_t_to_bytes() - Convert an int32_t to an array of bytes
     *
     * Convert an int32_t value in system-endianness to big-endian/MBS-first
     * format to send to the sensor.
     *
     * @param value Value to convert
     * @param bytes An array of at least four bytes
     */
    static void sensirion_common_int32_t_to_bytes(const int32_t value, Byte* bytes);

    /**
     * sensirion_common_int16_t_to_bytes() - Convert an int16_t to an array of bytes
     *
     * Convert an int16_t value in system-endianness to big-endian/MBS-first
     * format to send to the sensor.
     *
     * @param value Value to convert
     * @param bytes An array of at least two bytes
     */
    static void sensirion_common_int16_t_to_bytes(const int16_t value, Byte* bytes);

    /**
     * sensirion_common_float_to_bytes() - Convert an float to an array of bytes
     *
     * Convert an float value in system-endianness to big-endian/MBS-first
     * format to send to the sensor.
     *
     * @param value Value to convert
     * @param bytes An array of at least four bytes
     */
    static void sensirion_common_float_to_bytes(const float value, Byte* bytes);

    /**
     * sensirion_common_copy_bytes() - Copy bytes from one array to the other.
     *
     * @param source      Array of bytes to be copied.
     * @param destination Array of bytes to be copied to.
     * @param data_length Number of bytes to copy.
     */
    static void sensirion_common_copy_bytes(const Byte* source, Byte* destination,
        UShort data_length);
};
#endif /* SENSIRION_COMMON_H */
