/**
 * lzsa_wrapper.c - Implementation of clean LZSA wrapper API
 *
 * This file wraps Emmanuel Marty's LZSA library with a clean,
 * Rust-friendly C API.
 */

#include "lzsa_wrapper.h"
#include "lib.h"
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

lzsa_options_t lzsa_get_default_options(void) {
    lzsa_options_t options;
    options.version = LZSA_VERSION_1;
    options.mode = LZSA_MODE_NORMAL;
    options.quality = LZSA_QUALITY_RATIO;
    options.min_match_size = 3;
    return options;
}

size_t lzsa_get_max_compressed_size(
    size_t input_size,
    lzsa_version_t version,
    lzsa_mode_t mode)
{
    (void)version;  /* Unused - library function doesn't need it */
    (void)mode;     /* Unused - library function doesn't need it */

    /* Use the library's function */
    return lzsa_get_max_compressed_size_inmem(input_size);
}

size_t lzsa_get_max_decompressed_size(
    const uint8_t* compressed_data,
    size_t compressed_size)
{
    if (!compressed_data) {
        return 0;
    }

    size_t result = lzsa_get_max_decompressed_size_inmem(
        (const unsigned char*)compressed_data,
        compressed_size
    );

    /* Library returns (size_t)-1 on error, we return 0 */
    if (result == (size_t)-1) {
        return 0;
    }

    return result;
}

/* ============================================================================
 * Internal Helper
 * ============================================================================ */

static unsigned int lzsa_options_to_flags(const lzsa_options_t* options) {
    unsigned int flags = 0;

    if (options->quality == LZSA_QUALITY_RATIO) {
        flags |= LZSA_FLAG_FAVOR_RATIO;
    }

    if (options->mode == LZSA_MODE_RAW_FORWARD) {
        flags |= LZSA_FLAG_RAW_BLOCK;
    }

    return flags;
}

/* ============================================================================
 * Compression API
 * ============================================================================ */

lzsawrap_error_t lzsa_compress(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size,
    const lzsa_options_t* options)
{
    /* Validate inputs */
    if (!input) return LZSAWRAP_ERR_INPUT_NULL;
    if (!output) return LZSAWRAP_ERR_OUTPUT_NULL;
    if (!output_size) return LZSAWRAP_ERR_OUTPUT_SIZE_NULL;
    if (!options) return LZSAWRAP_ERR_INVALID_MODE;

    /* Validate version */
    if (options->version != LZSA_VERSION_1 && options->version != LZSA_VERSION_2) {
        return LZSAWRAP_ERR_INVALID_VERSION;
    }

    /* Handle empty input */
    if (input_size == 0) {
        *output_size = 0;
        return LZSAWRAP_OK;
    }

    /* Raw blocks have a size limit of 64KB */
    if (options->mode == LZSA_MODE_RAW_FORWARD && input_size > 65536) {
        return LZSAWRAP_ERR_COMPRESSION_FAILED;
    }

    /* Convert options to flags */
    unsigned int flags = lzsa_options_to_flags(options);
    int min_match = (int)options->min_match_size;
    if (min_match < 3) min_match = 3;
    if (min_match > 5) min_match = 5;

    /* lzsa_compress_inmem modifies the input buffer when using backward mode,
     * so we ALWAYS need to make a copy for safety */
    unsigned char* input_copy = (unsigned char*)malloc(input_size);
    if (!input_copy) {
        return LZSAWRAP_ERR_OUT_OF_MEMORY;
    }
    memcpy(input_copy, input, input_size);

    /* Call the library function */
    size_t result = lzsa_compress_inmem(
        input_copy,
        output,
        input_size,
        *output_size,
        flags,
        min_match,
        (int)options->version
    );

    free(input_copy);

    /* Check for error */
    if (result == (size_t)-1) {
        return LZSAWRAP_ERR_COMPRESSION_FAILED;
    }

    *output_size = result;
    return LZSAWRAP_OK;
}

lzsawrap_error_t lzsa_compress_v1(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    lzsa_options_t options = lzsa_get_default_options();
    options.version = LZSA_VERSION_1;
    return lzsa_compress(input, input_size, output, output_size, &options);
}

lzsawrap_error_t lzsa_compress_v2(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    lzsa_options_t options = lzsa_get_default_options();
    options.version = LZSA_VERSION_2;
    return lzsa_compress(input, input_size, output, output_size, &options);
}

/* ============================================================================
 * Decompression API
 * ============================================================================ */

lzsawrap_error_t lzsa_decompress(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size,
    lzsa_version_t* detected_version)
{
    /* Validate inputs */
    if (!input) return LZSAWRAP_ERR_INPUT_NULL;
    if (!output) return LZSAWRAP_ERR_OUTPUT_NULL;
    if (!output_size) return LZSAWRAP_ERR_OUTPUT_SIZE_NULL;

    /* Handle empty input */
    if (input_size == 0) {
        *output_size = 0;
        if (detected_version) {
            *detected_version = LZSA_VERSION_1;
        }
        return LZSAWRAP_OK;
    }

    /* Make a mutable copy for the library (it modifies input for some reason) */
    unsigned char* input_copy = (unsigned char*)malloc(input_size);
    if (!input_copy) {
        return LZSAWRAP_ERR_OUT_OF_MEMORY;
    }
    memcpy(input_copy, input, input_size);

    /* Let the library detect the version */
    int format_version = 0; /* Will be set by library */

    /* Call the library function */
    size_t result = lzsa_decompress_inmem(
        input_copy,
        (unsigned char*)output,
        input_size,
        *output_size,
        0, /* flags - normal decompression */
        &format_version
    );

    free(input_copy);

    /* Check for error */
    if (result == (size_t)-1) {
        return LZSAWRAP_ERR_DECOMPRESSION_FAILED;
    }

    *output_size = result;

    if (detected_version) {
        *detected_version = (format_version == 1) ? LZSA_VERSION_1 : LZSA_VERSION_2;
    }

    return LZSAWRAP_OK;
}

lzsawrap_error_t lzsa_decompress_v1(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    lzsa_version_t version;
    lzsawrap_error_t result = lzsa_decompress(input, input_size, output, output_size, &version);

    if (result == LZSAWRAP_OK && version != LZSA_VERSION_1) {
        return LZSAWRAP_ERR_INVALID_FORMAT;
    }

    return result;
}

lzsawrap_error_t lzsa_decompress_v2(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    lzsa_version_t version;
    lzsawrap_error_t result = lzsa_decompress(input, input_size, output, output_size, &version);

    if (result == LZSAWRAP_OK && version != LZSA_VERSION_2) {
        return LZSAWRAP_ERR_INVALID_FORMAT;
    }

    return result;
}

lzsawrap_error_t lzsa_decompress_v1_backward(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    /* Validate inputs */
    if (!input) return LZSAWRAP_ERR_INPUT_NULL;
    if (!output) return LZSAWRAP_ERR_OUTPUT_NULL;
    if (!output_size) return LZSAWRAP_ERR_OUTPUT_SIZE_NULL;

    if (input_size == 0) {
        *output_size = 0;
        return LZSAWRAP_OK;
    }

    /* For backward raw blocks, we need a large enough buffer */
    if (*output_size < 65536) {
        return LZSAWRAP_ERR_BUFFER_TOO_SMALL;
    }

    /* Make mutable copy - library modifies the buffer! */
    unsigned char* input_copy = (unsigned char*)malloc(input_size);
    if (!input_copy) {
        return LZSAWRAP_ERR_OUT_OF_MEMORY;
    }
    memcpy(input_copy, input, input_size);

    int format_version = LZSA_VERSION_1;

    /* Decompress with backward flag and RAW_BLOCK flag */
    size_t result = lzsa_decompress_inmem(
        input_copy,
        (unsigned char*)output,
        input_size,
        *output_size,
        LZSA_FLAG_RAW_BACKWARD | LZSA_FLAG_RAW_BLOCK,
        &format_version
    );

    free(input_copy);

    if (result == (size_t)-1) {
        return LZSAWRAP_ERR_DECOMPRESSION_FAILED;
    }

    *output_size = result;
    return LZSAWRAP_OK;
}

lzsawrap_error_t lzsa_decompress_v2_backward(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size)
{
    /* Validate inputs */
    if (!input) return LZSAWRAP_ERR_INPUT_NULL;
    if (!output) return LZSAWRAP_ERR_OUTPUT_NULL;
    if (!output_size) return LZSAWRAP_ERR_OUTPUT_SIZE_NULL;

    if (input_size == 0) {
        *output_size = 0;
        return LZSAWRAP_OK;
    }

    /* For backward raw blocks, we need a large enough buffer */
    if (*output_size < 65536) {
        return LZSAWRAP_ERR_BUFFER_TOO_SMALL;
    }

    /* Make mutable copy - library modifies the buffer! */
    unsigned char* input_copy = (unsigned char*)malloc(input_size);
    if (!input_copy) {
        return LZSAWRAP_ERR_OUT_OF_MEMORY;
    }
    memcpy(input_copy, input, input_size);

    int format_version = LZSA_VERSION_2;

    /* Decompress with backward flag and RAW_BLOCK flag */
    size_t result = lzsa_decompress_inmem(
        input_copy,
        (unsigned char*)output,
        input_size,
        *output_size,
        LZSA_FLAG_RAW_BACKWARD | LZSA_FLAG_RAW_BLOCK,
        &format_version
    );

    free(input_copy);

    if (result == (size_t)-1) {
        return LZSAWRAP_ERR_DECOMPRESSION_FAILED;
    }

    *output_size = result;
    return LZSAWRAP_OK;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

const char* lzsa_error_string(lzsawrap_error_t error) {
    switch (error) {
        case LZSAWRAP_OK: return "Success";
        case LZSAWRAP_ERR_INPUT_NULL: return "Input pointer is NULL";
        case LZSAWRAP_ERR_OUTPUT_NULL: return "Output pointer is NULL";
        case LZSAWRAP_ERR_OUTPUT_SIZE_NULL: return "Output size pointer is NULL";
        case LZSAWRAP_ERR_BUFFER_TOO_SMALL: return "Output buffer too small";
        case LZSAWRAP_ERR_COMPRESSION_FAILED: return "Compression failed";
        case LZSAWRAP_ERR_DECOMPRESSION_FAILED: return "Decompression failed";
        case LZSAWRAP_ERR_INVALID_FORMAT: return "Invalid or corrupted format";
        case LZSAWRAP_ERR_INVALID_VERSION: return "Invalid version specified";
        case LZSAWRAP_ERR_OUT_OF_MEMORY: return "Out of memory";
        case LZSAWRAP_ERR_INVALID_MODE: return "Invalid compression mode";
        default: return "Unknown error";
    }
}

const char* lzsa_version_string(void) {
    return "LZSA 1.4.1 (wrapper)";
}
