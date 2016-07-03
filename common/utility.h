#ifndef UTILITY_H__
#define UTILITY_H__

#include <string>
#include <sstream>
#include <array>
#include <memory>
#include <cstring>

namespace {
  std::string exec(const char* cmd) {
    std::string result;
    std::array<char, 128> buffer;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);

    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
      if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
        result += buffer.data();
    }

    return std::move(result);
  }

  template<typename T>
  class BitSet {
    T& m_bitset;

    public:
      BitSet(T& bitset)
        : m_bitset(bitset) {}

      class BitRef {
        BitSet<T> &m_ref;
        size_t     m_index;

        public:
          BitRef(BitSet<T> &ref, size_t index)
            : m_ref(ref), m_index(index) {}

          operator bool() {
            return (m_ref.m_bitset >> m_index) & 1UL;
          }

          bool operator=(bool x) {
            return m_ref.m_bitset ^= (-T(x) ^ m_ref.m_bitset) & (1UL << m_index);
          }
      };

      BitRef operator[](size_t index) {
        return BitRef(*this, index);
      }
  };

  template<typename T>
  BitSet<T> make_bitset(T&& bitset) {
    return BitSet<T>(std::forward<T>(bitset));
  }

  template <typename T>
  constexpr auto to_underlying(T t) noexcept {
    return static_cast<std::underlying_type_t<T>>(t);
  }

  template<typename T, typename F>
  struct Conv {
    T to_value;

    Conv(F&& v) {
      std::memcpy(&to_value, &v, sizeof(T));
    }

    operator T() {
      return to_value;
    }
  };

  template<typename T, typename F>
  T convert(F&& v) {
    return Conv<T, F>(std::forward<F>(v));
  }

  template<typename T>
  std::string to_hex(T v) {
    std::stringstream ss;
    ss << "0x" << std::hex << static_cast<unsigned>(v);
    return ss.str();
  };
}

#endif /* UTILITY_H__ */
