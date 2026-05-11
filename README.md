# printf Performance Benchmark

A performance benchmark comparing different approaches to converting a 16-byte identifier (similar to a GUID/UUID) into its canonical hex string representation (`XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX`).

## Inspiration

This project was inspired by [Dave Plummer](https://www.youtube.com/watch?v=VYTF4KIF2z0), who posted a YouTube video about how he optimized string formatting in the COM Windows code. He took the original `sprintf`-based code and created a faster version. This benchmark explores how far you can push that optimization.

## What It Does

The program generates 40 million random 16-byte identifiers and benchmarks ten different implementations of the binary-to-hex-string conversion:

| Method | Description |
|---|---|
| `IIDtoStringOriginal` | The baseline: uses `sprintf()` with `%08X-%04X-%04X-%02X%02X-...` format specifiers |
| `IIDtoStringFast` | Lookup table (`hex[]`) with a predefined byte order array to avoid branch-heavy index calculation |
| `IIDtoStringHex2` | 512-entry lookup table (`hex2[]`) that returns two hex characters at once per byte |
| `IIDtoStringHex16` | 256-entry lookup table (`hex16[]`) returning two chars as a `uint16_t`, written two bytes at a time |
| `IIDtoStringHex16Prefill` | Same as Hex16, but pre-fills hyphen separators and null terminator before the conversion loop |
| `IIDtoStringHex16Aligned` | Writes 32-bit aligned stores using `hex16[]`, avoiding the `memcpy` template entirely |
| `IIDtoStringAlignedStores` | Uses 64-bit and 32-bit aligned stores with `hex16[]` for wider memory writes |
| `IIDtoStringHex32Prefill` | 65536-entry lookup table (`hex32[]`) returning four hex characters as a `uint32_t` per 16-bit half |
| `IIDtoStringHex16MemCpy` | Uses `memcpy` to pre-fill the UUID template, then writes `hex16[]` results two bytes at a time |
| `IIDtoStringArith` | Pure arithmetic approach: computes hex characters using bit manipulation — no lookup tables |

## Requirements

- A C compiler (GCC or Clang)
- `make` build tool

## Build & Run

```bash
make
./stringperformance
```

To clean up built artifacts:

```bash
make clean
```

## Example Output

```
Generation Time: 0.850 seconds
Slow Runtime: 12.340 seconds
Fast Runtime: 1.200 seconds
Hex2 Runtime: 0.950 seconds
Hex16 Runtime: 0.720 seconds
Hex16 Prefill Runtime: 0.680 seconds
Hex16 Aligned Runtime: 0.650 seconds
Hex16 Aligned Stored Runtime: 0.610 seconds
Hex32 Prefill Runtime: 0.580 seconds
Hex16 MemCpy Runtime: 0.700 seconds
Arith Runtime: 0.640 seconds
Mismatches: 0 / 40000000
```

> **Note:** Actual numbers will vary depending on your CPU and compiler optimizations. The benchmark is compiled with `-O2`.

## Correctness Verification

After running all benchmarks, the program verifies every result against the `sprintf` baseline. A `Mismatches: 0 / 40000000` line confirms all implementations produce identical output to the original `sprintf` approach.

## Architecture

The core data structure being converted is a 16-byte identifier:

```c
typedef struct {
    unsigned int   Data1;   // 4 bytes
    unsigned short Data2;   // 2 bytes
    unsigned short Data3;   // 2 bytes
    unsigned char  Data4[8]; // 8 bytes
} IID;
```

This matches the layout of a Windows `IID`/`GUID` and a standard UUID (128 bits = 16 bytes), formatted as `8-4-4-4-12` hexadecimal characters separated by hyphens.

## Performance Results

Mean runtimes (in seconds) over 10 runs, compiled with `-O2`. Speedup is relative to the `Slow` (sprintf) baseline. The fastest algorithm per device is highlighted in **bold** with ⚡.

| Device | Slow | Fast | Hex2 | Hex16 | Hex16 Prefill | Hex16 Aligned | Hex16 Aligned Stored | Hex32 Prefill | Hex16 MemCpy | Arith |
|---|---|---|---|---|---|---|---|---|---|---|
| MacBook Pro 2019, Intel i9, 64GB, Apple clang 17.0.0 | 8.151 s (1.0x) | 0.671 s (12.1x) | 0.416 s (19.6x) | 0.413 s (19.7x) | 0.409 s (19.9x) | **0.384 s (21.2x)** ⚡ | 0.392 s (20.8x) | 0.459 s (17.8x) | 0.425 s (19.2x) | 0.684 s (11.9x) | 
> **Contributing results:** Run `./benchmark.sh` on your machine and add a new row to the table above. Open a pull request with your device specs and results.

## References

- [Dave Plummer's YouTube Video](https://www.youtube.com/watch?v=VYTF4KIF2z0) — the original inspiration for this benchmark
- [Windows GUID Documentation](https://learn.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid)

---

*Built out of curiosity. If you have a faster approach, open a pull request!*