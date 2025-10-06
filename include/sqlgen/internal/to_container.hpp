#ifndef SQLGEN_INTERNAL_TOCONTAINER_HPP_
#define SQLGEN_INTERNAL_TOCONTAINER_HPP_

#include "../Range.hpp"
#include "../Ref.hpp"
#include "../Result.hpp"
#include "../transpilation/value_t.hpp"
#include "is_range.hpp"

namespace sqlgen::internal {

template <class ContainerType, class IteratorType>
Result<ContainerType> to_container(const Result<IteratorType>& _res) {
  if constexpr (internal::is_range_v<ContainerType>) {
    return _res.transform([](auto&& _it) { return ContainerType(_it); });

  } else {
    return to_container<Range<IteratorType>>(_res).and_then(
        [](auto range) -> Result<ContainerType> {
          ContainerType container;
          for (auto& res : range) {
            if (res) {
              container.emplace_back(std::move(*res));
            } else {
              return error(res.error().what());
            }
          }
          return container;
        });
  }
}

}  // namespace sqlgen::internal

#endif
