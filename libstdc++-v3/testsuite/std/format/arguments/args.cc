// { dg-options "-std=gnu++20" }
// { dg-do run { target c++20 } }

#include <format>
#include <testsuite_hooks.h>

// LWG 4106. basic_format_args should not be default-constructible
static_assert( ! std::is_default_constructible_v<std::format_args> );
static_assert( ! std::is_default_constructible_v<std::wformat_args> );

template<typename Ctx, typename T>
bool equals(std::basic_format_arg<Ctx> fmt_arg, T expected) {
  return std::visit_format_arg([=](auto arg_val) {
    if constexpr (std::is_same_v<decltype(arg_val), T>)
      return arg_val == expected;
    else
      return false;
  }, fmt_arg);
}

void
test_empty()
{
  std::format_args args = std::make_format_args();
  VERIFY(!args.get(0));
  VERIFY(!args.get(1));
  VERIFY(!args.get((std::size_t)-1));
  VERIFY(equals(args.get(0), std::monostate{}));

  std::format_args cargs =  std::make_format_args<std::format_context>();
  VERIFY(!cargs.get(0));
  VERIFY(equals(cargs.get(0), std::monostate{}));

  std::wformat_args wargs = std::make_wformat_args();
  VERIFY(!wargs.get(0));
  VERIFY(equals(wargs.get(0), std::monostate{}));
}

enum E { ByGum };

template<>
struct std::formatter<E> : std::formatter<int>
{
  using std::formatter<int>::parse;

  std::format_context::iterator
  format(E e, std::format_context& fc) const
  { return std::formatter<int>::format((int)e, fc); }
};

void
test_args()
{
  bool b = false;
  int i = 1;
  char c = '2';
  double d = 3.4;

  auto store = std::make_format_args(b, i, c, d);
  std::format_args args = store;
  VERIFY(equals(args.get(0), false));
  VERIFY(equals(args.get(1), 1));
  VERIFY(equals(args.get(2), '2'));
  VERIFY(equals(args.get(3), 3.4));
  VERIFY(!args.get(4));

  long l = 5L;
  unsigned long long ull = 6ULL;
  float f = 7.8f;

  auto cstore = std::make_format_args<std::format_context>(l, ull, f);
  std::format_args cargs = cstore;
  if constexpr (sizeof(long) == sizeof(int))
    VERIFY(equals(cargs.get(0), 5));
  else
    VERIFY(equals(cargs.get(0), 5LL));
  VERIFY(equals(cargs.get(1), 6ULL));
  VERIFY(equals(cargs.get(2), 7.8f));
  VERIFY(!cargs.get(3));

  std::string s = "tenfour";
  VERIFY(equals(std::format_args(std::make_format_args(s)).get(0), std::string_view("tenfour")));

  char nine = '9';
  wchar_t ten = L'X';
  // This needs to be on the stack so that testing pointer equality works.
  wchar_t eleven[] = L"eleven";
  long double twelve13 = 12.13L;
  std::wstring tenfour = L"tenfour";

  auto wstore = std::make_wformat_args(nine, ten, eleven, twelve13, tenfour);
  std::wformat_args wargs = wstore;
  VERIFY(equals(wargs.get(0), static_cast<wchar_t>('9')));
  VERIFY(equals(wargs.get(1), L'X'));
  VERIFY(equals(wargs.get(2), static_cast<const wchar_t*>(eleven)));
  VERIFY(equals(wargs.get(3), 12.13L));
  VERIFY(equals(wargs.get(4), std::wstring_view(tenfour)));
  VERIFY(!wargs.get(5));

  std::nullptr_t null;
  E eebygum = E::ByGum;
  auto another_store = std::make_format_args(null, eebygum);
  args = another_store;
  VERIFY(equals(args.get(0), static_cast<const void*>(nullptr)));
  using handle = std::basic_format_arg<std::format_context>::handle;
  auto is_handle = []<typename T>(T) { return std::is_same_v<T, handle>; };
  VERIFY(std::visit_format_arg(is_handle, args.get(1)));
}

int main()
{
  test_empty();
  test_args();
}
