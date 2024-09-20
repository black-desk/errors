#include <iostream>

#include "errors/error.hpp"
#include "tl/expected.hpp"

using errors::common_error;
using errors::error_ptr;
using errors::make_error;
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
class stack_error_t : public errors::base_error {
    public:
        // NOTE:
        // An error context class records
        // the machine friendly error context information.
        // It can be used to generate developer and
        // user friendly error message later.
        struct context_t : public errors::context {
                context_t(int top, errors::source_location location =
                                           errors::source_location::current())
                        : ::errors::context(location)
                        , top(top)
                {
                }
                int top;
        };

        stack_error_t(context_t context, error_ptr &&cause = nullptr)
                : base_error(std::move(cause))
                , context(std::move(context))
        {
        }

        const errors::source_location &location() const override
        {
                return this->context.location;
        }

        // NOTE:
        // This method is used to generate error messages
        // meant to be printed in logs,
        // which should be developer friendly.
        std::optional<std::string> what() const override
        {
                std::string result;
                result.append("stack error [top=");
                result.append(std::to_string(context.top));
                result.append("]");
                return result;
        }

        context_t context;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(make_error<stack_error_t>(top));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return make_error<stack_error_t>(top);
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

        if (stack_error->context.top == 0) {
                std::cout << "Stack underflow" << std::endl;
                return;
        }

        if (stack_error->context.top == stack_t::MAX_SIZE) {
                std::cout << "Stack overflow" << std::endl;
                return;
        }

        std::cout << "Unknown stack error" << std::endl;
}

error_ptr fn(stack_t &stack)
{
        auto value = stack.pop();
        assert(!value);
        return errors::wrap(
                errors::wrap(errors::wrap(std::move(value).error())),
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

        err = make_error<stack_error_t>(1);
        print_stack_error(err);

        return 0;
}
