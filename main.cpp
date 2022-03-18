#include <iostream>

#include "RAM.h"

using namespace std;

int main() {
  RAM::Machine machine;
  machine.setPath("../ram-code.txt");
  machine.setInput("70010110");
  auto answer = machine.run();
  auto &test = machine.commands();

  cout << "{";
  bool first = true;
  for (const auto &pos: answer) {
	if (!first) {
	  cout << ", ";
	}
	first = false;
	cout << pos;
  }
  cout << "}";

  return 0;
}
