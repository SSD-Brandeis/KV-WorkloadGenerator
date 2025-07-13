#include "key.h"
#include <cmath>
#include <memory>
#include <random>

Key::Key() : string_enabled_(true), key_str_("") {}

Key::Key(const std::string &key) : string_enabled_(true), key_str_(key) {}

Key::Key(uint32_t key) : string_enabled_(false), key_int32_(key) {}

bool Key::operator<(const Key &t) const {
  if (string_enabled_ && t.string_enabled_) {
    return key_str_ < t.key_str_;
  } else {
    return key_int32_ < t.key_int32_;
  }
}

bool Key::operator==(const Key &t) const {
  if (string_enabled_ != t.string_enabled_)
    return false;
  return string_enabled_ ? (key_str_ == t.key_str_)
                         : (key_int32_ == t.key_int32_);
}

Key Key::operator+(const Key &t) const {
  return string_enabled_ ? Key(key_str_ + t.key_str_)
                         : Key(key_int32_ + t.key_int32_);
}

Key Key::get_key(int key_size, bool string_enabled) {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  if (string_enabled) {
    std::uniform_int_distribution<> dis(0, 61); // 62 characters

    std::string result;
    result.reserve(key_size);
    for (int i = 0; i < key_size; ++i) {
      result += key_alphanum[dis(gen)];
    }
    return Key(result);
  } else {
    uint32_t domain_size = (1U << key_size) - 1;
    std::uniform_int_distribution<uint32_t> dis(0, domain_size);
    return Key(dis(gen));
  }
}

std::ostream &operator<<(std::ostream &os, const Key &t) {
  return t.string_enabled_ ? os << t.key_str_ : os << t.key_int32_;
}

namespace std {
  template <>
  struct hash<Key> {
    size_t operator()(const Key &k) const {
      return k.string_enabled_ ? hash<std::string>()(k.key_str_)
                               : hash<uint32_t>()(k.key_int32_);
    }
  };
} // namespace std