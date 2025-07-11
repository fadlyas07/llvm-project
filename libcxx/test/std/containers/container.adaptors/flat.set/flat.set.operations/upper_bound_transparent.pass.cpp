//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

// <flat_set>

// template<class K> iterator       upper_bound(const K& x);
// template<class K> const_iterator upper_bound(const K& x) const;

#include <cassert>
#include <deque>
#include <flat_set>
#include <functional>
#include <string>
#include <utility>

#include "MinSequenceContainer.h"
#include "../helpers.h"
#include "test_macros.h"
#include "min_allocator.h"

// Constraints: The qualified-id Compare::is_transparent is valid and denotes a type.
template <class M>
concept CanUpperBound   = requires(M m, Transparent<int> k) { m.upper_bound(k); };
using TransparentSet    = std::flat_set<int, TransparentComparator>;
using NonTransparentSet = std::flat_set<int, NonTransparentComparator>;
static_assert(CanUpperBound<TransparentSet>);
static_assert(CanUpperBound<const TransparentSet>);
static_assert(!CanUpperBound<NonTransparentSet>);
static_assert(!CanUpperBound<const NonTransparentSet>);

template <class KeyContainer>
constexpr void test_one() {
  using Key = typename KeyContainer::value_type;
  using M   = std::flat_set<Key, TransparentComparator, KeyContainer>;

  {
    M m            = {"alpha", "beta", "epsilon", "eta", "gamma"};
    const auto& cm = m;
    ASSERT_SAME_TYPE(decltype(m.lower_bound(Transparent<std::string>{"abc"})), typename M::iterator);
    ASSERT_SAME_TYPE(decltype(std::as_const(m).lower_bound(Transparent<std::string>{"b"})), typename M::const_iterator);

    auto test_upper_bound = [&](auto&& map, const std::string& expected_key, long expected_offset) {
      auto iter = map.upper_bound(Transparent<std::string>{expected_key});
      assert(iter - map.begin() == expected_offset);
    };

    test_upper_bound(m, "abc", 0);
    test_upper_bound(m, "alpha", 1);
    test_upper_bound(m, "beta", 2);
    test_upper_bound(m, "bets", 2);
    test_upper_bound(m, "charlie", 2);
    test_upper_bound(m, "echo", 2);
    test_upper_bound(m, "epsilon", 3);
    test_upper_bound(m, "eta", 4);
    test_upper_bound(m, "gamma", 5);
    test_upper_bound(m, "golf", 5);
    test_upper_bound(m, "zzz", 5);

    test_upper_bound(cm, "abc", 0);
    test_upper_bound(cm, "alpha", 1);
    test_upper_bound(cm, "beta", 2);
    test_upper_bound(cm, "bets", 2);
    test_upper_bound(cm, "charlie", 2);
    test_upper_bound(cm, "echo", 2);
    test_upper_bound(cm, "epsilon", 3);
    test_upper_bound(cm, "eta", 4);
    test_upper_bound(cm, "gamma", 5);
    test_upper_bound(cm, "golf", 5);
    test_upper_bound(cm, "zzz", 5);
  }
  {
    // empty
    M m;
    auto iter = m.upper_bound(Transparent<std::string>{"a"});
    assert(iter == m.end());
  }
}

constexpr bool test() {
  test_one<std::vector<std::string>>();
#ifndef __cpp_lib_constexpr_deque
  if (!TEST_IS_CONSTANT_EVALUATED)
#endif
    test_one<std::deque<std::string>>();
  test_one<MinSequenceContainer<std::string>>();
  test_one<std::vector<std::string, min_allocator<std::string>>>();

  {
    bool transparent_used = false;
    TransparentComparator c(transparent_used);
    std::flat_set<int, TransparentComparator> m(std::sorted_unique, {1, 2, 3}, c);
    assert(!transparent_used);
    auto it = m.upper_bound(Transparent<int>{2});
    assert(it != m.end());
    assert(transparent_used);
  }
  {
    // LWG4239 std::string and C string literal
    using M = std::flat_set<std::string, std::less<>>;
    M m{"alpha", "beta", "epsilon", "eta", "gamma"};
    auto it = m.upper_bound("beta");
    assert(it == m.begin() + 2);
    auto it2 = m.upper_bound("beta2");
    assert(it2 == m.begin() + 2);
  }

  return true;
}

int main(int, char**) {
  test();
#if TEST_STD_VER >= 26
  static_assert(test());
#endif

  return 0;
}
