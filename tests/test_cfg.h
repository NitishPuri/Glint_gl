
#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
namespace ut = boost::ut;

namespace cfg {
class reporter : public ut::reporter<ut::printer> {
 public:
  using BaseReporter = ut::reporter<ut::printer>;

  auto on(ut::events::test_begin test) -> void { std::cout << "[TEST] Running: " << test.name << std::endl; }
  //   auto on(ut::events::suite_begin test) -> void { std::cout << "[SUITE] Running: " << test.name << std::endl; }
  //   auto on(ut::events::suite_end test) -> void { std::cout << "[SUITE] Finished: " << test.name << std::endl; }
  auto on(ut::events::test_skip test) -> void { std::cout << "[SKIP] Skipped: " << test.name << std::endl; }
  //   auto on(ut::events::test_end test) -> void { std::cout << "[DONE] Finished: " << test.name << std::endl; }

  auto operator=(const ut::options& options) -> reporter& {
    BaseReporter::operator=({options.colors});
    return *this;
  }

  // Add this operator to handle initializer list assignments
  template <class T>
  auto operator=(std::initializer_list<T> il) -> reporter& {
    // ut::reporter<ut::printer>::operator=(il);
    return *this;
  }

  using BaseReporter::on;
};

template <class Reporter = reporter>
class runner : public ut::runner<Reporter> {
 public:
  auto& operator=(const ut::options& options) {
    ut::runner<Reporter>::operator=(options);
    return *this;
  }
};
}  // namespace cfg
