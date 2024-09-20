#include <filesystem>
#include <iostream>

#include "errors/error.hpp"

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
        errors::error_ptr push(int value) noexcept
        {
                if (top == MAX_SIZE) {
                        return errors::make_error<errors::common_error>(
                                "overflow");
                }
                data[top++] = value;
                return nullptr;
        }
        errors::error_ptr pop(int &value) noexcept
        {
                if (top == 0) {
                        return errors::make_error<errors::common_error>(
                                "underflow");
                }

                value = data[--top];
                return nullptr;
        }

    private:
        static const int MAX_SIZE = 3;
        int data[MAX_SIZE];
        int top = 0;
};

namespace custom_print
{
std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err);
};

// Example of printing error message
void run() noexcept
{
        Stack stack;
        int value;

        auto err = stack.pop(value);

        assert(err != nullptr);
        std::cerr << "Failed to pop element from stack: " << err << std::endl;

        err = stack.push(1);
        assert(err == nullptr);
        err = stack.push(2);
        assert(err == nullptr);
        err = stack.push(3);
        assert(err == nullptr);

        err = stack.push(4);
        assert(err != nullptr);
        std::cerr << "Failed to push element to stack: " << err << std::endl;

        err = stack.push(4);
        assert(err != nullptr);
        using custom_print::operator<<;
        std::cerr << "Failed to push element to stack:" << err << std::endl;

#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
        err = stack.push(4);
        assert(err != nullptr);
        std::cerr << "Failed to push element to stack:" << std::endl
                  << nlohmann::json(err).dump(2) << std::endl;
#endif
        std::cerr
                << "Wrapped error: "
                << errors::wrap(errors::wrap(errors::wrap(
                           errors::make_error<errors::common_error>("error"))))
                << std::endl;
}

// A namespace to make sure this custom print function not interfere with other code
namespace custom_print
{
// Custom print function for errors
std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        auto current_err = err.get();
        if (!current_err) {
                os << "success";
                return os;
        }

        for (; current_err != nullptr;
             current_err = current_err->cause().get()) {
                os << std::endl
                   << "[function " << current_err->location().function_name()
                   << " at "
                   << std::filesystem::path(current_err->location().file_name())
                                .filename()
                                .string()
                   << " " << current_err->location().line() << ":"
                   << current_err->location().column() << "] "
                   << current_err->what().value_or("");
        }
        return os;
}
}
}
