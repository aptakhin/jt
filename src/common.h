// jt
//

#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <tuple>
#define NOMINMAX
#include <windows.h>

#include "report.h"

namespace jt {

typedef std::int8_t   i1;
typedef std::int16_t  i2;
typedef std::int32_t  i4;
typedef std::int64_t  i8;

typedef std::uint8_t  u1;
typedef std::uint16_t u2;
typedef std::uint32_t u4;
typedef std::uint64_t u8;

typedef float    r4;
typedef double   r8;

typedef std::string String;

#define assert(Expr) { if (!(Expr)) { _CrtDbgBreak(); } }

template <class T>
class TypeStorage {
public:
	TypeStorage()
	:	stored_(false),
		dbg_val_(nullptr) {}

	TypeStorage(T&& t)
	:	stored_(true) {
		new (&store_) T(std::move(t));
		dbg_val_ = cast<const T*>();
	}

	TypeStorage(const T& t)
	:	stored_(true) {
		new (&store_) T(t);
		dbg_val_ = cast<const T*>();
	}

	TypeStorage(const TypeStorage& t)
	:	stored_(t.stored_) {
		if (t.stored_)
			new (&store_) T(*t), dbg_val_ = cast<const T*>();
	}

	~TypeStorage() {
		reset();
	}

	void reset() {
		if (stored_) {
			c_cast<T*>()->~T();
			stored_ = false;
			dbg_val_ = nullptr;
		}
	}

	void set(T&& t) {
		reset();
		new (&store_) T(std::move(t));
		stored_ = true;
		dbg_val_ = cast<const T*>();
	}

	void set(const T& t) {
		reset();
		new (&store_) T(t);
		stored_ = true;
		dbg_val_ = cast<const T*>();
	}

	T& operator * () {
		return *c_cast<T*>();
	}

	const T& operator * () const {
		return *cast<const T*>();
	}

	bool stored() const { return stored_; }

private:
	template <class Cast>
	Cast cast() const {
		return static_cast<Cast>(static_cast<const void*>(&store_));
	}

	template <class Cast>
	Cast c_cast() const {
		return static_cast<Cast>(const_cast<void*>(static_cast<const void*>(&store_)));
	}

private:
	typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type store_;
	const T* dbg_val_;

	bool stored_;
};

class NulloptT {};
extern NulloptT Nullopt;

template <class T>
class Optional {
public:
	Optional() {}

	explicit Optional(const T& t)
	:	store_(t) {}

	explicit Optional(T&& t)
	:	store_(t) {}

	Optional(NulloptT) {}

	explicit Optional(Optional&& opt)
	:	store_(std::move(opt.store_)) {}

	void operator = (const T& t) {
		store_.set(t);
	}

	void operator = (T&& t) {
		store_.set(t);
	}

	void operator = (NulloptT) {
		store_.reset();
	}

	T& operator * () {
		return *store_;
	}

	const T& operator * () const {
		return *store_;
	}

	operator bool() {
		return store_.stored();
	}

private:
	TypeStorage<T> store_;
};

template <class T>
Optional<T> make_optional(const T& t) {
	return Optional<T>(t);
}

template <typename T, typename... Args>
Optional<T> make_optional(Args... args) {
	return Optional<T>(std::move(T(args...)));
}

template <typename Container>
Optional<typename Container::mapped_type> iter2optional(typename Container::const_iterator iter, typename Container::const_iterator end) {
	if (iter == end)
		return Nullopt;
	else
		return make_optional(iter->second);
}

bool starts_with(const String& str, const char* is_prefix);
bool starts_with(const String& str, const String& is_prefix);

class ParserContext;
class FuncTermImpl;

} // namespace jt {