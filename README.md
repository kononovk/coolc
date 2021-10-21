# Coolc

_Compiler for [Cool](https://en.wikipedia.org/wiki/Cool_(programming_language)) (Classroom Object-Oriented Language)._

### Simple Cool program:
```text
class Main inherits IO {
    pal(s : String) : Bool {
	if s.length() = 0
	then true
	else if s.length() = 1
	then true
	else if s.substr(0, 1) = s.substr(s.length() - 1, 1)
	then pal(s.substr(1, s.length() -2))
	else false
	fi fi fi
    };

    i : Int;

    main() : SELF_TYPE {
	{
            i <- ~1;
	    out_string("enter a string\n");
	    if pal(in_string())
	    then out_string("that was a palindrome\n")
	    else out_string("that was not a palindrome\n")
	    fi;
	}
    };
};
```
More examples you can check in [examples](/examples) directory.

### Hot to build:
```bash
mkdir build
cd build
cmake .. [-DCOOLC_SANITIZER=UBSAN/ASAN/TSAN/MEMSAN] [-DBUILD_TESTING=ON]
cmake --build . [--target lexer/parser/semant/coolc]
```

### How to run end-to-end tests:
```bash
test/e2e/test_runner -t path/to/test_dir -e path/to/executable
```
* Example for lexer:
```bash
test/e2e/test_runner  -t test/e2e/lexer -e build/main/parser
```
