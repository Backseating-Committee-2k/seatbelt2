#pragma once

enum class ErrorCode {
    // lexer errors
    InvalidInput,
    MissingNewlineAtEndOfSourceCode,
    UnterminatedComment,
    // parser errors
    UnexpectedToken,
    // type errors
};
