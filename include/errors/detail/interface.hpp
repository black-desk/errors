#pragma once

namespace errors
{
namespace detail
{

// This is the abstract class for all interface type.
// Disable copy and move operations.
// See c++ core guidelines C.67 at
// <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c67-a-polymorphic-class-should-suppress-public-copymove>
class interface {
    public:
        interface() = default;
        virtual ~interface() = default;

        interface(const interface &) = delete;
        interface(interface &&) = delete;
        interface &operator=(interface &&) = delete;
        interface &operator=(const interface &) = delete;
};

}
}
