# lzsa-sys

[![Build Status](https://github.com/tommyo123/lzsa-sys/workflows/Build%20and%20Test/badge.svg)](https://github.com/tommyo123/lzsa-sys/actions)
[![Crates.io](https://img.shields.io/crates/v/lzsa-sys.svg)](https://crates.io/crates/lzsa-sys)
[![Documentation](https://docs.rs/lzsa-sys/badge.svg)](https://docs.rs/lzsa-sys)
[![License](https://img.shields.io/crates/l/lzsa-sys.svg)](https://github.com/tommyo123/lzsa-sys#license)

Rust FFI bindings for the [LZSA compression library](https://github.com/emmanuel-marty/lzsa) by Emmanuel Marty.

LZSA is a family of byte-aligned compression formats that are specifically optimized for fast decompression on 8-bit systems (such as vintage computers and game consoles). The library provides two formats:

- **LZSA1**: Extremely fast decompression with good compression ratio
- **LZSA2**: Better compression ratio with still very fast decompression

## Features

- ✅ Safe Rust API wrapping the C library
- ✅ Support for both LZSA1 and LZSA2 formats
- ✅ Statically linked (no external dependencies at runtime)
- ✅ Cross-platform (Windows, Linux, macOS)
- ✅ Configurable compression options (quality, min match size)
- ✅ Auto-detection of format version during decompression

## Installation

Add this to your `Cargo.toml`:

```toml
[dependencies]
lzsa-sys = { git = "https://github.com/tommyo123/lzsa-sys" }
```

### Build Requirements

This crate compiles the LZSA C library from source, so you need a C compiler:

- **Windows**: [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) or MinGW
- **Linux**: `sudo apt install build-essential` (Debian/Ubuntu) or equivalent
- **macOS**: `xcode-select --install`

## Usage

### Basic Compression and Decompression

```rust
use lzsa_sys::{compress_v1, decompress};

fn main() -> Result<(), lzsa_sys::Error> {
    let original = b"Hello, world! This is test data for compression.";
    
    // Compress with LZSA1
    let compressed = compress_v1(original)?;
    println!("Original: {} bytes", original.len());
    println!("Compressed: {} bytes", compressed.len());
    
    // Decompress (auto-detects format)
    let decompressed = decompress(&compressed)?;
    assert_eq!(original, decompressed.as_slice());
    
    Ok(())
}
```

### Using LZSA2 Format

```rust
use lzsa_sys::{compress_v2, decompress};

fn main() -> Result<(), lzsa_sys::Error> {
    let original = b"LZSA2 provides better compression ratio!";
    
    let compressed = compress_v2(original)?;
    let decompressed = decompress(&compressed)?;
    
    assert_eq!(original, decompressed.as_slice());
    Ok(())
}
```

### Custom Compression Options

```rust
use lzsa_sys::{compress_with_options, decompress, Options, Version, Mode, Quality};

fn main() -> Result<(), lzsa_sys::Error> {
    let original = b"Custom compression settings example";
    
    let options = Options {
        version: Version::V2,
        mode: Mode::Normal,
        quality: Quality::Ratio,  // Favor compression ratio
        min_match_size: 3,
    };
    
    let compressed = compress_with_options(original, &options)?;
    let decompressed = decompress(&compressed)?;
    
    assert_eq!(original, decompressed.as_slice());
    Ok(())
}
```

## API Documentation

### Compression Functions

- `compress_v1(input: &[u8]) -> Result<Vec<u8>>` - Compress with LZSA1 format
- `compress_v2(input: &[u8]) -> Result<Vec<u8>>` - Compress with LZSA2 format
- `compress_with_options(input: &[u8], options: &Options) -> Result<Vec<u8>>` - Compress with custom settings

### Decompression Functions

- `decompress(input: &[u8]) -> Result<Vec<u8>>` - Decompress (auto-detects LZSA1 or LZSA2)
- `decompress_v1(input: &[u8]) -> Result<Vec<u8>>` - Decompress LZSA1 data
- `decompress_v2(input: &[u8]) -> Result<Vec<u8>>` - Decompress LZSA2 data

### Options

```rust
pub struct Options {
    pub version: Version,        // V1 or V2
    pub mode: Mode,              // Normal or RawForward
    pub quality: Quality,        // Speed or Ratio
    pub min_match_size: u32,     // 3-5, default 3
}
```

## Examples

Run the included examples:

```bash
# Simple compression/decompression
cargo run --example simple

# Compare LZSA1 and LZSA2
cargo run --example all_modes
```

## Performance

LZSA is designed for fast decompression, especially on resource-constrained systems:

- **LZSA1**: Extremely fast decompression (~50% faster than LZ4)
- **LZSA2**: Better compression ratio with still very fast decompression

Both formats are particularly well-suited for:
- Vintage computer systems (8-bit, 16-bit CPUs)
- Embedded systems
- Game asset compression
- Any scenario where decompression speed is critical

## Benchmarks

Example compression ratios on typical data:

| Format | Compression Ratio | Decompression Speed |
|--------|------------------|---------------------|
| LZSA1  | ~50-60%         | Extremely Fast      |
| LZSA2  | ~55-65%         | Very Fast           |

*Actual results vary based on input data characteristics.*

## Building

```bash
# Debug build
cargo build

# Release build (recommended for benchmarks)
cargo build --release

# Run tests
cargo test --release

# Generate documentation
cargo doc --open
```

# License

This project (lzsa-sys) is dual-licensed under:
- **Zlib License** for the Rust bindings and most of the LZSA C code
- **CC0 (Public Domain)** for matchfinder.c

This matches the licensing of the underlying LZSA compression library by Emmanuel Marty.

## lzsa-sys Rust Bindings

Copyright (c) 2025 Tommy Olsen

This Rust wrapper is licensed under the **Zlib License** to match the underlying LZSA library.

## LZSA Compression Library

The LZSA compression library by Emmanuel Marty is dual-licensed:

- **Most of the code**: Zlib License (see [LICENSE-ZLIB.md](LICENSE-ZLIB.md))
- **src/matchfinder.c**: CC0 1.0 Universal - Public Domain (see [LICENSE-CC0.md](LICENSE-CC0.md))

### LZSA Copyright Notice

Copyright (c) 2019 Emmanuel Marty

The LZSA library is available at: https://github.com/emmanuel-marty/lzsa

---

For the full text of each license, please see:
- [LICENSE-ZLIB.md](LICENSE-ZLIB.md) - Zlib License
- [LICENSE-CC0.md](LICENSE-CC0.md) - CC0 1.0 Universal (Public Domain)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Links

- [LZSA Original Project](https://github.com/emmanuel-marty/lzsa)
- [Documentation](https://docs.rs/lzsa-sys)
- [Crates.io](https://crates.io/crates/lzsa-sys)
- [Repository](https://github.com/tommyo123/lzsa-sys)

## See Also

Other Rust compression libraries:
- [lz4](https://crates.io/crates/lz4) - LZ4 compression
- [zstd](https://crates.io/crates/zstd) - Zstandard compression
- [flate2](https://crates.io/crates/flate2) - DEFLATE, gzip, zlib