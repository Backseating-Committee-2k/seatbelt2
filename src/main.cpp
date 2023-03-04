#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <magic_enum.hpp>


int main() {
    const auto filename = std::string{ "test.bs" };
    const auto source_code = utils::read_text_file(filename).value();
    auto tokens = tokenize(filename, source_code);
    if (tokens.has_value()) {
        for (const auto& token : *tokens) {
            fmt::print(
                    stderr, "{}:{}:{}: {} (\"{}\")\n", token.location.filename(), token.location.line_number(),
                    token.location.column_number(), magic_enum::enum_name(token.type),
                    utils::to_string_view(token.location.lexeme())
            );
        }
        const auto program = parse(std::move(*tokens));
        if (program.has_value()) {
            fmt::print(stderr, "{}", program->to_string());
        } else {
            fmt::print(stderr, "parser error:\n");
            for (const auto& error : program.error()) {
                fmt::print(
                        stderr, "{}:{}:{}: {} (\"{}\")\n", error.location.filename(), error.location.line_number(),
                        error.location.column_number(), magic_enum::enum_name(error.error_code),
                        utils::to_string_view(error.location.lexeme())
                );
            }
        }
    } else {
        fmt::print(
                stderr, "{}:{}:{}: {} (\"{}\")", tokens.error().location.filename(),
                tokens.error().location.line_number(), tokens.error().location.column_number(),
                magic_enum::enum_name(tokens.error().error_code),
                utils::to_string_view(tokens.error().location.lexeme())
        );
    }
}
