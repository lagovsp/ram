// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#ifndef INCLUDE_RAM_H_
#define INCLUDE_RAM_H_

#include <list>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <variant>
#include <unordered_map>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <functional>

#define MACHINE_STATIC_FUN_DECL(name) static ComIt name(Machine &, ComIt &)

#define ALL_MACHINE_STATIC_FUN_DECL \
    MACHINE_STATIC_FUN_DECL(load); \
    MACHINE_STATIC_FUN_DECL(store); \
    MACHINE_STATIC_FUN_DECL(add); \
    MACHINE_STATIC_FUN_DECL(sub); \
    MACHINE_STATIC_FUN_DECL(mult); \
    MACHINE_STATIC_FUN_DECL(div); \
    MACHINE_STATIC_FUN_DECL(read); \
    MACHINE_STATIC_FUN_DECL(write); \
    MACHINE_STATIC_FUN_DECL(jump); \
    MACHINE_STATIC_FUN_DECL(jgtz); \
    MACHINE_STATIC_FUN_DECL(jzero); \
    MACHINE_STATIC_FUN_DECL(halt)

namespace RAM {

using Add = int;
using Val = int;

using Arg = std::variant<int, std::string>;
using Lab = std::optional<std::string>;

using Name = std::string;
using ComT = std::string;
using ArgT = std::string;

struct Com {
  ComT ctype_;
  ArgT atype_;
  Arg arg_;
  Lab label_;
};

using Program = std::list<Com>;
using ComIt = Program::iterator;

using Tape = std::list<Val>;
using Labels = std::unordered_map<Lab, ComIt>;
using Memory = std::map<Add, Val>;

std::ostream &operator<<(std::ostream &, const Tape &);
std::ostream &operator<<(std::ostream &, const Memory &);

class Machine {
 public:
  Machine() = default;
  Tape run();

  void set_name(const Name &);
  void set_code(std::istream &);
  void set_input(std::istream &);
  void set_input(std::initializer_list<Val>);
  void reset();

  void set_log_stream(std::ostream &os);
  void be_verbose(bool);

  static inline const ComT LOAD = "LOAD";
  static inline const ComT STORE = "STORE";
  static inline const ComT ADD = "ADD";
  static inline const ComT SUB = "SUB";
  static inline const ComT MULT = "MULT";
  static inline const ComT DIV = "DIV";
  static inline const ComT READ = "READ";
  static inline const ComT WRITE = "WRITE";
  static inline const ComT JUMP = "JUMP";
  static inline const ComT JGTZ = "JGTZ";
  static inline const ComT JZERO = "JZERO";
  static inline const ComT HALT = "HALT";

  static inline const ArgT ADDRESS = "()";
  static inline const ArgT VALUE = "(=)";
  static inline const ArgT ADDRESS_AT_ADDRESS = "(*)";
  static inline const ArgT LABEL = "(<>)";

 private:
  Name name_;
  Tape input_;
  Tape output_;
  Memory memory_;
  Labels labels_;
  Program commands_;

  bool stop_ = false;
  bool verbose_ = false;
  std::ostream *out_ = &std::cout;

  void add_label(ComIt);
  ComIt process_command(const std::string &);

  static Com parse_command(const std::string &);
  static ComT get_com_type(const std::string &);
  static ArgT get_arg_type(const std::string &);
  static Arg get_arg_number(const std::string &);
  static Arg get_arg_label(const std::string &);
  static Lab get_label(const std::string &);

  Arg get_arg(ComIt);

  static Arg value(Machine &, const Arg &);
  static Arg direct_value(Machine &, const Arg &);
  static Arg indirect_value(Machine &, const Arg &);
  static Arg label(Machine &, const Arg &);

  using ArgHand = std::function<Arg(Machine &, const Arg &)>;

  static inline const std::map<ArgT, ArgHand> FROM_T = {
	  {VALUE, value},
	  {ADDRESS, direct_value},
	  {ADDRESS_AT_ADDRESS, indirect_value},
  };

  static inline const std::map<ArgT, ArgHand> TO_T = {
	  {ADDRESS, value},
	  {ADDRESS_AT_ADDRESS, direct_value},
  };

  static inline const std::map<ArgT, ArgHand> MOVE_T = {
	  {LABEL, label},
  };

  ALL_MACHINE_STATIC_FUN_DECL;
  using Fun = std::function<ComIt(Machine &, ComIt &)>;

  static inline const std::map<ComT, std::pair<std::map<ArgT, ArgHand>, Fun>>
	  COM_HAND = {
	  {LOAD, {FROM_T, load}},
	  {STORE, {TO_T, store}},
	  {ADD, {FROM_T, add}},
	  {SUB, {FROM_T, sub}},
	  {MULT, {FROM_T, mult}},
	  {DIV, {FROM_T, div}},
	  {READ, {TO_T, read}},
	  {WRITE, {FROM_T, write}},
	  {JUMP, {MOVE_T, jump}},
	  {JGTZ, {MOVE_T, jgtz}},
	  {JZERO, {MOVE_T, jzero}},
	  {HALT, {MOVE_T, halt}},
  };
};

}

#endif // INCLUDE_RAM_H_
