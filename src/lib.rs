//! # lzsa-sys
//!
//! Rust bindings for the LZSA compression library by Emmanuel Marty.
//!
//! Supports both LZSA1 and LZSA2 formats.
//!
//! ## Example
//!
//! ```rust
//! use lzsa_sys::{compress_v1, decompress};
//!
//! let original = b"Hello, world! This is test data.";
//! let compressed = compress_v1(original)?;
//! let decompressed = decompress(&compressed)?;
//! assert_eq!(original, decompressed.as_slice());
//! # Ok::<(), lzsa_sys::Error>(())
//! ```

use std::os::raw::c_int;

/// LZSA compression format version
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(i32)]
pub enum Version {
    /// LZSA1 format - faster decompression
    V1 = 1,
    /// LZSA2 format - better compression ratio
    V2 = 2,
}

/// Compression mode
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(i32)]
pub enum Mode {
    /// Forward compression with framing (default)
    Normal = 0,
    /// Forward compression, raw block (no frame header)
    RawForward = 1,
}

/// Compression quality setting
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(i32)]
pub enum Quality {
    /// Favor decompression speed
    Speed = 0,
    /// Favor compression ratio (default)
    Ratio = 1,
}

/// Compression options
#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct Options {
    pub version: Version,
    pub mode: Mode,
    pub quality: Quality,
    pub min_match_size: u32,
}

impl Default for Options {
    fn default() -> Self {
        Self {
            version: Version::V1,
            mode: Mode::Normal,
            quality: Quality::Ratio,
            min_match_size: 3,
        }
    }
}

/// Error type for LZSA operations
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Error {
    InputNull,
    OutputNull,
    OutputSizeNull,
    BufferTooSmall,
    CompressionFailed,
    DecompressionFailed,
    InvalidFormat,
    InvalidVersion,
    OutOfMemory,
    InvalidMode,
    Unknown(i32),
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::InputNull => write!(f, "Input pointer is NULL"),
            Self::OutputNull => write!(f, "Output pointer is NULL"),
            Self::OutputSizeNull => write!(f, "Output size pointer is NULL"),
            Self::BufferTooSmall => write!(f, "Output buffer too small"),
            Self::CompressionFailed => write!(f, "Compression failed"),
            Self::DecompressionFailed => write!(f, "Decompression failed"),
            Self::InvalidFormat => write!(f, "Invalid or corrupted format"),
            Self::InvalidVersion => write!(f, "Invalid version specified"),
            Self::OutOfMemory => write!(f, "Out of memory"),
            Self::InvalidMode => write!(f, "Invalid compression mode"),
            Self::Unknown(code) => write!(f, "Unknown error: {}", code),
        }
    }
}

impl std::error::Error for Error {}

impl From<i32> for Error {
    fn from(code: i32) -> Self {
        match code {
            -1 => Self::InputNull,
            -2 => Self::OutputNull,
            -3 => Self::OutputSizeNull,
            -4 => Self::BufferTooSmall,
            -5 => Self::CompressionFailed,
            -6 => Self::DecompressionFailed,
            -7 => Self::InvalidFormat,
            -8 => Self::InvalidVersion,
            -9 => Self::OutOfMemory,
            -10 => Self::InvalidMode,
            _ => Self::Unknown(code),
        }
    }
}

pub type Result<T> = std::result::Result<T, Error>;

// FFI bindings
unsafe extern "C" {
    #[allow(dead_code)]
    fn lzsa_get_default_options() -> Options;

    fn lzsa_get_max_compressed_size(
        input_size: usize,
        version: c_int,
        mode: c_int,
    ) -> usize;

    fn lzsa_get_max_decompressed_size(
        compressed_data: *const u8,
        compressed_size: usize,
    ) -> usize;

    fn lzsa_compress(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
        options: *const Options,
    ) -> c_int;

    fn lzsa_compress_v1(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
    ) -> c_int;

    fn lzsa_compress_v2(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
    ) -> c_int;

    fn lzsa_decompress(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
        detected_version: *mut c_int,
    ) -> c_int;

    fn lzsa_decompress_v1(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
    ) -> c_int;

    fn lzsa_decompress_v2(
        input: *const u8,
        input_size: usize,
        output: *mut u8,
        output_size: *mut usize,
    ) -> c_int;
}

// High-level Rust API

/// Compress data with custom options
pub fn compress_with_options(input: &[u8], options: &Options) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_compressed_size(
            input.len(),
            options.version as c_int,
            options.mode as c_int,
        );

        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;

        let result = lzsa_compress(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
            options as *const Options,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

/// Compress with LZSA1 (convenience function)
pub fn compress_v1(input: &[u8]) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_compressed_size(input.len(), 1, 0);
        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;

        let result = lzsa_compress_v1(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

/// Compress with LZSA2 (convenience function)
pub fn compress_v2(input: &[u8]) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_compressed_size(input.len(), 2, 0);
        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;

        let result = lzsa_compress_v2(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

/// Decompress data (auto-detects LZSA1 or LZSA2)
pub fn decompress(input: &[u8]) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_decompressed_size(input.as_ptr(), input.len());

        if max_size == 0 {
            return Err(Error::DecompressionFailed);
        }

        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;
        let mut detected_version: c_int = 0;

        let result = lzsa_decompress(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
            &mut detected_version,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

/// Decompress LZSA1 data
pub fn decompress_v1(input: &[u8]) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_decompressed_size(input.as_ptr(), input.len());

        if max_size == 0 {
            return Err(Error::DecompressionFailed);
        }

        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;

        let result = lzsa_decompress_v1(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

/// Decompress LZSA2 data
pub fn decompress_v2(input: &[u8]) -> Result<Vec<u8>> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    unsafe {
        let max_size = lzsa_get_max_decompressed_size(input.as_ptr(), input.len());

        if max_size == 0 {
            return Err(Error::DecompressionFailed);
        }

        let mut output = vec![0u8; max_size];
        let mut output_size = max_size;

        let result = lzsa_decompress_v2(
            input.as_ptr(),
            input.len(),
            output.as_mut_ptr(),
            &mut output_size,
        );

        if result != 0 {
            return Err(Error::from(result));
        }

        output.truncate(output_size);
        Ok(output)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compress_decompress_v1() {
        let original = b"Hello, world! This is a test.";
        let compressed = compress_v1(original).unwrap();
        let decompressed = decompress(&compressed).unwrap();
        assert_eq!(original, decompressed.as_slice());
    }

    #[test]
    fn test_compress_decompress_v2() {
        let original = b"Hello, world! This is a test.";
        let compressed = compress_v2(original).unwrap();
        let decompressed = decompress(&compressed).unwrap();
        assert_eq!(original, decompressed.as_slice());
    }

    #[test]
    fn test_custom_options() {
        let original = b"Custom options test data";
        let options = Options {
            version: Version::V2,
            mode: Mode::Normal,
            quality: Quality::Speed,
            min_match_size: 3,
        };
        let compressed = compress_with_options(original, &options).unwrap();
        let decompressed = decompress(&compressed).unwrap();
        assert_eq!(original, decompressed.as_slice());
    }

    #[test]
    fn test_empty_input() {
        let empty: &[u8] = &[];
        let compressed = compress_v1(empty).unwrap();
        assert_eq!(compressed.len(), 0);
        let decompressed = decompress(&compressed).unwrap();
        assert_eq!(decompressed.len(), 0);
    }
}
