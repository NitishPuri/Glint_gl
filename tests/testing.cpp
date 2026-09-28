#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

// template <>
// auto ut::cfg<ut::override> = cfg::runner<cfg::reporter>{};

constexpr auto sum(auto... values) { return (values + ...); }

void camera_test();

// End-to-end tests
void e2e_test();

int main(int argc, char** argv) {
  // setup filter
  const auto filter = argc > 1 ? argv[1] : "*";
  ut::cfg<ut::override> = ut::options{.filter = filter};

  using namespace boost::ut;
  "sum"_test = [] {
    expect(sum(0) == 0_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2) > 0_i and 42_i == sum(40, 2));
  };

  // Unit Tests
  camera_test();

  // E2E
  //   e2e_test();

  return 0;
}