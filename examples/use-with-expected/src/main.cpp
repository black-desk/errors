#include <iostream>

#include "errors/errors.hpp"
#include "tl/expected.hpp"

using errors::error_ptr;
using errors::impl::runtime_error;
using tl::expected;
using tl::unexpected;

// NOTE:
// If you have a `Stack` class that can push and pop integer elements.
class stack_t {
    public:
        // NOTE:
        // You will have some functions that has return value in a semantic way,
        // like the `pop` function.
        // It is **RECOMMEND** use `std::expected` (or `tl::expected` for c++11)
        // to return the expected value when function return correctly
        // and the error_ptr when something goes wrong.
        expected<int, error_ptr> pop() noexcept;

        // NOTE:
        // You can also use `std::expected` (or `tl::expected` for c++11)
        // for the `push` function which returns nothing.
        // But its somekind of wired,
        // because you can just simply return `error_ptr`.
        error_ptr push(int value) noexcept;

    private:
        static const int MAX_SIZE = 3;
        int data[MAX_SIZE];
        int top = 0;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(
                        errors::make<runtime_error>::with("underflow"));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return errors::make<runtime_error>::with("overflow");
        }

        data[top++] = value;
        return nullptr;
}

int main()
{
        stack_t stack;

        auto value = stack.pop();
        assert(!value);
        std::cerr << "Failed to pop element from stack: " << value.error()
                  << std::endl;

        auto err = stack.push(1);
        assert(err == nullptr);
        err = stack.push(2);
        assert(err == nullptr);
        err = stack.push(3);
        assert(err == nullptr);

        err = stack.push(4);
        assert(err != nullptr);
        std::cerr << "Failed to push element to stack: " << err << std::endl;
        return 0;
}
