# Cool language compiler

### Hot to build:
```bash
mkdir build
cd build
cmake .. [-DCOOLC_SANITIZER=UBSAN/ASAN/TSAN/MEMSAN]
make -j coolc
```

### TODO:
- [ ] Add concurrency
- [ ] Add abseil result instead of optional
- [ ] Code refactoring: GetStringLiteral
- [ ] Unit tests for lexer
- [ ] Fix CI
