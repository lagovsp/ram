// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#include "ram.h"

using namespace std;

namespace RAM {

static const char SP = ' ';
static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';

static const string LABEL_USED_ERROR = "label already used";
static const string LABEL_NOT_FOUND_ERROR = "label not found";
static const string BAD_COMMAND_ERROR = "bad command";
static const string PATH_NOT_SET_ERROR = "file path not set";
static const string FILE_NOT_FOUND_ERROR = "file not found";
static const string BAD_ISTREAM_ERROR = "bad istream";
static const string BAD_OSTREAM_ERROR = "bad ostream";

static const string FILE_PROCESSED_MSG = "File processed ";

static const string INPUT_RECEIVED_MSG = "Input set ";
static const string CODE_SET_MSG = "Code set ";

static const string COMMANDS_RECOGNIZED_MSG = "Commands recognized ";
static const string COMMANDS_EXECUTED_MSG = "Commands executed ";
static const string CALLED_MSG = "called ";

static const string IN_TAPE_MSG = "Input: ";
static const string OUT_TAPE_MSG = "Output: ";

static const string RUNNING_MSG = "RUNNING... ";
static const string FINISHED_MSG = "PROGRAM FINISHED ";

#define LOG(msg) do { if (out_) { *out_ << msg << endl; } } while(false)
#define LOG_VERBOSE(msg) do { if (verbose_) { LOG(msg); } } while(false)

#define GET_ARG(m, c)                                                        \
    RAM::Machine::GROUPS.at((c)->ctype_).at((c)->atype_)((m), (c)->arg_)

#define GET_VALUE(m, c) get<int>(GET_ARG((m), (c)))
#define GET_LABEL(m, c) get<string>(GET_ARG((m), (c)))

ostream &operator<<(ostream &os, const Arg &a) {
  if (a.index() == 0) { os << get<int>(a); }
  else { os << get<string>(a); }
  return os;
}

ostream &operator<<(ostream &os, const Tape &c) {
  os << "{ ";
  bool first = true;
  for (const auto &pos: c) {
	if (!first) { os << ", "; }
	first = false;
	os << pos;
  }
  return os << " }";
}

ostream &operator<<(ostream &os, const Memory &m) {
  os << "{ ";
  bool first = true;
  for (const auto &[key, value]: m) {
	if (!first) { os << ", "; }
	first = false;
	os << "(" << key << ")->" << value;
  }
  return os << " }";
}

static const set<ArgT> LABEL_COMS = {
	Machine::JUMP,
	Machine::JGTZ,
	Machine::JZERO,
	Machine::HALT,
};

static const map<char, ArgT> ARG_TYPES = {
	{EQ, Machine::VALUE},
	{PTR, Machine::ADDRESS_AT_ADDRESS},
};

Arg Machine::value_to_argument(const Machine &m, const Arg &a) {
  return get<int>(a);
}

Arg Machine::address_to_argument(const Machine &m, const Arg &a) {
  if (get<int>(a) < 0) {
	m.stop_ = true;
	return Arg{};
  }
  return m.memory_.at(get<int>(a));
}

Arg Machine::address_to_address_to_argument(const Machine &m, const Arg &a) {
  if (get<int>(a) < 0) {
	m.stop_ = true;
	return Arg{};
  }
  auto ct = m.memory_.at(get<int>(a));
  if (ct < 0) {
	m.stop_ = true;
	return Arg{};
  }
//  return address_to_argument(m, address_to_argument(m, a));
  return m.memory_.at(ct);
}

Arg Machine::label_to_argument(const Machine &m, const Arg &a) {
  return get<string>(a);
}

Com Machine::parse_command(const string &line) {
  auto ct = parse_command_t(line);
  auto at = (LABEL_COMS.contains(ct)) ? LABEL : parse_argument_t(line);
  auto l = parse_label(line);
  auto a = (LABEL_COMS.contains(ct)) ?
		   parse_argument_label(line) :
		   parse_argument_numeric(line);
  return {move(ct), move(at), move(a), move(l)};
}

ComT Machine::parse_command_t(const string &line) {
  auto it = COM_HAND.find(line.substr(0, line.find(LP)));
  if (it == COM_HAND.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgT Machine::parse_argument_t(const string &line) {
  auto it = ARG_TYPES.find(line.at(line.find(LP) + 1));
  return (it != ARG_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::parse_argument_numeric(const string &line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (ARG_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
  return stoi(arg);
}

Arg Machine::parse_argument_label(const string &line) {
  size_t lp = line.find(LP);
  return line.substr(lp + 1, line.find(RP) - lp - 1);
}

Lab Machine::parse_label(const string &line) {
  Lab ans;
  string extra = line.substr(line.find(RP) + 1, string::npos);
  if (extra.size() > 1) {
	ans = extra.substr(1, string::npos);
  }
  return ans;
}

Tape Machine::run() {
  auto it = commands_.begin();
  uint64_t executed = 0;

  LOG(IN_TAPE_MSG << input_);
  LOG_VERBOSE(RUNNING_MSG);

  while (!stop_ && it != commands_.cend()) {
	auto f = COM_HAND.at(it->ctype_);
	LOG_VERBOSE(memory_);
	++executed;
	it = f(*this, it);
  }

  LOG_VERBOSE(FINISHED_MSG);
  LOG_VERBOSE(COMMANDS_EXECUTED_MSG << executed);
  LOG(OUT_TAPE_MSG << output_);
  return output_;
}

void Machine::set_code(istream &is) {
  if (!is) {
	LOG(BAD_ISTREAM_ERROR);
	throw runtime_error(BAD_ISTREAM_ERROR);
  }
  string buffer;
  while (getline(is, buffer)) {
	if (buffer.empty()) {
	  continue;
	}
	auto inserted = process_command(buffer);
	if (inserted->label_.has_value()) {
	  add_label(inserted);
	}
  }
  LOG(CODE_SET_MSG);
  LOG_VERBOSE(COMMANDS_RECOGNIZED_MSG << commands_.size());
}

void Machine::set_input(istream &is) {
  if (!is) {
	LOG(BAD_ISTREAM_ERROR);
	throw runtime_error(BAD_ISTREAM_ERROR);
  }
  string s;
  while (getline(is, s, SP)) {
	input_.push_back(stoi(s));
  }
  LOG(INPUT_RECEIVED_MSG << input_);
}

void Machine::set_log_stream(ostream &os) {
  out_ = &os;
}

void Machine::set_input(initializer_list<Val> vals) {
  input_.assign(vals);
}

void Machine::be_verbose(bool flag) {
  verbose_ = flag;
}

//void Machine::process_code(istream &is) {
//  if (!is) {
//	throw runtime_error(BAD_ISTREAM_ERROR);
//  }
//  string buffer;
//  while (getline(is, buffer)) {
//	if (buffer.empty()) {
//	  continue;
//	}
//	auto inserted = process_command(buffer);
//	if (inserted->label_.has_value()) {
//	  add_label(inserted);
//	}
//  }
//  *out_ << CODE_SET_MSG << endl;
//  if (verbose_) {
//	*out_ << COMMANDS_RECOGNIZED_MSG << commands_.size() << endl;
//  }
//}

ComIt Machine::process_command(const string &line) {
  Com c = parse_command(line);
  return commands_.insert(commands_.end(), move(c));
}

void Machine::add_label(ComIt it) {
  if (labels_.contains(it->label_.value())) {
	LOG(LABEL_USED_ERROR);
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[it->label_.value()] = it;
}

ComIt Machine::load(Machine &m, ComIt &c) {
  m.memory_[0] = GET_VALUE(m, c);
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  auto buf = m.memory_.at(0);
  m.memory_[GET_VALUE(m, c)] = buf;
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] += buf;
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] -= buf;
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] *= buf;
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] /= buf;
  return ++c;
}

ComIt Machine::read(Machine &m, ComIt &c) {
  m.memory_[GET_VALUE(m, c)] = m.input_.front();
  m.input_.pop_front();
  return ++c;
}

ComIt Machine::write(Machine &m, ComIt &c) {
  m.output_.push_back(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::jump(Machine &m, ComIt &c) {
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::jgtz(Machine &m, ComIt &c) {
  if (m.memory_.at(0) == 0) {
	return ++c;
  }
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  if (m.memory_.at(0) != 0) {
	return ++c;
  }
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  return m.commands_.end();
}

}