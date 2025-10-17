// build.rs - Builds Marty's LZSA C code with optimizations
use std::env;

fn main() {
    println!("cargo:rerun-if-changed=lzsa-upstream/src/");
    println!("cargo:rerun-if-changed=csrc/");

    // List of all C files we need from Marty's project
    let sources = vec![
        // Our wrapper
        "csrc/lzsa_wrapper.c",

        // Compression
        "lzsa-upstream/src/shrink_inmem.c",
        "lzsa-upstream/src/shrink_context.c",
        "lzsa-upstream/src/shrink_block_v1.c",
        "lzsa-upstream/src/shrink_block_v2.c",
        "lzsa-upstream/src/matchfinder.c",

        // Decompression
        "lzsa-upstream/src/expand_inmem.c",
        "lzsa-upstream/src/expand_context.c",
        "lzsa-upstream/src/expand_block_v1.c",
        "lzsa-upstream/src/expand_block_v2.c",

        // Support modules
        "lzsa-upstream/src/frame.c",
        // NOTE: format.c does not exist - it's just format.h with constants

        // libdivsufsort (suffix array library)
        "lzsa-upstream/src/libdivsufsort/lib/divsufsort.c",
        "lzsa-upstream/src/libdivsufsort/lib/divsufsort_utils.c",
        "lzsa-upstream/src/libdivsufsort/lib/sssort.c",
        "lzsa-upstream/src/libdivsufsort/lib/trsort.c",
    ];

    let mut build = cc::Build::new();

    // Add all source files
    build.files(&sources);

    // Include paths
    build
        .include("csrc")
        .include("lzsa-upstream/src")
        .include("lzsa-upstream/src/libdivsufsort/include");

    // Check if we are building for release or debug
    let profile = env::var("PROFILE").unwrap_or_else(|_| "debug".to_string());
    let is_release = profile == "release";

    // Platform-specific optimizations
    let target = env::var("TARGET").unwrap();

    if target.contains("windows") && target.contains("msvc") {
        // Windows MSVC
        build.define("_CRT_SECURE_NO_WARNINGS", None);

        if is_release {
            build.flag("/O2");      // Maximum speed optimization
            build.flag("/GL");      // Whole program optimization
            build.flag("/Gy");      // Function-level linking
        } else {
            build.flag("/Od");      // No optimization for debug
        }
    } else {
        // GCC/Clang (Linux, macOS, MinGW)
        if is_release {
            build.flag("-O3");                  // Maximum optimization
            build.flag("-fomit-frame-pointer"); // Extra performance
            build.flag("-march=native");        // Optimize for current CPU
            build.flag("-flto");                // Link-time optimization
        } else {
            build.flag("-O0");      // No optimization for debug
            build.flag("-g");       // Debug symbols
        }
    }

    // Compile to a static library
    build.compile("lzsa");

    println!("cargo:rustc-link-lib=static=lzsa");
}
