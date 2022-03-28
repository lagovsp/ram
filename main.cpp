#include <iostream>

#include "RAM.h"

using namespace std;
using namespace RAM;

int main() {
  Machine m;
  m.set_path("../ram-count-0-1.txt");
  m.set_input({2, 0, 1});
  m.be_verbose(true);

  ofstream ofs("../output.txt");
  m.set_ostream(ofs);

  auto output = m.run();
  cout << "Output: " << output;

  return 0;
}