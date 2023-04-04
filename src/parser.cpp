#include "parser.hpp"
#include <exception>

using namespace parser_nodes;

enum class Throwing {
    Yes,
    No,
};

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
        if (result) {
            return std::move(*result);
        }
        return tl::unexpected{ std::move(m_errors) };
    }

private:
    [[nodiscard]] tl::optional<Program> program() {
        auto imports = import_statements();

        auto definitions = this->definitions();

        if (not is_end_of_input()) {
            error(ParserError{ current(), ErrorCode::UnexpectedToken });
        }

        const auto successful = m_errors.empty();

        if (successful) {
            return Program{ std::move(imports), std::move(definitions) };
        }
        return {};
    }

    [[nodiscard]] std::vector<ImportStatement> import_statements() {
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
        return imports;
    }

    [[nodiscard]] std::unique_ptr<Statement> definition(tl::optional<Token> export_token) {
        switch (current().type) {
            case TokenType::Function:
                return function(export_token);
            default:
                error(ParserError{ current(), ErrorCode::UnexpectedToken }, Throwing::No);
                synchronize();
                return {};
        }
    }

    [[nodiscard]] std::vector<std::unique_ptr<Statement>> definitions() {
        auto results = std::vector<std::unique_ptr<Statement>>{};
        while (not is_end_of_input()) {
            const auto export_token = try_consume(TokenType::Export);
            results.push_back(definition(export_token));
        }
        return results;
    }

    [[nodiscard]] static bool is_definition_keyword(const TokenType type) {
        using enum TokenType;
        return type == Function or type == Type or type == Struct or type == Import;
    }

    [[nodiscard]] std::unique_ptr<Statement> statement() {
        const auto export_token = try_consume(TokenType::Export);
        const auto is_definition = (export_token or is_definition_keyword(current().type));
        if (is_definition) {
            return definition(export_token);
        }
        // statement, but not a definition
        // todo
        assert(false and "not implemented");
        return {};
    }

    [[nodiscard]] std::vector<std::unique_ptr<Statement>> statements() {
        auto results = std::vector<std::unique_ptr<Statement>>{};
        while (not is_end_of_input() and not current_is(TokenType::RightCurlyBracket)) {
            results.push_back(statement());
        }
        return results;
    }

    [[nodiscard]] Block block() {
        const auto left_curly_bracket = consume(TokenType::LeftCurlyBracket);
        auto statements = this->statements();
        const auto right_curly_bracket = consume(TokenType::RightCurlyBracket);
        return Block{ left_curly_bracket, std::move(statements), right_curly_bracket };
    }

    template<TokenType... token_types>
    [[nodiscard]] auto consume() {
        if constexpr (sizeof...(token_types) == 1) {
            return consume(token_types...);
        } else if constexpr (sizeof...(token_types) > 1) {
            return std::tuple{ consume(token_types)... };
        } else {
            throw; // zero arguments are not allowed
        }
    }

    [[nodiscard]] std::unique_ptr<Statement> function([[maybe_unused]] tl::optional<Token> export_token) {
        assert(current().type == TokenType::Function);
        const auto [function_token, identifier_token] = consume<TokenType::Function, TokenType::Identifier>();
        auto type_parameter_list = this->type_parameter_list();
        auto parameter_list = this->type_parameter_list();
        auto return_type = this->return_type();
        return {};
        // todo
        // return std::make_unique<FunctionDefinition>(export_token, function_token, identifier_token, type_parameter_list, parameter_list, return_type, )
    }

    template<
            TokenType start_token,
            TokenType end_token,
            typename ElementParser,
            typename Elements = std::vector<std::remove_cvref_t<decltype(std::declval<ElementParser>()())>>,
            typename ReturnType = tl::optional<std::tuple<Token, Elements, Token>>>
    [[nodiscard]] ReturnType try_parse_list_with_optional_trailing_comma(ElementParser element_parser) {
        const auto start = try_consume(start_token);
        if (not start) {
            return {};
        }

        auto elements = Elements{};
        while (not is_end_of_input() and current().type != end_token) {
            elements.push_back(element_parser());
            const auto comma_token = try_consume(TokenType::Comma);
            if (not comma_token) {
                break;
            }
        }
        const auto end = consume<end_token>();
        return std::tuple{ *start, std::move(elements), end };
    }

    template<
            TokenType start_token,
            TokenType end_token,
            typename ElementParser,
            typename Elements = std::vector<std::remove_cvref_t<decltype(std::declval<ElementParser>()())>>,
            typename ReturnType = std::tuple<Token, Elements, Token>>
    [[nodiscard]] ReturnType parse_list_with_optional_trailing_comma(ElementParser element_parser) {
        auto result = try_parse_list_with_optional_trailing_comma<start_token, end_token>(element_parser);
        if (not result) {
            // try (again) to consume start token to force regular error reporting
            std::ignore = consume<start_token>(); // throws
            std::unreachable();
        }
        return std::move(*result);
    }

    [[nodiscard]] tl::optional<TypeParameterList> type_parameter_list() {
        auto list =
                try_parse_list_with_optional_trailing_comma<TokenType::LeftCurlyBracket, TokenType::RightCurlyBracket>(
                        [&]() { return consume<TokenType::Identifier>(); }
                );
        if (not list) {
            return {};
        }
        auto [left_curly_bracket, type_identifiers, right_curly_bracket] = *list;
        return TypeParameterList{ left_curly_bracket, std::move(type_identifiers), right_curly_bracket };
    }

    [[nodiscard]] ParameterList parameter_list() {
        auto [left_parenthesis, list, right_parenthesis] =
                parse_list_with_optional_trailing_comma<TokenType::LeftParenthesis, TokenType::RightParenthesis>([&]() {
                    const auto [identifier, _] = consume<TokenType::Identifier, TokenType::Colon>();
                    auto type_name = name();
                    return Parameter{ identifier, std::move(type_name) };
                });
        return ParameterList{ left_parenthesis, std::move(list), right_parenthesis };
    }

    [[nodiscard]] ReturnType return_type() {
        assert(current_is(TokenType::TildeArrow));
        const auto tilde_arrow = consume(TokenType::TildeArrow);
        auto type_name = name();
        return ReturnType{ tilde_arrow, std::move(type_name) };
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

    [[nodiscard]] bool current_is(const TokenType type) const {
        return current().type == type;
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

    void error(const ParserError error, const Throwing shouldThrow = Throwing::Yes) {
        m_errors.push_back(error);
        if (shouldThrow == Throwing::Yes) {
            throw ParserSynchronization{};
        }
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
