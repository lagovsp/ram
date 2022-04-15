# Random Access Machine

[![Build Status](https://app.travis-ci.com/justddreamm/ram.svg?branch=master)](https://app.travis-ci.com/justddreamm/ram)

## Manual

1. Set up `CMakeLists.txt`

```cmake
add_executable(exec *.cpp)

add_subdirectory(ram)
target_link_libraries(exec PUBLIC ram)
```

2. Include the library and create an instance of RAM

```cpp
#include "RAM.h"
using namespace RAM;
...
Machine m;
```

3. For further identification, give it a name

```cpp
m.set_name("super-machine"); // optional
```

4. Give the machine a stream to write to. Specify the logging mode

```cpp
ofstream ls("log.txt");
m.set_log_stream(ls); // default -> cout
m.be_verbose(true); // default -> false
```

5. Provide it with the input stream to take the RAM source code from

```cpp
ifstream ss("source.txt");
m.set_code(ss);
```

6. In order to set the input pick one of the following options

```cpp
ifstream is("input.txt");
m.set_input(is);
```

```cpp
m.set_input({ 6, 1, 0, 1, 0, 0, 1 });
```

7. Run the code

```cpp
auto output = m.run();
```

8. Check the logs or print the out tape

```cpp
cout << output;
```

## Notes

- The machine does not control any uninitialized cells access. In case of reaching such, random value is generated
  as its content. Logs disclose these occurrences with the corresponding messages
- There is no any prompts or tips on the emerged issue with the source code correctness. Error message is the only
  sign of the ill-formed source code
- All the functions are to be in capitals with a mandatory parentheses following (even for `HALT()`)

## Example

***Task:***
The program receives `n + 1` values. The first value is `n`. Among other values, every single one is either `0` or `1`.
Tell the value that prevails among the given `n` values. In case, there are more `1`s than `0`s, print `1`. If `0`s -
print `0`. If the values occur the same number of times, the output is to be two-character `1`,`0`.

***Note:***
*It is forbidden to use negative values everywhere*

### Solution sample

```
LOAD(=0)
STORE(2)
STORE(3)
READ(1)
LOAD(1)
JZERO(EQ)

JZERO(A) R
READ(0)
JGTZ(U)
JUMP(Z)

LOAD(3) U
ADD(=1)
STORE(3)
JUMP(N)

LOAD(2) Z
ADD(=1)
STORE(2)
JUMP(N)

LOAD(1) N
SUB(=1)
STORE(1)
JUMP(R)

WRITE(=1) EQ
WRITE(=0)
HALT()

LOAD(2) A
JZERO(FU)
LOAD(3)
JZERO(FZ)
DIV(2)
JZERO(FZ)
LOAD(2)
DIV(3)
JZERO(FU)
JUMP(EQ)

WRITE(=1) FU
HALT()

WRITE(=0) FZ
```

### Outputs

```cpp
m.set_input({3, 1, 0, 1});
Output: { 1 }
```

```cpp
m.set_input({ 6, 1, 0, 1, 0, 0, 0 });
Output: { 0 }
```

```cpp
m.set_input({6, 1, 0, 1, 0, 0, 1});
Output: { 1, 0 }
```

```cpp
m.set_input({0});
Output: { 1, 0 }
```

Detailed logs are in `demo_tasks\<task-name>\log.txt`

###### Copyright 2022, Sergey Lagov
