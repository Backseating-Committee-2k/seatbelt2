#pragma once

#include "types.hpp"
#include "utils.hpp"
#include <cassert>
#include <functional>
#include <string_view>
#include <variant>
#include <vector>

struct SourceLocation final {
private:
    std::string_view m_filename;

private:
    std::u8string_view m_source_code;
    std::u8string_view m_lexeme;

public:
    SourceLocation(std::string_view filename, std::u8string_view source_code, std::u8string_view lexeme)
        : m_filename{ filename },
          m_source_code{ source_code },
          m_lexeme{ lexeme } { }

    [[nodiscard]] auto filename() const {
        return m_filename;
    }

    [[nodiscard]] auto source_code() const {
        return m_source_code;
    }

    [[nodiscard]] auto lexeme() const {
        return m_lexeme;
    }

    [[nodiscard]] auto ascii_lexeme() const {
        return utils::to_string_view(lexeme());
    }

    [[nodiscard]] usize line_number() const {
        assert(std::greater_equal{}(m_lexeme.data(), m_source_code.data()));
        assert(std::less_equal{}(m_lexeme.data() + m_lexeme.length(), m_source_code.data() + m_source_code.length()));
        auto result = usize{ 1 };
        for (auto current = m_source_code.data(); current != m_lexeme.data(); ++current) {
            if (*current == '\n') {
                ++result;
            }
        }
        return result;
    }

    [[nodiscard]] usize column_number() const {
        assert(std::greater_equal{}(m_lexeme.data(), m_source_code.data()));
        assert(std::less_equal{}(m_lexeme.data() + m_lexeme.length(), m_source_code.data() + m_source_code.length()));
        const auto up_until_lexeme = m_source_code.substr(
                0, static_cast<decltype(m_source_code)::size_type>(m_lexeme.data() - m_source_code.data())
        );
        const auto newline_index = up_until_lexeme.rfind('\n');
        const auto found = (newline_index != decltype(up_until_lexeme)::npos);
        const auto line_start = (found ? m_source_code.data() + newline_index + 1 : m_source_code.data());
        const auto line_until_lexeme = std::u8string_view{ line_start, m_lexeme.data() };

        return utils::utf8_width(line_until_lexeme).value() + 1;
    }
};
