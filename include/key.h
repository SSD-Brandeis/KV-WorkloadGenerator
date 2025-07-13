#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#define INTEGER_KEY_DOMAIN 1073741824 // 2^30
#define STRING_PREFIX_DIGITS 2

class Key {
public:
  Key();
  explicit Key(const std::string &key);
  explicit Key(uint32_t key);

  bool operator<(const Key &t) const;
  bool operator==(const Key &t) const;
  Key operator+(const Key &t) const;

  friend std::ostream &operator<<(std::ostream &os, const Key &t);

  static Key get_key(int key_size, bool string_enabled = true);

private:
  bool string_enabled_ = true;
  std::string key_str_;
  uint32_t key_int32_ = 0;

  static constexpr const char *key_alphanum =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  friend struct std::hash<Key>;
};