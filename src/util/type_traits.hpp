#pragma once
#include <type_traits>

namespace coolc::util {

template <typename T, typename... Args>
constexpr inline bool IsOneOfV = (... || std::is_same_v<T, Args>);

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}  // namespace coolc::util
