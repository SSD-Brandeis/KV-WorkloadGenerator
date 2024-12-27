#ifndef KEY_H
#define KEY_H

#include <random>
#include <iostream>
#include <cstring>
#include <functional>

#define INTEGER_KEY_DOMAIN 1073741824 // 2^30
#define STRING_PREFIX_DIGITS 2
using namespace std;

class Key
{
public:
	bool string_enabled_;
	string key_str_;
	uint32_t key_int32_;
	static const char key_alphanum[];
	Key();
	Key(string key);
	Key(uint32_t key);
	bool operator<(const Key &t) const;
	bool operator==(const Key &t) const;
	Key operator+(const Key &t);
	friend ostream &operator<<(ostream &os, const Key &t);
	static Key get_key(int key_size, bool string_enabled = true);
};

namespace std {
    template <>
    struct hash<Key> {
        std::size_t operator()(const Key &k) const {
            if (k.string_enabled_) {
                return std::hash<std::string>()(k.key_str_);
            } else {
                return std::hash<uint32_t>()(k.key_int32_);
            }
        }
    };
}

#endif
