
#include "common.h"

namespace jt {

NulloptT Nullopt;

bool starts_with(const String& str, const char* is_prefix){
	const char* s = str.c_str(), *p = is_prefix;
	for (; *p; ++s, ++p) {
		if (*s != *p || !*s && *p) // Not equal prefix or string too 
			return false;          // short for prefix
	}
	return true;
}

bool starts_with(const String& str, const String& is_prefix) {
    return is_prefix.size() <= str.size() 
		&& str.compare(0, is_prefix.size(), is_prefix) == 0;
}

} // namespace jt {