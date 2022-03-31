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
//using Path = std::optional<std::string>;

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

  void set_code(std::istream &);
  void set_input(std::istream &);
  void set_input(std::initializer_list<Val>);

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
  mutable bool stop_ = false;

  Tape input_;
  Tape output_;
  Memory memory_;
  Labels labels_;
  Program commands_;

  bool verbose_ = false;
  std::ostream *out_ = nullptr;

  void add_label(ComIt);
  ComIt process_command(const std::string &);

  static Com parse_command(const std::string &);
  static ComT parse_command_t(const std::string &);
  static ArgT parse_argument_t(const std::string &);
  static Arg parse_argument_numeric(const std::string &);
  static Arg parse_argument_label(const std::string &);
  static Lab parse_label(const std::string &);

  static Arg value_to_argument(const Machine &, const Arg &);
  static Arg address_to_argument(const Machine &, const Arg &);
  static Arg address_to_address_to_argument(const Machine &, const Arg &);
  static Arg label_to_argument(const Machine &, const Arg &);

  using ArgHand = std::function<Arg(const Machine &, const Arg &)>;

  static inline const std::map<ArgT, ArgHand> FROM_HAND = {
	  {ADDRESS, address_to_argument},
	  {VALUE, value_to_argument},
	  {ADDRESS_AT_ADDRESS, address_to_address_to_argument},
  };

  static inline const std::map<ArgT, ArgHand> TO_HAND = {
	  {ADDRESS, value_to_argument},
	  {VALUE, value_to_argument},
	  {ADDRESS_AT_ADDRESS, address_to_address_to_argument},
  };

  static inline const std::map<ArgT, ArgHand> MOVE_HAND = {
	  {LABEL, label_to_argument},
  };

  static inline const std::map<ComT, std::map<ArgT, ArgHand>> GROUPS = {
	  {LOAD, FROM_HAND},
	  {STORE, TO_HAND},
	  {ADD, FROM_HAND},
	  {SUB, FROM_HAND},
	  {MULT, FROM_HAND},
	  {DIV, FROM_HAND},
	  {READ, TO_HAND},
	  {WRITE, FROM_HAND},
	  {JUMP, MOVE_HAND},
	  {JGTZ, MOVE_HAND},
	  {JZERO, MOVE_HAND},
	  {HALT, MOVE_HAND},
  };

  ALL_MACHINE_STATIC_FUN_DECL;
  using ComHand = std::function<ComIt(Machine &, ComIt &)>;

  static inline const std::map<ComT, ComHand> COM_HAND = {
	  {LOAD, load},
	  {STORE, store},
	  {ADD, add},
	  {SUB, sub},
	  {MULT, mult},
	  {DIV, div},
	  {READ, read},
	  {WRITE, write},
	  {JUMP, jump},
	  {JGTZ, jgtz},
	  {JZERO, jzero},
	  {HALT, halt},
  };
};

}

#endif // INCLUDE_RAM_H_
