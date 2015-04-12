// jt
//
#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <vector>
#include <fstream>

namespace jt {

typedef int8_t   i1;
typedef int16_t  i2;
typedef int32_t  i4;
typedef int64_t  i8;

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef float    r4;
typedef double   r8;

typedef std::string String;

template<typename T>
using cres_ptr = std::unique_ptr<T, std::function<void (T*)>>;

#define JT_CONCAT_IMPL(a, b) a##b
#define JT_CONCAT(a, b) JT_CONCAT_IMPL(a, b)

#ifdef _WIN32
#	define JT_PLATFORM_WIN32 1
#else
#	define JT_PLATFORM_WIN32 0
#endif

#if !JT_PLATFORM_WIN32
#	define JT_PLATFORM_MAC 1
#else
#	define JT_PLATFORM_MAC 0
#endif

#if JT_PLATFORM_WIN32 + JT_PLATFORM_MAC > 1
#	error "Too many platforms activated"
#endif


template <class T>
class TypeStorage {
public:
	TypeStorage()
	:	stored_(false) {}

	TypeStorage(T&& t)
	:	stored_(true) {
		new (&store_) T(std::move(t));
	}

	TypeStorage(const T& t)
	:	stored_(true) {
		new (&store_) T(t);
	}

	TypeStorage(const TypeStorage& t)
	:	stored_(t.stored_) {
		if (t.stored_)
			new (&store_) T(*t);
	}

	~TypeStorage() {
		reset();
	}

	void reset() {
		if (stored_) {
			c_cast<T*>()->~T();
			stored_ = false;
		}
	}

	void set(T&& t) {
		reset();
		new (&store_) T(std::move(t));
		stored_ = true;
	}

	void set(const T& t) {
		reset();
		new (&store_) T(t);
		stored_ = true;
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

	//explicit Optional(Optional&& opt)
	//:	store_(std::move(opt.store_)) {}

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

template <typename ContA, typename ContB>
class Zip {
public:
	typedef typename ContA::value_type ValA;
	typedef typename ContB::value_type ValB;

	Zip(ContA& a, ContB& b)
	:	begin_(a.begin(), b.begin()),
		end_(a.end(), b.end()) {
	}

	template <typename IterA, typename IterB>
	class Iter {
	private:
		friend class Zip;

		Iter(IterA ait, IterB bit) 
		:	ait_(ait), bit_(bit) {}

	public:
		Iter& operator ++() {
			++ait_, ++bit_;
			return *this;
		}

		bool operator == (const Iter& other) const {
			return ait_ == other.ait_ && bit_ == other.bit_;
		}

		bool operator != (const Iter& other) const {
			return !(*this == other);
		}

		Iter& operator * () {
			fst = &*ait_; snd = &*bit_;
			return *this;
		}

		ValA* fst;
		ValB* snd;

	private:
		IterA ait_;
		IterB bit_;
	};

	typedef Iter<typename ContA::iterator, typename ContB::iterator> It;

	It begin() const { return begin_; }
	It end() const { return end_; }

private:
	It begin_, end_;
};

template <typename ContA, typename ContB>
Zip<ContA, ContB> zip(ContA& a, ContB& b) {
	return {a, b};
}

class ParserContext;
class FuncTermImpl;
class Term;
class TermImpl;

template <typename Cont>
String str_join(const String& separator, const Cont& cont, std::function<String (const typename Cont::value_type&)> func) {
	String result;

	typename Cont::const_iterator i = begin(cont);
	typename Cont::const_iterator e = end(cont);

	if (i == e)
		return result;

	do {
		result += func(*i);
		++i;
		if (i != e)
			result += separator;
	} while (i != e);

	return result;
}

} // namespace jt {

#include "report.h"
