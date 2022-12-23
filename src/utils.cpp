#include "utils.hpp"
#include <cassert>
#include <gsl/gsl>
#include <utf8proc.h>

namespace utils {

    [[nodiscard]] Result<std::u8string, IoError> read_text_file(const std::filesystem::path& path) {
        // open file with file cursor at the end, binary to not have windows newlines replaced
        auto file = std::ifstream{ path, std::ios::in | std::ios::binary | std::ios::ate };
        if (not file) {
            return Error<IoError>{ IoError::CouldNotOpenFile };
        }

        // determine filesize
        const auto tellg_result = file.tellg();
        if (tellg_result == decltype(tellg_result){ -1 }) {
            return Error<IoError>{ IoError::UnableToDetermineFileSize };
        }
        const auto size = gsl::narrow_cast<usize>(tellg_result);

        // reset file cursor to the beginning
        file.seekg(0, std::ios::beg);

        // read contents of the file
        auto result = std::u8string{};
        result.resize(size + 1);
        try {
            file.read(reinterpret_cast<char*>(result.data()), gsl::narrow_cast<std::streamsize>(size));
        } catch (const std::ios::failure&) {
            return Error<IoError>{ IoError::UnableToReadFile };
        }

        // always end the string with a newline
        result.back() = '\n';

        return result;
    }

    [[nodiscard]] Result<usize, Utf8Error> utf8_width(const std::u8string_view string) {
        auto width = usize{ 0 };
        auto current = reinterpret_cast<const utf8proc_uint8_t*>(string.data());
        auto codepoint = utf8proc_int32_t{};

        while (current < reinterpret_cast<const utf8proc_uint8_t*>(string.data() + string.length())) {
            auto bytes_read = utf8proc_iterate(current, -1, &codepoint);
            if (codepoint == -1) {
                return Error<Utf8Error>{ Utf8Error::InvalidUtf8String };
            }
            assert(utf8proc_codepoint_valid(codepoint));
            width += static_cast<usize>(utf8proc_charwidth(codepoint));
            current += bytes_read;
        }
        if (current != reinterpret_cast<const utf8proc_uint8_t*>(string.data() + string.length())) {
            return Error<Utf8Error>{ Utf8Error::InvalidUtf8String };
        }
        return width;
    }

    [[nodiscard]] Result<std::u8string_view, Utf8Error> first_utf8_codepoint(const std::u8string_view string) {
        auto current = reinterpret_cast<const utf8proc_uint8_t*>(string.data());
        auto codepoint = utf8proc_int32_t{};
        auto bytes_read = utf8proc_iterate(current, -1, &codepoint);
        if (codepoint == -1) {
            return Error<Utf8Error>{ Utf8Error::InvalidUtf8Codepoint };
        }
        return string.substr(0, bytes_read);
    }

    [[nodiscard]] std::string_view to_string_view(const std::u8string_view string) {
        const auto view = std::string_view{
            reinterpret_cast<const char*>(string.data()),
            reinterpret_cast<const char*>(string.data()) + string.length(),
        };
        return view;
    }

} // namespace utils
