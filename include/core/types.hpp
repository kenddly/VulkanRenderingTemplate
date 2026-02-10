#pragma once
#include <memory>

namespace vks
{
    template<typename T>
    using Ref = std::shared_ptr<T>;
}
