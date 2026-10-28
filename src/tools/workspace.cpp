// src/tools/workspace.cpp - Workspace::resolve (spec 4.4).

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    try {
        if (user_path.empty()) {
            *reason = "empty path";
            return false;
        }
        if (user_path.find('\0') != std::string_view::npos) {
            *reason = "path contains a NUL byte";
            return false;
        }

        fs::path p{std::string(user_path)};
        if (p.is_absolute() || p.has_root_path()) {
            *reason = "absolute paths are not allowed: " + std::string(user_path);
            return false;
        }

        fs::path n = p.lexically_normal();
        if (n.empty() || *n.begin() == "..") {
            *reason = "path escapes the workspace: " + std::string(user_path);
            return false;
        }

        if (n == ".")
            *out = root();
        else
            *out = root() / n;
        return true;
    } catch (...) {
        *reason = "invalid path";
        return false;
    }
}
