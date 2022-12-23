#pragma once

#include "types.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace utils {

    enum class IoError {
        CouldNotOpenFile,
        UnableToReadFile,
        UnableToDetermineFileSize,
    };

    [[nodiscard]] Result<std::u8string, IoError> read_text_file(const std::filesystem::path& path);

    enum class Utf8Error {
        InvalidUtf8String,
        InvalidUtf8Codepoint,
    };

    [[nodiscard]] Result<usize, Utf8Error> utf8_width(std::u8string_view string);
    [[nodiscard]] Result<std::u8string_view, Utf8Error> first_utf8_codepoint(std::u8string_view string);

    [[nodiscard]] std::string_view to_string_view(std::u8string_view string);

} // namespace utils
