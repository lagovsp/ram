// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#ifndef INCLUDE_RAM_H_
#define INCLUDE_RAM_H_

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <variant>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <functional>

namespace RAM {

using Addr = int;
using Val = int;

using ComType = std::string;
using ArgType = std::string;

using Arg = std::variant<int, std::string>;
using Lab = std::optional<std::string>;

static const ComType LOAD = "LOAD";
static const ComType STORE = "STORE";
static const ComType ADD = "ADD";
static const ComType SUB = "SUB";
static const ComType MULT = "MULT";
static const ComType DIV = "DIV";
static const ComType READ = "READ";
static const ComType WRITE = "WRITE";
static const ComType JUMP = "JUMP";
static const ComType JGTZ = "JGTZ";
static const ComType JZERO = "JZERO";
static const ComType HALT = "HALT";

static const ArgType ADDRESS = "ADDRESS";
static const ArgType VALUE = "VALUE";
static const ArgType ADDRESS_AT_ADDRESS = "ADDRESS_AT_ADDRESS";

struct Com {
  ComType ctype_; // LOAD, STORE, ADD, etc.
  ArgType atype_; // ADDRESS, VALUE, ADDRESS_AT_ADDRESS
  Arg arg_; // 2, FU, etc.
  Lab label_; // no_value, FU, etc.

  friend std::ostream &operator<<(std::ostream &os, const Com &c);
};

using ComIt = std::list<Com>::iterator;

class Machine {
 public:
  Machine() = default;
  explicit Machine(std::string);
  std::list<int> run();
  void setPath(const std::string &);
  void setInput(const std::string &);
  const std::list<Com> &commands() const;

 private:
  void processFile(const std::string &);
  void processCom(std::string line);
  void addLabel(Lab);

  static Com parseCom(std::string line);
  static ComType parseComType(std::string line);
  static ArgType parseArgType(std::string line);
  static Arg parseArgNum(std::string line);
  static Arg parseArgLab(std::string line);
  static Lab parseLab(std::string line);

  static int fromValToArg(const Machine &m, const Arg &a);
  static int fromAddToArg(const Machine &m, const Arg &a);
  static int fromAddAtAddToArg(const Machine &m, const Arg &a);

  using ArgHandler = std::function<int(const Machine &, const Arg &)>;

  inline static const std::map<ArgType, ArgHandler> ARG_HANDLERS = {
	  {VALUE, Machine::fromValToArg},
	  {ADDRESS, Machine::fromAddToArg},
	  {ADDRESS_AT_ADDRESS, Machine::fromAddAtAddToArg},
  };

  static ComIt load(Machine &, ComIt &);
  static ComIt store(Machine &, ComIt &);
  static ComIt add(Machine &, ComIt &);
  static ComIt sub(Machine &, ComIt &);
  static ComIt mult(Machine &, ComIt &);
  static ComIt div(Machine &, ComIt &);
  static ComIt read(Machine &, ComIt &);
  static ComIt write(Machine &, ComIt &);
  static ComIt jump(Machine &, ComIt &);
  static ComIt jgtz(Machine &, ComIt &);
  static ComIt jzero(Machine &, ComIt &);
  static ComIt halt(Machine &, ComIt &);

  using ComHandler = std::function<ComIt(Machine &, ComIt &)>;

  inline static const std::map<ComType, ComHandler> COM_HANDLERS = {
	  {LOAD, Machine::load},
	  {STORE, Machine::store},
	  {ADD, Machine::add},
	  {SUB, Machine::sub},
	  {MULT, Machine::mult},
	  {DIV, Machine::div},
	  {READ, Machine::read},
	  {WRITE, Machine::write},
	  {JUMP, Machine::jump},
	  {JGTZ, Machine::jgtz},
	  {JZERO, Machine::jzero},
	  {HALT, Machine::halt},
  };

  std::unordered_map<Addr, Val> memory_;

  std::string file_;
  std::list<int> input_;
  std::list<int> output_;

  std::list<Com> commands_;
  std::unordered_map<Lab, ComIt> labels_;
};

}

#endif // INCLUDE_RAM_H_
