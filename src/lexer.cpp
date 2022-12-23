#include "lexer.hpp"
#include "fmt/core.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <ctre-unicode.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/core.hpp>

struct LexerState final {
private:
    std::string_view m_filename;
    std::u8string_view m_source_code;
    usize m_index{ 0 };
    usize m_line_number{ 1 };

public:
    LexerState(const std::string_view filename, const std::u8string_view sourceCode)
        : m_filename{ filename },
          m_source_code{ sourceCode } { }

    [[nodiscard]] char8_t current() const {
        return m_source_code.at(m_index);
    }

    [[nodiscard]] Optional<char8_t> peek(const usize offset = 1) const {
        if (m_index + offset >= m_source_code.length()) {
            return {};
        }
        return m_source_code[m_index + offset];
    }

    void advance_bytes(const usize amount = 1) {
        for (usize i = 0; i < amount; ++i) {
            if (current() == '\n') {
                ++m_line_number;
            }
            ++m_index;
        }
    }

    [[nodiscard]] bool is_end_of_file() const {
        return m_index >= m_source_code.length();
    }

    [[nodiscard]] auto filename() const {
        return m_filename;
    }

    [[nodiscard]] auto source_code() const {
        return m_source_code;
    }

    [[nodiscard]] auto line_number() const {
        return m_line_number;
    }

    [[nodiscard]] auto source_location_from_bytes(const usize num_bytes = 1) const {
        assert(not is_end_of_file());
        const auto lexeme = std::u8string_view{ &m_source_code.at(m_index), &m_source_code.at(m_index) + num_bytes };
        return SourceLocation{ m_filename, m_source_code, lexeme };
    }

    [[nodiscard]] auto source_location_from_codepoints(const usize num_codepoints = 1) const {
        assert(not is_end_of_file());
        auto current_index = m_index;
        for (usize i = 0; i < num_codepoints; ++i) {
            const auto remaining_source = m_source_code.substr(m_index);
            const auto codepoint = utils::first_utf8_codepoint(remaining_source).value();
            current_index += codepoint.length();
        }
        const auto lexeme = std::u8string_view{ &m_source_code.at(m_index), &m_source_code.at(current_index) };
        return SourceLocation{ m_filename, m_source_code, lexeme };
    }


    [[nodiscard]] auto view_from_current() const {
        assert(not is_end_of_file());
        return m_source_code.substr(m_index);
    }
};

[[nodiscard]] constexpr std::array<bool, 127> source_character_lookup_table() {
    auto result = std::array<bool, 127>{};
    result['\t'] = true;
    result['\n'] = true;
    result['\r'] = true;
    for (auto c = ' '; c <= '~'; ++c) {
        result[c] = true;
    }
    return result;
}

[[nodiscard]] bool is_in_source_character_set(const char c) {
    static constexpr auto lookup_table = source_character_lookup_table();
    if (c < 0 or static_cast<usize>(c) >= lookup_table.size()) {
        return false;
    }
    return lookup_table[static_cast<usize>(static_cast<unsigned char>(c))];
}

[[nodiscard]] Result<TokenVector, LexerError>
tokenize(const std::string_view filename, const std::u8string_view source_code) {
    assert(not source_code.empty() and source_code.back() == '\n');
    using namespace std::string_view_literals;

    static constexpr auto non_keyword_tokens = std::array{
        std::pair{u8"->"sv,                 TokenType::Arrow},
        std::pair{u8"~>"sv,            TokenType::TildeArrow},
        std::pair{u8"!="sv, TokenType::ExclamationMarkEquals},
        std::pair{u8"::"sv,           TokenType::DoubleColon},
        std::pair{u8">="sv,       TokenType::GreaterOrEquals},
        std::pair{u8"<="sv,          TokenType::LessOrEquals},
        std::pair{u8"=="sv,          TokenType::EqualsEquals},
        std::pair{ u8"-"sv,                 TokenType::Minus},
        std::pair{ u8":"sv,                 TokenType::Colon},
        std::pair{ u8","sv,                 TokenType::Comma},
        std::pair{ u8";"sv,             TokenType::Semicolon},
        std::pair{ u8"+"sv,                  TokenType::Plus},
        std::pair{ u8"-"sv,                 TokenType::Minus},
        std::pair{ u8"*"sv,              TokenType::Asterisk},
        std::pair{ u8"/"sv,          TokenType::ForwardSlash},
        std::pair{ u8"("sv,       TokenType::LeftParenthesis},
        std::pair{ u8")"sv,      TokenType::RightParenthesis},
        std::pair{ u8"{"sv,      TokenType::LeftCurlyBracket},
        std::pair{ u8"}"sv,     TokenType::RightCurlyBracket},
        std::pair{ u8"["sv,     TokenType::LeftSquareBracket},
        std::pair{ u8"]"sv,    TokenType::RightSquareBracket},
        std::pair{ u8"="sv,                TokenType::Equals},
        std::pair{ u8"."sv,                   TokenType::Dot},
        std::pair{ u8"!"sv,       TokenType::ExclamationMark},
        std::pair{ u8">"sv,           TokenType::GreaterThan},
        std::pair{ u8"<"sv,              TokenType::LessThan},
        std::pair{ u8"@"sv,                    TokenType::At},
    };

    static constexpr auto keywords = std::array{
        std::pair{u8"dump_registers"sv,       TokenType::DumpRegisters},
        std::pair{      u8"function"sv,            TokenType::Function},
        std::pair{           u8"mod"sv,                 TokenType::Mod},
        std::pair{           u8"let"sv,                 TokenType::Let},
        std::pair{          u8"true"sv,         TokenType::BoolLiteral},
        std::pair{         u8"false"sv,         TokenType::BoolLiteral},
        std::pair{           u8"bsm"sv,      TokenType::InlineBssembly},
        std::pair{        u8"import"sv,              TokenType::Import},
        std::pair{     u8"namespace"sv,           TokenType::Namespace},
        std::pair{           u8"and"sv,                 TokenType::And},
        std::pair{            u8"or"sv,                  TokenType::Or},
        std::pair{           u8"not"sv,                 TokenType::Not},
        std::pair{           u8"xor"sv,                 TokenType::Xor},
        std::pair{            u8"if"sv,                  TokenType::If},
        std::pair{          u8"else"sv,                TokenType::Else},
        std::pair{          u8"loop"sv,                TokenType::Loop},
        std::pair{         u8"break"sv,               TokenType::Break},
        std::pair{      u8"continue"sv,            TokenType::Continue},
        std::pair{         u8"while"sv,               TokenType::While},
        std::pair{            u8"do"sv,                  TokenType::Do},
        std::pair{           u8"for"sv,                 TokenType::For},
        std::pair{       u8"mutable"sv,             TokenType::Mutable},
        std::pair{         u8"const"sv,               TokenType::Const},
        std::pair{        u8"return"sv,              TokenType::Return},
        std::pair{         u8"label"sv,               TokenType::Label},
        std::pair{          u8"goto"sv,                TokenType::Goto},
        std::pair{       u8"nothing"sv,      TokenType::NothingLiteral},
        std::pair{      u8"Function"sv, TokenType::CapitalizedFunction},
        std::pair{        u8"export"sv,              TokenType::Export},
        std::pair{     u8"type_size"sv,            TokenType::TypeSize},
        std::pair{    u8"value_size"sv,           TokenType::ValueSize},
        std::pair{          u8"type"sv,                TokenType::Type},
        std::pair{        u8"struct"sv,              TokenType::Struct},
        std::pair{    u8"restricted"sv,          TokenType::Restricted},
    };
    static constexpr char char_pattern[] = R"('(\\'|[ -\[\]-~]|\\[n\\tnvfr0])')";
    static constexpr char integer_pattern[] =
            "(0o([0-7]+_?)+)|(0x([\\dA-Fa-to_string_view]+_?)+)|(0b([01]+_?)+)|(\\d+_?)+";
    static constexpr char identifier_pattern[] =
            R"((\p{XID_Start}|[^\u0000-\u007F])(\p{XID_Continue}|[^\u0000-\u007F])*)";

    auto tokens = TokenVector{};

    auto state = LexerState{ filename, source_code };

    const auto push_token = [&tokens, &state](const TokenType token_type, const usize num_lexeme_bytes = 1) {
        tokens.emplace_back(state.source_location_from_bytes(num_lexeme_bytes), token_type);
        state.advance_bytes(num_lexeme_bytes);
    };

    while (not state.is_end_of_file()) {
        const auto codepoint = utils::first_utf8_codepoint(state.view_from_current());
        if (not codepoint.has_value()) {
            return Error<LexerError>{
                LexerError{state.source_location_from_bytes(), ErrorCode::InvalidInput}
            };
        }

        if (std::isspace(static_cast<unsigned char>(state.current()))) {
            state.advance_bytes();
            continue;
        }

        // one-line comments
        if (const auto next = state.peek(); next and state.current() == '/' and *next == '/') {
            state.advance_bytes(2);
            while (not state.is_end_of_file() and state.current() != '\n') {
                state.advance_bytes();
            }
            continue;
        }

        // multi-line comments
        if (auto next = state.peek(); next and state.current() == '/' and *next == '*') {
            auto starting_source_location = state.source_location_from_bytes(2);
            state.advance_bytes(2);
            auto nesting_level = usize{ 1 };

            while (not state.is_end_of_file()) {
                if (next = state.peek(); next and state.current() == '/' and *next == '*') {
                    state.advance_bytes(2);
                    ++nesting_level;
                    continue;
                }

                if (next = state.peek(); next and state.current() == '*' and *next == '/') {
                    state.advance_bytes(2);
                    --nesting_level;
                    if (nesting_level == 0) {
                        break;
                    }
                    continue;
                }

                state.advance_bytes();
            }
            if (state.is_end_of_file()) {
                return Error<LexerError>{
                    LexerError{starting_source_location, ErrorCode::UnterminatedComment}
                };
            }
            continue;
        }

        const auto non_keyword_iterator = ranges::find_if(non_keyword_tokens, [&state](const auto& pair) {
            const auto& lexeme = pair.first;
            return state.view_from_current().starts_with(lexeme);
        });
        const auto found_keyword = (non_keyword_iterator != non_keyword_tokens.cend());
        if (found_keyword) {
            const auto& [lexeme, token_type] = *non_keyword_iterator;
            push_token(token_type, lexeme.length());
            continue;
        }

        const auto remaining_source = state.view_from_current();

        if (const auto char_literal_result = ctre::starts_with<char_pattern>(remaining_source)) {
            const auto length = char_literal_result.view().length();
            push_token(TokenType::CharLiteral, length);
            continue;
        }

        if (const auto integer_literal_result = ctre::starts_with<integer_pattern>(remaining_source)) {
            const auto length = integer_literal_result.view().length();
            push_token(TokenType::U32Literal, length);
            continue;
        }

        if (const auto identifier_result = ctre::starts_with<identifier_pattern>(remaining_source)) {
            const auto keyword_iterator =
                    ranges::find_if(keywords, [&](const auto& pair) { return pair.first == identifier_result.view(); });
            const auto is_keyword = (keyword_iterator != keywords.cend());
            if (is_keyword) {
                const auto& [lexeme, token_type] = *keyword_iterator;
                push_token(token_type, lexeme.length());
            } else {
                const auto length = identifier_result.view().length();
                push_token(TokenType::Identifier, length);
            }
            continue;
        }

        return Error<LexerError>{
            LexerError{state.source_location_from_codepoints(), ErrorCode::InvalidInput}
        };
    }
    tokens.emplace_back(
            SourceLocation{ state.filename(), state.source_code(),
                            state.source_code().substr(state.source_code().length() - 1) },
            TokenType::EndOfFile
    );

    return std::move(tokens);
}
