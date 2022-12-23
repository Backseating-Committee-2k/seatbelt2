#pragma once

#include "error_codes.hpp"
#include "tokens.hpp"
#include "types.hpp"

using TokenVector = std::vector<Token>;

struct LexerError final {
    SourceLocation location;
    ErrorCode error_code;
};

[[nodiscard]] Result<TokenVector, LexerError> tokenize(std::string_view filename, std::u8string_view source_code);
