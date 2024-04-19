
#ifndef __INICONFIG_H__
#define __INICONFIG_H__

#include <cstring>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

typedef enum config_error {
  NO_ERROR = 0,
  SECTION_NOTFOUND,
  VALUE_NOTFOUND,
} config_error_t;

class INIConfig {
public:
  INIConfig(){};
  INIConfig(std::string filename);
  INIConfig(FILE *file);

  static INIConfig *GetInstance() { return sInstance; }
  void Init(std::string filename);
  void Init(FILE *file);

  void write() const;
  void show_config() const;
  config_error_t CheckAttbute(const std::string &section,
                              const std::string &name) const;

  template <typename T>
  T GetAttbute(const std::string &section, const std::string &name) {
    std::string value;
    config_error_t ret;
    ret = GetValue(section, name, value);
    if constexpr (std::is_same<T, std::string>()) return static_cast<T>(value);
    else if constexpr (std::is_same<T, int>())    return static_cast<T>(std::stoi(value));
    else if constexpr (std::is_same<T, double>()) return static_cast<T>(std::stod(value));
    else if constexpr (std::is_same<T, bool>())   return static_cast<T>(BoolConverter(value));
    else
      throw std::runtime_error("Unsupported type " + section + " " + name);
  };

  void DelAttribute(const std::string &section, const std::string &name);

  void DelSection(const std::string &section);

  template <typename T>
  void SetAttribute(const std::string &section, const std::string &name,
                    const T &v) {
    _values[section][name] = V2String(v);
  };

  template <typename T>
  void SetAttribute(const std::string &section, const std::string &name,
                    const std::vector<T> &vs) {
    _values[section][name] = Vec2String(vs);
  };

  template <typename T>
  std::vector<T> GetVector(const std::string &section,
                           const std::string &name) const {
    std::string value;
    config_error_t ret;
    ret = GetValue(section, name, value);
    std::istringstream out{value};
    const std::vector<std::string> strs{std::istream_iterator<std::string>{out},
                                        std::istream_iterator<std::string>()};
    try {
      std::vector<T> vs{};
      for (const std::string &s : strs) vs.emplace_back(Converter<T>(s));
      return vs;
    } catch (std::exception &e) {
      throw std::runtime_error("Parse vector<T> file" + value);
    }
  };

  template <typename T>
  std::vector<T> GetVector(const std::string &section, const std::string &name,
                           const std::vector<T> &default_v) const {
    try {
      return GetVector<T>(section, name);
    } catch (std::runtime_error &e) {
      return default_v;
    };
  };

private:
  static INIConfig *sInstance;
  static std::string configFilename;
  static FILE *configFile;

protected:
  int _error;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      _values;

  int ParseError() const;
  const std::set<std::string> Sections() const;
  const std::set<std::string> Keys(std::string section) const;
  config_error_t GetSection(std::string section, 
                            std::unordered_map<std::string, std::string> &val) const;
  config_error_t GetValue(const std::string &section, const std::string &name,
                          std::string &val) const;
  static int ValueHandler(void *user, const char *section, const char *name,
                          const char *value);
  bool BoolConverter(std::string s) const;

  template <typename T> inline T Converter(const std::string &s) const {
    try {
      T v{};
      std::istringstream _{s};
      _.exceptions(std::ios::failbit);
      _ >> v;
      return v;
    } catch (std::exception &e) {
      throw std::runtime_error("cannot parse value '" + s + "' to type<T>.");
    };
  }

  template <typename T> inline std::string V2String(const T &v) const {
    std::stringstream ss;
    ss << v;
    return ss.str();
  }

  template <typename T>
  inline std::string Vec2String(const std::vector<T> &v) const {
    if (v.empty()) return "";
    std::ostringstream oss;
    std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(oss, " "));
    oss << v.back();
    return oss.str();
  }
};

#endif // __INICONFIG_H__
