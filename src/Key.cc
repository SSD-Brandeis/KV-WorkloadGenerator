#include "Key.h"
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
size_t hash<Key>::operator()(const Key &k) const {
  return k.string_enabled_ ? hash<std::string>()(k.key_str_)
                           : hash<uint32_t>()(k.key_int32_);
}
} // namespace std

// #include "Key.h"

// const char Key::key_alphanum[] =
// "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; Key::Key()
// {
//   string_enabled_ = true;
//   key_str_ = "";
// }

// Key::Key(string key)
// {
//   key_str_ = key;
//   string_enabled_ = true;
// }

// Key::Key(uint32_t key)
// {
//   key_int32_ = key;
//   string_enabled_ = false;
// }

// bool Key::operator<(const Key &t) const
// {
//   if (string_enabled_)
//   {
//     return key_str_ < t.key_str_;
//   }
//   else
//   {
//     return key_int32_ < t.key_int32_;
//   }
// }

// bool Key::operator==(const Key &t) const
// {
//     if (string_enabled_ != t.string_enabled_)
//     {
//         return false;
//     }
//     if (string_enabled_)
//     {
//         return key_str_ == t.key_str_;
//     }
//     else
//     {
//         return key_int32_ == t.key_int32_;
//     }
// }

// Key Key::operator+(const Key &t)
// {
//   if (string_enabled_)
//   {
//     return Key(this->key_str_ + t.key_str_);
//   }
//   else
//   {
//     return Key(this->key_int32_ + t.key_int32_);
//   }
// }

// Key Key::get_key(int _key_size, bool string_enabled)
// {
//   if (string_enabled)
//   {
//     char *s = new char[(int)_key_size + 1];
//     for (int i = 0; i < _key_size; ++i)
//     {
//       s[i] = key_alphanum[rand() % (sizeof(key_alphanum) - 1)];
//     }
//     s[_key_size] = '\0';
//     Key key = Key(std::string(s));  // (shubham): fixes dangling pointer for
//     s delete[] s; return key;
//   }
//   else
//   {
//     // here key_size means bits in fact
//     uint32_t domain_size = (uint32_t)(pow(2, _key_size) - 1);
//     return Key((uint32_t)(rand() * rand()) % domain_size);
//   }
// }

// ostream &operator<<(ostream &os, const Key &t)
// {
//   if (t.string_enabled_)
//   {
//     os << t.key_str_;
//   }
//   else
//   {
//     os << t.key_int32_;
//   }
//   return os;
// }
