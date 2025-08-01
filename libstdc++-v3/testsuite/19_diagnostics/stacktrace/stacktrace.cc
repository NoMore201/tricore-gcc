// { dg-options "-std=gnu++23 -lstdc++exp" }
// { dg-do run { target c++23 } }
// { dg-require-effective-target stacktrace }

#include <stacktrace>
#include "testsuite_allocator.h"

static_assert( std::is_nothrow_default_constructible_v<std::stacktrace> );
static_assert( std::is_copy_constructible_v<std::stacktrace> );
static_assert( std::is_nothrow_move_constructible_v<std::stacktrace> );
static_assert( std::is_copy_assignable_v<std::stacktrace> );
static_assert( std::is_nothrow_move_assignable_v<std::stacktrace> );
static_assert( std::is_nothrow_swappable_v<std::stacktrace> );

void
test_cons()
{
  {
    using Stacktrace = std::stacktrace;
    using Alloc = Stacktrace::allocator_type;

    Stacktrace s0;
    VERIFY( s0.empty() );
    VERIFY( s0.size() == 0 );
    VERIFY( s0.begin() == s0.end() );

    Stacktrace s1(Alloc{});
    VERIFY( s1.empty() );
    VERIFY( s1.size() == 0 );
    VERIFY( s1.begin() == s1.end() );

    VERIFY( s0 == s1 );

    Stacktrace s2(s0);
    VERIFY( s2 == s0 );

    const Stacktrace curr = Stacktrace::current();

    Stacktrace s3(curr);
    VERIFY( ! s3.empty() );
    VERIFY( s3.size() != 0 );
    VERIFY( s3.begin() != s3.end() );
    VERIFY( s3 != s0 );

    Stacktrace s4(s3);
    VERIFY( ! s4.empty() );
    VERIFY( s4.size() != 0 );
    VERIFY( s4.begin() != s4.end() );
    VERIFY( s4 == s3 );
    VERIFY( s4 != s0 );

    Stacktrace s5(std::move(s3));
    VERIFY( ! s5.empty() );
    VERIFY( s5.size() != 0 );
    VERIFY( s5.begin() != s5.end() );
    VERIFY( s5 == s4 );
    VERIFY( s5 != s0 );
    VERIFY( s3 == s0 );

    Stacktrace s6(s4, Alloc{});
    VERIFY( s6 == s4 );

    Stacktrace s7(std::move(s6), Alloc{});
    VERIFY( s7 == s4 );
  }

  {
    using Alloc = __gnu_test::uneq_allocator<std::stacktrace_entry>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s0;
    VERIFY( s0.empty() );
    VERIFY( s0.size() == 0 );
    VERIFY( s0.begin() == s0.end() );

    Stacktrace s1(Alloc{});
    VERIFY( s1.empty() );
    VERIFY( s1.size() == 0 );
    VERIFY( s1.begin() == s1.end() );

    VERIFY( s0 == s1 );

    Stacktrace s2(s0);
    VERIFY( s2 == s0 );

    const Stacktrace curr = Stacktrace::current();

    Stacktrace s3(curr);
    VERIFY( ! s3.empty() );
    VERIFY( s3.size() != 0 );
    VERIFY( s3.begin() != s3.end() );
    VERIFY( s3 != s0 );

    Stacktrace s4(s3);
    VERIFY( ! s4.empty() );
    VERIFY( s4.size() != 0 );
    VERIFY( s4.begin() != s4.end() );
    VERIFY( s4 == s3 );
    VERIFY( s4 != s0 );

    Stacktrace s5(std::move(s3));
    VERIFY( ! s5.empty() );
    VERIFY( s5.size() != 0 );
    VERIFY( s5.begin() != s5.end() );
    VERIFY( s5 == s4 );
    VERIFY( s5 != s0 );
    VERIFY( s3 == s0 );

    Stacktrace s6(s5, Alloc{6});
    VERIFY( ! s6.empty() );
    VERIFY( s6.size() != 0 );
    VERIFY( s6.begin() != s6.end() );
    VERIFY( s6 == s5 );
    VERIFY( s5 != s0 );
    VERIFY( s6.get_allocator().get_personality() == 6 );

    Stacktrace s7(std::move(s6), Alloc{7});
    VERIFY( ! s7.empty() );
    VERIFY( s7.size() != 0 );
    VERIFY( s7.begin() != s7.end() );
    VERIFY( s7 == s5 );
    VERIFY( s5 != s0 );
    VERIFY( s7.get_allocator().get_personality() == 7 );
  }

  {
    using Alloc = __gnu_test::SimpleAllocator<std::stacktrace_entry>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s0;
    VERIFY( s0.empty() );
    VERIFY( s0.size() == 0 );
    VERIFY( s0.begin() == s0.end() );

    Stacktrace s1(Alloc{});
    VERIFY( s1.empty() );
    VERIFY( s1.size() == 0 );
    VERIFY( s1.begin() == s1.end() );

    VERIFY( s0 == s1 );

    Stacktrace s2(s0);
    VERIFY( s2 == s0 );

    const Stacktrace curr = Stacktrace::current();

    Stacktrace s3(curr);
    VERIFY( ! s3.empty() );
    VERIFY( s3.size() != 0 );
    VERIFY( s3.begin() != s3.end() );
    VERIFY( s3 != s0 );

    Stacktrace s4(s3);
    VERIFY( ! s4.empty() );
    VERIFY( s4.size() != 0 );
    VERIFY( s4.begin() != s4.end() );
    VERIFY( s4 == s3 );
    VERIFY( s4 != s0 );

    Stacktrace s5(std::move(s3));
    VERIFY( ! s5.empty() );
    VERIFY( s5.size() != 0 );
    VERIFY( s5.begin() != s5.end() );
    VERIFY( s5 == s4 );
    VERIFY( s5 != s0 );
    VERIFY( s3 == s0 );

    Stacktrace s6(s5, Alloc{});
    VERIFY( ! s6.empty() );
    VERIFY( s6.size() != 0 );
    VERIFY( s6.begin() != s6.end() );
    VERIFY( s6 == s5 );
    VERIFY( s5 != s0 );

    Stacktrace s7(std::move(s6), Alloc{});
    VERIFY( ! s7.empty() );
    VERIFY( s7.size() != 0 );
    VERIFY( s7.begin() != s7.end() );
    VERIFY( s7 == s5 );
    VERIFY( s5 != s0 );
  }

{
    using Stacktrace = std::pmr::stacktrace;
    using Alloc = Stacktrace::allocator_type;

    Stacktrace s0;
    VERIFY( s0.empty() );
    VERIFY( s0.size() == 0 );
    VERIFY( s0.begin() == s0.end() );

    Stacktrace s1(Alloc{});
    VERIFY( s1.empty() );
    VERIFY( s1.size() == 0 );
    VERIFY( s1.begin() == s1.end() );

    VERIFY( s0 == s1 );

    Stacktrace s2(s0);
    VERIFY( s2 == s0 );

    const Stacktrace curr = Stacktrace::current();

    Stacktrace s3(curr);
    VERIFY( ! s3.empty() );
    VERIFY( s3.size() != 0 );
    VERIFY( s3.begin() != s3.end() );
    VERIFY( s3 != s0 );

    Stacktrace s4(s3);
    VERIFY( ! s4.empty() );
    VERIFY( s4.size() != 0 );
    VERIFY( s4.begin() != s4.end() );
    VERIFY( s4 == s3 );
    VERIFY( s4 != s0 );

    Stacktrace s5(std::move(s3));
    VERIFY( ! s5.empty() );
    VERIFY( s5.size() != 0 );
    VERIFY( s5.begin() != s5.end() );
    VERIFY( s5 == s4 );
    VERIFY( s5 != s0 );
    VERIFY( s3 == s0 );

    __gnu_test::memory_resource mr;
    Stacktrace s6(s5, &mr);
    VERIFY( ! s6.empty() );
    VERIFY( s6.size() != 0 );
    VERIFY( s6.begin() != s6.end() );
    VERIFY( s6 == s5 );
    VERIFY( s5 != s0 );
    VERIFY( s6.get_allocator() != s5.get_allocator() );

    Stacktrace s7(std::move(s6), Alloc{});
    VERIFY( ! s7.empty() );
    VERIFY( s7.size() != 0 );
    VERIFY( s7.begin() != s7.end() );
    VERIFY( s7 == s5 );
    VERIFY( s5 != s0 );
    VERIFY( s7.get_allocator() != s6.get_allocator() );
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, false>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(s1);
    VERIFY( s2.get_allocator() != s1.get_allocator() );
    Stacktrace s3(std::move(s1));
    VERIFY( s3.get_allocator() == s1.get_allocator() );
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, true>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(s1);
    VERIFY( s2.get_allocator() == s1.get_allocator() );
    Stacktrace s3(std::move(s1));
    VERIFY( s3.get_allocator() == s1.get_allocator() );
  }
}

void
test_assign()
{
  {
    using Stacktrace = std::stacktrace;

    Stacktrace s0;
    s0 = s0;
    VERIFY( s0.empty() );
    s0 = std::move(s0);
    VERIFY( s0.empty() );

    Stacktrace s1 = Stacktrace::current();
    VERIFY( s1 != s0 );
    s0 = s1;
    VERIFY( s0 == s1 );
    VERIFY( s0.at(0).source_line() == (__LINE__ - 4) );

    s1 = Stacktrace::current();
    VERIFY( s1 != s0 );
    Stacktrace s2 = s0;
    Stacktrace s3 = s1;
    s0 = std::move(s1);
    VERIFY( s0 == s3 );
    VERIFY( s1 == s2 ); // ISO C++: valid but unspecified; GCC: swapped.
  }

  {
    using Alloc = __gnu_test::uneq_allocator<std::stacktrace_entry>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s0;
    s0 = s0;
    VERIFY( s0.empty() );
    s0 = std::move(s0);
    VERIFY( s0.empty() );

    Stacktrace s1 = Stacktrace::current();
    VERIFY( s1 != s0 );
    s0 = s1;
    VERIFY( s0 == s1 );

    s1 = Stacktrace::current(Alloc(__LINE__));
    VERIFY( s1 != s0 );
    s0 = std::move(s1);
    VERIFY( s0.at(0).source_line() == s0.get_allocator().get_personality() );
    VERIFY( s1.empty() ); // ISO C++: valid but unspecified; GCC: empty.

    Stacktrace s2 = Stacktrace::current(s0.get_allocator());
    Stacktrace s3 = s2;
    s2 = std::move(s0);
    VERIFY( s2.at(0).source_line() == s2.get_allocator().get_personality() );
    VERIFY( s0 == s3 ); // ISO C++: valid but unspecified, GCC: swapped.
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, false>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(Alloc{2});
    s2 = s1;
    VERIFY( s2.get_allocator() != s1.get_allocator() );
    s1 = std::move(s2);
    VERIFY( s1.get_allocator() != s2.get_allocator() );
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, true>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(Alloc{2});
    s2 = s1;
    VERIFY( s2.get_allocator() == s1.get_allocator() );
    s1 = Stacktrace::current(Alloc{3});
    VERIFY( s1.get_allocator().get_personality() == 3 );
  }
}

void
test_swap()
{
  {
    using Stacktrace = std::stacktrace;

    Stacktrace s0;
    Stacktrace s1 = Stacktrace::current();
    swap(s0, s1);
    VERIFY( s1.empty() );
    VERIFY( ! s0.empty() );
  }

  {
    using Alloc = __gnu_test::uneq_allocator<std::stacktrace_entry>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s0;
    Stacktrace s1 = Stacktrace::current();
    swap(s0, s1);
    VERIFY( s1.empty() );
    VERIFY( ! s0.empty() );
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, false>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(Alloc{1}); // Must use equal allocators when not propagating.
    swap(s1, s2);
    VERIFY( s1.get_allocator() == s2.get_allocator() );
  }

  {
    using Alloc
      = __gnu_test::propagating_allocator<std::stacktrace_entry, true>;
    using Stacktrace = std::basic_stacktrace<Alloc>;

    Stacktrace s1 = Stacktrace::current(Alloc{1});
    Stacktrace s2(Alloc{2});
    swap(s1, s2);
    VERIFY( s1.get_allocator().get_personality() == 2 );
    VERIFY( s2.get_allocator().get_personality() == 1 );
  }
}

void
test_pr105031()
{
  // PR libstdc++/105031
  // wrong constexpr if statement in operator=(basic_stacktrace&&)
  using Alloc = __gnu_test::uneq_allocator<std::stacktrace_entry>;
  std::basic_stacktrace<Alloc> s;
  s = auto(s);
}

void
test_pr115063()
{
  // PR libstdc++/115063
  // compilation error: std::basic_stracktrace::max_size()
  std::stacktrace s;
  VERIFY( s.max_size() != 0 );
}

int main()
{
  test_cons();
  test_assign();
  test_swap();
  test_pr105031();
  test_pr115063();
}
