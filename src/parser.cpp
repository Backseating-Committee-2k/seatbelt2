#include "parser.hpp"
#include <exception>

using namespace parser_nodes;

struct ParserSynchronization : public std::exception { };

struct ParserState {
private:
    TokenVector m_tokens;
    usize m_index{ 0 };
    ParserErrors m_errors{};

public:
    explicit ParserState(TokenVector&& tokens) : m_tokens{ std::move(tokens) } { }

    [[nodiscard]] tl::expected<Program, ParserErrors> parse() {
        auto result = program();
        if (result.has_value()) {
            return std::move(*result);
        }
        return tl::unexpected{ std::move(m_errors) };
    }

private:
    [[nodiscard]] tl::optional<Program> program() {
        auto imports = std::vector<ImportStatement>{};
        while (const auto import_token = try_consume(TokenType::Import)) {
            try {
                auto module_name = name();
                const auto semicolon_token = consume(TokenType::Semicolon);
                imports.emplace_back(*import_token, std::move(module_name), semicolon_token);
            } catch ([[maybe_unused]] const ParserSynchronization& sync) {
                synchronize();
            }
        }
        if (not is_end_of_input()) {
            error(ParserError{ current(), ErrorCode::UnexpectedToken });
        }

        auto definitions = std::vector<std::unique_ptr<Definition>>{};

        const auto successful = m_errors.empty();

        if (successful) {
            return Program{ std::move(imports), std::move(definitions) };
        }
        return {};
    }

    void synchronize() {
        while (not is_end_of_input() and current().type != TokenType::Semicolon) {
            advance();
        }
        if (not is_end_of_input()) {
            assert(current().type == TokenType::Semicolon);
            advance();
        }
    }

    [[nodiscard]] Name name() {
        auto tokens = std::vector<Token>{};
        tokens.push_back(consume(TokenType::Identifier));
        while (const auto double_colon_token = try_consume(TokenType::DoubleColon)) {
            tokens.push_back(*double_colon_token);
            tokens.push_back(consume(TokenType::Identifier));
        }
        return Name{ std::move(tokens) };
    }

    [[nodiscard]] const Token& current() const {
        return m_tokens.at(m_index);
    }

    [[nodiscard]] const Token& peek() const {
        return m_tokens.at(m_index + 1);
    }

    [[nodiscard]] tl::optional<const Token&> try_consume(const TokenType type) {
        if (current().type != type) {
            return {};
        }
        return m_tokens.at(m_index++);
    }

    [[nodiscard]] const Token& consume(const TokenType type) {
        if (current().type != type) {
            error(ParserError{ current(), ErrorCode::UnexpectedToken, type });
        }
        return m_tokens.at(m_index++);
    }

    void error(ParserError error) {
        m_errors.push_back(error);
        throw ParserSynchronization{};
    }

    void advance(const usize amount = 1) {
        m_index += amount;
    }

    [[nodiscard]] bool is_end_of_input() const {
        return m_index >= m_tokens.size() or current().type == TokenType::EndOfFile;
    }
};

[[nodiscard]] tl::expected<Program, ParserErrors> parse(TokenVector&& tokens) {
    auto parser_state = ParserState{ std::move(tokens) };
    return parser_state.parse();
}
