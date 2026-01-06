# GitHub Actions Workflows

This directory contains CI/CD workflows for the min-dx8-sdk project.

## Workflows

### mingw-64bit-ci.yml
**Comprehensive CI for mingw-64bit-support branch**

**Triggers:**
- Push to `mingw-64bit-support` branch
- Pull requests to `mingw-w64-compatibility` or `main`

**Jobs:**

1. **cross-compile-linux** (Ubuntu, matrix: i686 + x86_64)
   - Cross-compiles Windows executables from Linux
   - Tests basic D3D8 linking
   - Builds full d3d8thk test suite (64-bit)
   - Verifies symbols and dependencies
   - Uploads artifacts for inspection

2. **windows-test** (Windows)
   - Native Windows compilation with MinGW-w64
   - Runs d3d8thk tests on real Windows
   - Captures test output
   - Uploads results as artifacts
   - **Note:** May fail on GitHub runners (no D3D8 GPU support)

3. **code-quality** (Ubuntu)
   - Checks for emoji violations (STYLE.md)
   - Validates GPL3 license structure
   - Verifies Microsoft copyright preservation
   - Checks documentation completeness
   - Validates C++17 standard declaration

4. **report**
   - Generates CI summary
   - Provides next steps based on results

**Duration:** ~10-15 minutes

**When to use:** Full validation before merging

### pr-checks.yml
**Fast PR validation**

**Triggers:**
- Pull requests to any main branch

**Jobs:**

1. **quick-checks** (Ubuntu, ~2 min)
   - Validates commit authorship (per GIT_COMMIT_AUTHORSHIP_INSTRUCTIONS.md)
   - Style guide compliance (no emojis, auto restrictions)
   - License file existence
   - Documentation completeness

2. **compile-check** (Ubuntu, ~3 min)
   - Quick compilation test
   - Symbol verification
   - Ensures no obvious breakage

3. **pr-summary**
   - Simple pass/fail report

**Duration:** ~5 minutes

**When to use:** Quick validation on every PR

## Expected Results

### Cross-compilation (Linux)
✓ **Should always pass**
- Tests that code compiles for Windows
- Verifies linking succeeds
- Checks symbol presence

### Windows Native Tests
⚠️ **May fail on GitHub runners**
- GitHub-hosted Windows VMs lack D3D8 GPU support
- Tests may fail with "device not found" or similar
- This is EXPECTED - tests need real hardware

**How to interpret:**
- Compilation succeeds → Code is valid
- Tests fail → Expected without GPU driver
- Review artifacts for detailed output

### Code Quality
✓ **Must pass**
- No emojis in code/docs
- Correct licensing structure
- Proper documentation

## Artifacts

### Test Executables
**Location:** `test-executables-{arch}`
**Contents:** 
- `main.exe` - Basic D3D8 link test
- `test_*.exe` - d3d8thk validation tests

**Use:**
- Download and run on real Windows hardware
- Validate d3d8thk functionality
- Share test results

### Windows Test Results
**Location:** `windows-test-results`
**Contents:**
- `test_basic_output.txt`
- `test_surface_output.txt`
- `test_context_output.txt`
- `test_draw_output.txt`

**Use:**
- Review what worked/failed
- Diagnose d3d8thk issues
- Document GPU driver compatibility

## Running Locally

### Reproduce Linux cross-compile
```bash
# Install MinGW-w64
sudo apt-get install mingw-w64 cmake

# Build tests
mkdir build-test-d3d8thk
cd build-test-d3d8thk
cmake ../test_d3d8thk -DCMAKE_TOOLCHAIN_FILE=../test_project/toolchain-mingw-w64-x86_64.cmake
make
```

### Reproduce Windows build
```bash
# On Windows with MinGW-w64
mkdir build-test-windows
cd build-test-windows
cmake ../test_d3d8thk -G "MinGW Makefiles"
mingw32-make

# Run tests
./test_basic.exe
./test_surface.exe
./test_context.exe
./test_draw.exe
```

## Troubleshooting

### "d3d8thk.dll not found"
- MinGW-w64 library mismatch
- Update MinGW-w64 installation
- Check toolchain configuration

### "Context creation failed"
- Normal on VMs and GitHub runners
- GPU doesn't have D3D8 driver
- Test on bare metal Windows

### "Authorship check failed"
- Commits have wrong author/committer
- Follow GIT_COMMIT_AUTHORSHIP_INSTRUCTIONS.md
- Amend commits with correct authorship

### "Emoji found"
- Remove all emojis from code/docs
- Use text markers: [OK], [NO], [WARN]
- Check STYLE.md for rules

## Adding New Tests

1. Create test file in `test_d3d8thk/`
2. Add to `test_d3d8thk/CMakeLists.txt`
3. Update workflow to build new test
4. Update this README

## Continuous Deployment

Currently no CD pipeline (no artifacts to deploy).

Future CD could:
- Build release packages
- Generate documentation site
- Publish test results dashboard

## Maintenance

### Workflow Updates
- Review when GitHub Actions updates runners
- Update MinGW-w64 versions as needed
- Adjust timeouts if builds grow

### Monitoring
- Check artifact retention (7 days default)
- Monitor Windows test success rate
- Track build times

## References

- [GitHub Actions docs](https://docs.github.com/en/actions)
- [MinGW-w64 project](https://www.mingw-w64.org/)
- [test_d3d8thk/README.md](../test_d3d8thk/README.md)
