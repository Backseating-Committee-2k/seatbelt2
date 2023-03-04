#pragma once

#include "error_codes.hpp"
#include "parser_nodes/parser_nodes.hpp"
#include "source_location.hpp"

struct ParserError final {
    ParserError(const Token& token, ErrorCode error_code, tl::optional<TokenType> expected = {})
        : ParserError{ token.location, error_code, expected } { }

    ParserError(SourceLocation location, ErrorCode error_code, tl::optional<TokenType> expected = {})
        : location{ location },
          error_code{ error_code },
          expected{ expected } { }

    SourceLocation location;
    ErrorCode error_code;
    tl::optional<TokenType> expected{};
};

using ParserErrors = std::vector<ParserError>;

[[nodiscard]] tl::expected<parser_nodes::Program, ParserErrors> parse(TokenVector&& tokens);
