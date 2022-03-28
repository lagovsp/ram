#include <iostream>

#include "RAM.h"

using namespace std;
using namespace RAM;

int main() {
  Machine machine;
  machine.set_path("../ram-code.txt");
  machine.set_input({2, 1, 0});
  machine.be_verbose(true);
  machine.set_ostream(cout);

  auto output = machine.run();
  cout << "Output: " << output;

  return 0;
}
