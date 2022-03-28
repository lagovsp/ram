#include <iostream>

#include "RAM.h"

using namespace std;
using namespace RAM;

int main() {
  Machine m;
  m.set_path("../ram-count-0-1.txt");
  m.set_input({4, 0, 1, 1, 0,});
  m.be_verbose(true);

  ofstream os("../output.txt");
  m.set_ostream(os);

  auto output = m.run();
  os << "Output: " << output;

  return 0;
}