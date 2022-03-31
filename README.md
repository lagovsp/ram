# Random Access Machine

## Manual

1. Set `CMakeLists.txt` properly

```cmake
add_executable(exec *.cpp)

add_subdirectory(ram)
target_link_libraries(exec PUBLIC ram)
```

2. Setting up

```cpp
#include "RAM.h"
using namespace RAM;
```

3. Create an instance of RAM

```cpp
Machine m;
```

4. Adjust it

```cpp
m.set_code("ram-count-0-1.txt"); // set the source RAM-code path
m.set_input({6, 1, 0, 1, 0, 0, 1}); // enter the input data
m.be_verbose(false); // to be verbose while executing (false by default)
m.set_ostream(cout); // customize the out stream (cout by default)
```

5. Run the code

```cpp
m.run();
```

6. Check the log file or print the out tape

```cpp
cout << m.run();
```

## Example

***Task:***
The program receives `n + 1` values. The first value is `n`. Among other values, every single one is either `0` or `1`.
Tell the value that prevails among the given `n` values. In case, there are more `1`s than `0`s, print `1`. If `0`s -
print `0`. If the values occur the same number of times, the output is to be two-character `1`,`0`.

***Note:***
*It is forbidden to use negative values everywhere*

### RAM-code sample

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
HALT()
```

### Outputs

```cpp
m.set_input({6, 1, 0, 1, 0, 0, 1});
Output: { 1, 0 }
```

```cpp
m.set_input({6, 1, 0, 1, 0, 0, 0});
Output: { 0 }
```

```cpp
m.set_input({3, 1, 0, 1});
Output: { 1 }
```

```cpp
m.set_input({2, 0, 1});
Output: { 1, 0 }
```

```cpp
m.set_input({0});
Output: { 1, 0 }
```

You can have a glance at the logs in `output.txt`

###### Copyright 2022 Sergey Lagov lagovsp@gmail.com
