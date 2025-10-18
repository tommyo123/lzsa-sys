use lzsa_sys::{compress_v1, compress_v2, decompress};

fn main() {
    let test_data = b"The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";

    println!("Original size: {} bytes\n", test_data.len());

    // Test LZSA1 forward
    println!("=== LZSA1 Forward ===");
    let compressed_v1 = compress_v1(test_data).expect("LZSA1 compression failed");
    println!(
        "Compressed: {} bytes ({:.1}%)",
        compressed_v1.len(),
        (compressed_v1.len() as f64 / test_data.len() as f64) * 100.0
    );
    let decompressed = decompress(&compressed_v1).expect("Decompression failed");
    assert_eq!(test_data, decompressed.as_slice());
    println!("✓ Verified\n");

    // Test LZSA2 forward
    println!("=== LZSA2 Forward ===");
    let compressed_v2 = compress_v2(test_data).expect("LZSA2 compression failed");
    println!(
        "Compressed: {} bytes ({:.1}%)",
        compressed_v2.len(),
        (compressed_v2.len() as f64 / test_data.len() as f64) * 100.0
    );
    let decompressed = decompress(&compressed_v2).expect("Decompression failed");
    assert_eq!(test_data, decompressed.as_slice());
    println!("✓ Verified\n");

    println!("All modes tested successfully! ✓");
}
