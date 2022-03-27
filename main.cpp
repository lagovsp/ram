#include <iostream>

#include "RAM.h"

using namespace std;

int main() {
  RAM::Machine machine;
  machine.set_path("../ram-code.txt");
  machine.set_input({2, 1, 0}); // make as a cells with values
  auto answer = machine.run();

  cout << "Output: {";
  bool first1 = true;
  for (const auto &pos: answer) {
	if (!first1) { cout << ", "; }
	first1 = false;
	cout << pos;
  }
  cout << "}";

  return 0;
}
