/**
 * lzsa_wrapper.h - Clean C API wrapper for LZSA library
 *
 * This header provides a simplified, Rust-friendly API for the LZSA
 * compression library by Emmanuel Marty.
 */

#ifndef LZSA_WRAPPER_H
#define LZSA_WRAPPER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Types and Constants
 * ============================================================================ */

/**
 * LZSA format version
 */
typedef enum {
    LZSA_VERSION_1 = 1,  /* LZSA1 format */
    LZSA_VERSION_2 = 2,  /* LZSA2 format */
} lzsa_version_t;

/**
 * Compression mode
 */
typedef enum {
    LZSA_MODE_NORMAL = 0,     /* Forward compression, framed output */
    LZSA_MODE_RAW_FORWARD,    /* Forward compression, raw block (no frame) */
} lzsa_mode_t;

/**
 * Compression quality level
 */
typedef enum {
    LZSA_QUALITY_SPEED = 0,   /* Favor decompression speed */
    LZSA_QUALITY_RATIO = 1,   /* Favor compression ratio (default) */
} lzsa_quality_t;

/**
 * Error codes (using LZSAWRAP_ prefix to avoid collision with original library)
 */
typedef enum {
    LZSAWRAP_OK = 0,
    LZSAWRAP_ERR_INPUT_NULL = -1,
    LZSAWRAP_ERR_OUTPUT_NULL = -2,
    LZSAWRAP_ERR_OUTPUT_SIZE_NULL = -3,
    LZSAWRAP_ERR_BUFFER_TOO_SMALL = -4,
    LZSAWRAP_ERR_COMPRESSION_FAILED = -5,
    LZSAWRAP_ERR_DECOMPRESSION_FAILED = -6,
    LZSAWRAP_ERR_INVALID_FORMAT = -7,
    LZSAWRAP_ERR_INVALID_VERSION = -8,
    LZSAWRAP_ERR_OUT_OF_MEMORY = -9,
    LZSAWRAP_ERR_INVALID_MODE = -10,
} lzsawrap_error_t;

/**
 * Compression options
 */
typedef struct {
    lzsa_version_t version;   /* Format version (1 or 2) */
    lzsa_mode_t mode;         /* Compression mode */
    lzsa_quality_t quality;   /* Quality/speed tradeoff */
    uint32_t min_match_size;  /* Minimum match size (3-5, default 3) */
} lzsa_options_t;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Get default compression options
 *
 * @return Default options (LZSA1, normal mode, ratio quality)
 */
lzsa_options_t lzsa_get_default_options(void);

/**
 * Get maximum compressed size for given input
 *
 * @param input_size Size of input data in bytes
 * @param version Format version
 * @param mode Compression mode
 * @return Maximum possible compressed size
 */
size_t lzsa_get_max_compressed_size(
    size_t input_size,
    lzsa_version_t version,
    lzsa_mode_t mode
);

/**
 * Get maximum decompressed size from compressed data
 *
 * @param compressed_data Compressed data
 * @param compressed_size Size of compressed data
 * @return Maximum decompressed size, or 0 on error
 */
size_t lzsa_get_max_decompressed_size(
    const uint8_t* compressed_data,
    size_t compressed_size
);

/* ============================================================================
 * Compression API
 * ============================================================================ */

/**
 * Compress data with custom options
 *
 * @param input Input data to compress
 * @param input_size Size of input data
 * @param output Output buffer for compressed data
 * @param output_size Pointer to output buffer size (in: max size, out: actual size)
 * @param options Compression options
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_compress(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size,
    const lzsa_options_t* options
);

/**
 * Compress with LZSA1 (convenience function)
 *
 * @param input Input data
 * @param input_size Size of input
 * @param output Output buffer
 * @param output_size Pointer to output size
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_compress_v1(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size
);

/**
 * Compress with LZSA2 (convenience function)
 *
 * @param input Input data
 * @param input_size Size of input
 * @param output Output buffer
 * @param output_size Pointer to output size
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_compress_v2(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size
);

/* ============================================================================
 * Decompression API
 * ============================================================================ */

/**
 * Decompress data (auto-detect format)
 *
 * Automatically detects LZSA1 or LZSA2 from input data.
 *
 * @param input Compressed data
 * @param input_size Size of compressed data
 * @param output Output buffer for decompressed data
 * @param output_size Pointer to output size (in: max size, out: actual size)
 * @param detected_version Pointer to store detected version (can be NULL)
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_decompress(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size,
    lzsa_version_t* detected_version
);

/**
 * Decompress with LZSA1 (if you know the format)
 *
 * @param input Compressed data
 * @param input_size Size of compressed data
 * @param output Output buffer
 * @param output_size Pointer to output size
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_decompress_v1(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size
);

/**
 * Decompress with LZSA2
 *
 * @param input Compressed data
 * @param input_size Size of compressed data
 * @param output Output buffer
 * @param output_size Pointer to output size
 * @return LZSAWRAP_OK on success, error code on failure
 */
lzsawrap_error_t lzsa_decompress_v2(
    const uint8_t* input,
    size_t input_size,
    uint8_t* output,
    size_t* output_size
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get error message string
 *
 * @param error Error code
 * @return Human-readable error message
 */
const char* lzsa_error_string(lzsawrap_error_t error);

/**
 * Get version information
 *
 * @return Version string (e.g., "LZSA 1.4.1")
 */
const char* lzsa_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* LZSA_WRAPPER_H */
