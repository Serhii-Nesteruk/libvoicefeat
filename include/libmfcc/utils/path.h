#pragma once

#include <filesystem>

#include "libmfcc/compat/source_location.h"

namespace libmfcc::utils {

    inline std::filesystem::path resolve_from_callsite(
        const std::filesystem::path& p,
        libmfcc::compat::source_location loc = libmfcc::compat::source_location::current())
    {
        if (p.is_absolute()) return std::filesystem::weakly_canonical(p);

        const std::filesystem::path call_file{loc.file_name()};
        auto call_dir = call_file.parent_path();

        const auto try_resolve = [&](const std::filesystem::path& base) -> std::filesystem::path
        {
            if (base.empty()) return {};
            std::error_code ec;
            const auto candidate = std::filesystem::weakly_canonical(base / p, ec);
            if (ec) return {};
            if (std::filesystem::exists(candidate)) return candidate;
            return {};
        };

        for (auto dir = call_dir; !dir.empty(); )
        {
            if (auto resolved = try_resolve(dir); !resolved.empty())
            {
                return resolved;
            }

            const auto parent = dir.parent_path();
            if (parent == dir)
            {
                break;
            }
            dir = parent;
        }

        if (auto resolved = try_resolve(std::filesystem::current_path()); !resolved.empty())
        {
            return resolved;
        }

        return std::filesystem::weakly_canonical(std::filesystem::current_path() / p);
    }

}
