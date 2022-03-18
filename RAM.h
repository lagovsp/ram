// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#ifndef INCLUDE_RAM_H_
#define INCLUDE_RAM_H_

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace RAM {

using CellID = int;
using CellVal = int;

using CommandType = std::string;
using ArgType = std::string;

using Arg = std::variant<int, std::string>;
using Label = std::optional<std::string>;

static const CommandType LOAD = "LOAD";
static const CommandType STORE = "STORE";
static const CommandType ADD = "ADD";
static const CommandType SUB = "SUB";
static const CommandType MULT = "MULT";
static const CommandType DIV = "DIV";
static const CommandType READ = "READ";
static const CommandType WRITE = "WRITE";
static const CommandType JUMP = "JUMP";
static const CommandType JGTZ = "JGTZ";
static const CommandType JZERO = "JZERO";
static const CommandType HALT = "HALT";

static const ArgType ADDRESS = "ADDRESS";
static const ArgType VALUE = "VALUE";
static const ArgType ADDRESS_AT_ADDRESS = "ADDRESS_AT_ADDRESS";

struct Command {
  CommandType ctype_;
  ArgType atype_;
  Arg arg_;
  Label label_;

  friend std::ostream &operator<<(std::ostream &os, const Command &c);
};

using CommandIt = std::list<Command>::iterator;

class Machine {
 public:
  Machine() = default;
  explicit Machine(std::string);
  std::list<int> run();
  void setPath(std::string);
  void setInput(std::string);
  const std::list<Command> &commands() const;

 private:
  void processFile(const std::string &);
  void processCommand(std::string);
  void addLabel(Label);

  Command parseCommand(std::string);
  CommandType parseCommandType(std::string);
  ArgType parseArgumentType(std::string);
  Arg parseArgumentNumeric(std::string line);
  Arg parseArgumentLabel(std::string line);
  Label parseLabel(std::string);

  static Arg fromValueToValue(const Machine &, const Arg &);
  static Arg fromAddressToValue(const Machine &, const Arg &);
  static Arg fromAddressAtAddressToValue(const Machine &, const Arg &);

  using ArgumentHandler = std::function<Arg(const Machine &, const Arg &)>;

  const std::unordered_map<ArgType, ArgumentHandler> ARGUMENTS_HANDLERS = {
	  {VALUE, Machine::fromValueToValue},
	  {ADDRESS, Machine::fromAddressToValue},
	  {ADDRESS_AT_ADDRESS, Machine::fromAddressAtAddressToValue},
  };

  static void load(Machine &, const Arg &, CommandIt &);
  static void store(Machine &, const Arg &, CommandIt &);
  static void add(Machine &, const Arg &, CommandIt &);
  static void sub(Machine &, const Arg &, CommandIt &);
  static void mult(Machine &, const Arg &, CommandIt &);
  static void div(Machine &, const Arg &, CommandIt &);
  static void read(Machine &, const Arg &, CommandIt &);
  static void write(Machine &, const Arg &, CommandIt &);
  static void jump(Machine &, const Arg &, CommandIt &);
  static void jgtz(Machine &, const Arg &, CommandIt &);
  static void jzero(Machine &, const Arg &, CommandIt &);
  static void halt(Machine &, const Arg &, CommandIt &);

  using CommandHandler =
  std::function<void(Machine &, const Arg &, CommandIt &)>;

  const std::unordered_map<CommandType, CommandHandler> COMMANDS_HANDLERS = {
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

  std::unordered_map<CellID, CellVal> memory_;

  std::string file_;
  std::string input_;
  std::list<int> output_;

  std::list<Command> commands_;
  std::unordered_map<Label, CommandIt> labels_;
};

}

#endif // INCLUDE_RAM_H_
