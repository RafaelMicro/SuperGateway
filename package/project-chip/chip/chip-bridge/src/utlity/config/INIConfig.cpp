
#include "INIConfig.h"
#include "ini.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <set>
#include <string>
#include <unordered_map>

INIConfig *INIConfig::sInstance;
std::string INIConfig::configFilename;
FILE *INIConfig::configFile;

INIConfig::INIConfig(std::string filename) {
  _error = inih::ini_parse(filename.c_str(), ValueHandler, this);
  ParseError();
}

INIConfig::INIConfig(FILE *file) {
  _error = inih::ini_parse_file(file, ValueHandler, this);
  ParseError();
}

void INIConfig::Init(std::string filename) {
  configFilename = filename;
  sInstance = new INIConfig(configFilename);
};

void INIConfig::Init(FILE *file) {
  configFile = file;
  sInstance = new INIConfig(configFile);
};

inline int INIConfig::ParseError() const {
  switch (_error) {
  case 0:
    break;
  case -1:
    throw std::runtime_error("ini file NotFound.");
  case -2:
    throw std::runtime_error("memory alloc error");
  default:
    throw std::runtime_error("parse error: " + std::to_string(_error));
  }
  return 0;
}

const std::set<std::string> INIConfig::Sections() const {
  std::set<std::string> retval;
  for (auto const &element : _values)
    retval.insert(element.first);
  return retval;
}

const std::unordered_map<std::string, std::string>
INIConfig::GetSection(std::string section) const {
  auto const _section = _values.find(section);
  if (_section == _values.end())
    throw std::runtime_error("Section NotFound " + section);
  return _section->second;
}

const std::set<std::string> INIConfig::Keys(std::string section) const {
  std::set<std::string> retval;
  for (auto const &element : GetSection(section))
    retval.insert(element.first);
  return retval;
}

std::string INIConfig::GetValue(const std::string &section,
                                const std::string &name) const {
  auto const _section = GetSection(section);
  auto const _value = _section.find(name);
  if (_value == _section.end())
    throw std::runtime_error("Value NotFound " + section + " " + name);
  return _value->second;
}

inline bool INIConfig::BoolConverter(std::string s) const {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  static const std::unordered_map<std::string, bool> s2b{
      {"1", true},  {"true", true},   {"yes", true}, {"on", true},
      {"0", false}, {"false", false}, {"no", false}, {"off", false},
  };
  auto const value = s2b.find(s);
  if (value == s2b.end())
    throw std::runtime_error("Invalid boolean value " + s);
  return value->second;
};

inline int INIConfig::ValueHandler(void *user, const char *section,
                                   const char *name, const char *value) {
  INIConfig *reader = (INIConfig *)user;
  if (reader->_values[section][name].size() > 0)
    throw std::runtime_error("Duplicate " + std::string(section) + " " + name);
  reader->_values[section][name] = value;
  return 1;
}

void INIConfig::write() const {
  std::ofstream out;
  out.open(configFilename);
  if (!out.is_open())
    throw std::runtime_error("cannot open file: " + configFilename);
  for (const auto &section : Sections()) {
    out << "[" << section << "]\n";
    for (const auto &key : Keys(section))
      out << key << "=" << GetValue(section, key) << "\n";
  }
  out.close();
}

void INIConfig::show_config() const {
  for (const auto &section : Sections()) {
    printf("[%s]\n", section.c_str());
    for (const auto &key : Keys(section))
      printf("\t%s: %s\n", key.c_str(), (GetValue(section, key)).c_str());
  }
}
