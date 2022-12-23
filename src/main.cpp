#include "lexer.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <magic_enum.hpp>


int main() {
    const auto filename = std::string{ "test.bs" };
    const auto source_code = utils::read_text_file(filename).value();
    const auto tokens = tokenize(filename, source_code);
    if (tokens.has_value()) {
        for (const auto& token : *tokens) {
            fmt::print(
                    stderr, "{}:{}:{}: {} (\"{}\")\n", token.location.filename(), token.location.line_number(),
                    token.location.column_number(), magic_enum::enum_name(token.type),
                    utils::to_string_view(token.location.lexeme())
            );
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
