# Random Access Machine

## Usage

1. Setting up

```cpp
#include "RAM.h"
using namespace RAM;
```

2. Create an instance of RAM

```cpp
Machine m;
```

3. Adjust it

```cpp
m.set_path("ram-count-0-1.txt"); // set the source RAM-code path
m.set_input({6, 1, 0, 1, 0, 0, 1}); // enter the input data
m.be_verbose(false); // to be verbose while executing (false by default)
m.set_ostream(cout); // customize the out stream (cout by default)
```

4. Run the code

```cpp
auto output = m.run();
```

5. See the output

```cpp
cout << "Output: " << output;
```

## Example

*Task:*
The program receives an `n + 1` values. The first value is equal to `n`. Every single other value is either `0` or `1`.
Say, which value prevails among the given `n` values. In case, there are more `1` than `0`, print `1`. If `0` -
print `0`. if the values occur the same number of times, print `10`.

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

File processed
Commands 39
Output: { 1, 0 }
```

```cpp
m.set_input({6, 1, 0, 1, 0, 0, 0});

File processed
Commands 39
Output: { 0 }
```

```cpp
m.set_input({3, 1, 0, 1});

File processed
Commands 39
Output: { 1 }
```

```cpp
m.set_input({0});

File processed
Commands 39
Output: { 1, 0 }
```

###### Copyright 2022 Sergey Lagov lagovsp@gmail.com
