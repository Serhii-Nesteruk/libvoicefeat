#pragma once
#include <string_view>

namespace libvoicefeat::compat {
    struct source_location {
        const char* file_name_;
        const char* function_name_;
        int line_;

        static constexpr source_location current(
            const char* file = __builtin_FILE(),
            const char* function = __builtin_FUNCTION(),
            int line = __builtin_LINE()) noexcept
        {
            return {file, function, line};
        }

        [[nodiscard]] constexpr const char* file_name() const noexcept { return file_name_; }
        [[nodiscard]] constexpr const char* function_name() const noexcept { return function_name_; }
        [[nodiscard]] constexpr int line() const noexcept { return line_; }
    };
}
