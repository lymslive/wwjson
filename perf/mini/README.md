# Mini Examples for Assembly Analysis

This directory contains minimal examples for studying compiler optimization
behavior on wwjson's integer serialization and JSON building code.

## Examples

| File | Description | Focus Areas |
|------|-------------|-------------|
| `itoa_u16.cpp` | uint16_t serialization | Recursive template expansion, division/modulo optimization |
| `itoa_u32.cpp` | uint32_t serialization | Same as above, with larger number |
| `builder.cpp` | Minimal JSON building | Function call inlining, buffer direct writing |

## Quick Start

```bash
# Build all examples
make

# Build and generate assembly
make asm

# Build and run
make run

# Clean generated files
make clean
```

## Assembly Analysis

After running `./build.sh asm`, the following files will be generated:

- `itoa_u16_linux.exe` - Compiled executable
- `itoa_u16.s` - Disassembly with source intermix
- `itoa_u32_linux.exe` - Compiled executable
- `itoa_u32.s` - Disassembly with source intermix
- `builder_linux.exe` - Compiled executable
- `builder.s` - Disassembly with source intermix

### Key Analysis Points

1. **Integer Serialization (itoa_u16/itoa_u32)**:
   - Check if `UnsignedWriter` template recursion is fully unrolled
   - Verify integer division/modulo is optimized to multiplication/shift
   - Look for the digit pair lookup table usage

2. **JSON Builder (builder)**:
   - Verify direct buffer writes without intermediate allocations
   - Count actual function calls after inlining
   - Check escape logic is optimized away for simple strings

## Compiler Version

```bash
g++ --version
```

For consistent results, the same compiler version should be used in CI environment.

## CI Integration

See `.github/workflows/ci-mini.yml` for automated assembly analysis in CI.

## Notes

- Executable suffix `*_linux.exe` is used to distinguish from native `.exe` on Windows
- Both `*.exe` and `*.s` files are gitignored
