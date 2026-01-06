# Code Style Guide for min-dx8-sdk

## General Rules
- Never use emojis in any files
- All text files must end with a newline
- Trim trailing whitespace from all lines
- Use present tense in documentation ("Fixes" not "Fixed")

## Code Changes Scope
- Code edits only touch lines that serve the intended goal
- Big refactors should not be combined with logical changes
- If a change requires refactor, create separate commits
- Keep changes minimal and focused

## C++ Standard
- Use C++17 standard for new implementation code
- The min-dx8-sdk library itself (headers and .lib files) remains unchanged
- Future wrapper implementations should use C++17 features that improve clarity:
  - nullptr instead of NULL
  - override keyword for virtual functions
  - Range-based for loops
  - std::optional for optional return values
  - if constexpr for compile-time decisions
  - Scoped enums (enum class)
- Keep code simple - don't use features just because they exist

## Code Style
- Fit nearby code style for consistency
- Write clear, explicit code that's easy to understand
- Maintain consistent indentation within files

## Indentation
- C/C++ files (*.h, *.cpp, *.c): 4 spaces or match existing style
- CMake files (CMakeLists.txt, *.cmake): 4 spaces
- Python files (*.py): 4 spaces
- Markdown files (*.md): No specific requirement

## Language Features - Auto Keyword Usage
- Use explicit types by default for maximum readability
- auto is ONLY allowed in range-based for loops:
  ```cpp
  // ALLOWED: Range-based for loop (idiomatic and clear)
  for (const auto& item : collection) {
      // ...
  }
  
  // NOT ALLOWED: Regular variable declarations
  DisplayMode mode = GetDisplayMode();  // Good - explicit type
  auto mode = GetDisplayMode();         // Bad - unclear what type is returned
  
  // NOT ALLOWED: Iterators (use explicit type)
  std::map<std::string, Device*>::iterator it = map.find(key);  // Good
  auto it = map.find(key);                                       // Bad
  
  // NOT ALLOWED: Casts (keep cast type visible)
  IDirect3D8* pD3D = static_cast<IDirect3D8*>(ptr);  // Good
  auto pD3D = static_cast<IDirect3D8*>(ptr);         // Bad
  ```

## Other Language Features
- Prefer simple, readable code over clever optimizations
- Use const where appropriate
- Avoid unnecessary template complexity

## Comments
- Comment user-facing changes where they occur
- Maintenance changes (build fixes) typically don't need comments
- Keep comments concise and meaningful
- Avoid redundant comments that repeat code

## Documentation
- Write in present tense
- No abbreviations in user-facing text
- Be specific about what changed and why
- Include rationale for controversial changes

## File Organization
- Place all imports at the top unless there's a specific reason not to
- Group related functionality together
- Keep header files minimal and focused
- Separate interface from implementation

## Commit Messages
- Use conventional commit format: type: Description
- Start with action verb: fix, add, change, refactor, etc.
- Be concise but descriptive
- Begin with lowercase after the colon
- No period at the end

## Allowed Commit Types
- bugfix: - Fixes a user-facing bug
- fix: - Fixes something (not user-facing bug)
- feat: - Adds new feature
- build: - Build system or compilation fixes
- docs: - Documentation changes
- refactor: - Code restructuring without behavior change
- perf: - Performance improvements
- test: - Test additions or fixes
- chore: - Maintenance tasks

## Pull Requests
- Title format: "type: Description starting with verb"
- Link related issues in PR body
- Explain what changed and why
- Include test results when applicable
- Keep PR scope focused

## Testing
- Test changes before committing
- Document test procedures
- Include both positive and negative test cases
- Test on target architectures (32-bit and 64-bit)

## Platform Specific
- Be mindful of 32-bit vs 64-bit differences
- Test MinGW builds when changing build system
- Windows-specific code should be clearly marked
- Use WINAPI calling convention where appropriate