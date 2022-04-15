// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#include "ram.h"
#include <gtest/gtest.h>

using namespace std;
using namespace RAM;

TEST(RAM, task_pow3n_plus_pow2n) {
  Machine m;
  ofstream os;
  ifstream ss("../tasks/3**n+2**n/source.txt");

  m.set_log_stream(os);
  m.set_code(ss);
  m.set_input({15});
  m.run();
  ASSERT_EQ(14381675, m.output().front());
}