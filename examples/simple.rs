use lzsa_sys::{compress_v1, decompress};

fn main() {
    let original = b"Hello, LZSA! This is a test of compression. Let's repeat this to get better compression: Hello, LZSA! This is a test of compression.";

    println!("Original size: {} bytes", original.len());

    // Compress with LZSA1
    let compressed = compress_v1(original).expect("Compression failed");
    println!("Compressed size: {} bytes", compressed.len());
    println!(
        "Compression ratio: {:.1}%",
        (compressed.len() as f64 / original.len() as f64) * 100.0
    );

    // Decompress (auto-detect version)
    let decompressed = decompress(&compressed).expect("Decompression failed");

    // Verify round-trip
    assert_eq!(original, decompressed.as_slice());
    println!("✓ Round-trip successful!");
    println!();
    println!(
        "Compression saved {} bytes",
        original.len() - compressed.len()
    );
}
