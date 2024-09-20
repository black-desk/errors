#include <iostream>

#include "errors/error.hpp"
#include "tl/expected.hpp"

namespace example1
{
void run() noexcept;
}

int main()
{
        example1::run();
        return 0;
}

namespace example1
{

// Stack class that can push and pop integer elements.
class Stack {
    public:
        tl::expected<void, errors::error_ptr> push(int value) noexcept
        {
                if (top == MAX_SIZE) {
                        return tl::unexpected(
                                errors::make_error<errors::common_error>(
                                        "overflow"));
                }
                data[top++] = value;
                return {};
        }
        tl::expected<int, errors::error_ptr> pop() noexcept
        {
                if (top == 0) {
                        return tl::unexpected(
                                errors::make_error<errors::common_error>(
                                        "underflow"));
                }

                return data[--top];
        }

    private:
        static const int MAX_SIZE = 3;
        int data[MAX_SIZE];
        int top = 0;
};

// Example of using with tl::expected
void run() noexcept
{
        Stack stack;

        auto value = stack.pop();
        assert(!value);
        std::cerr << "Failed to pop element from stack: " << value.error()
                  << std::endl;

        auto result = stack.push(1);
        assert(result);
        result = stack.push(2);
        assert(result);
        result = stack.push(3);
        assert(result);

        result = stack.push(4);
        assert(!result);
        std::cerr << "Failed to push element to stack: " << result.error()
                  << std::endl;

}
}
