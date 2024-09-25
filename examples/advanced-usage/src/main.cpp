#include <iostream>

#include "errors/error.hpp"
#include "tl/expected.hpp"

using errors::error_ptr;
using errors::make_error;
using errors::wrap;
using tl::expected;
using tl::unexpected;

class stack_t {
    public:
        expected<int, error_ptr> pop() noexcept;
        error_ptr push(int value) noexcept;

        static const int MAX_SIZE = 3;

    private:
        int data[MAX_SIZE];
        int top = 0;
};

// NOTE:
// A new error type for errors reported by stack_t class.
// An error type should records
// the machine friendly error context information.
// It can be used to generate developer and
// user friendly error message later.
struct stack_error_t : public errors::message_error {
    public:
        // NOTE:
        // Write a constructor whichs first two arguments are
        // `source_location` and `error_ptr`
        // to make this error type compatible with errors::make_error.
        stack_error_t(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                errors::source_location location,
#endif
                error_ptr &&cause, int top)
                : message_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                          std::move(location),
#endif
                          std::move(cause),
                          "stack error [top=" + std::to_string(top) + "]")
                , top(top)
        {
        }

        int top;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(make_error<stack_error_t>(nullptr, top));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return make_error<stack_error_t>(nullptr, top);
        }

        data[top++] = value;
        return nullptr;
}

// NOTE:
// This function prints the error message for stack_error_t.
// It is used to generate user friendly error messages.
void print_stack_error(const error_ptr &err)
{
        if (!err) {
                return;
        }

        if (!err->is<stack_error_t>()) {
                return;
        }

        std::cout << "Stack error occurs" << std::endl;

        auto stack_error = err->as<stack_error_t>();
        assert(stack_error != nullptr);

        if (stack_error->top == 0) {
                std::cout << "Stack underflow" << std::endl;
                return;
        }

        if (stack_error->top == stack_t::MAX_SIZE) {
                std::cout << "Stack overflow" << std::endl;
                return;
        }

        std::cout << "Unknown stack error" << std::endl;
}

error_ptr fn(stack_t &stack)
{
        auto value = stack.pop();
        assert(!value);
        return wrap(wrap(wrap(std::move(value).error())),
                    "something goes wrong");
}

int main()
{
        stack_t stack;

        auto err = fn(stack);
        // NOTE:
        // Print logs for developer here.
        std::cerr << "Error: " << err << std::endl;

        // NOTE:
        // Print errors for user here.
        print_stack_error(err);

        err = stack.push(1);
        assert(err == nullptr);
        err = stack.push(2);
        assert(err == nullptr);
        err = stack.push(3);
        assert(err == nullptr);

        err = stack.push(4);
        assert(err != nullptr);
        print_stack_error(err);

        err = make_error<stack_error_t>(nullptr, 1);
        print_stack_error(err);

        return 0;
}
