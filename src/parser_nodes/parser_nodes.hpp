// this file was automatically generated from the source file "parser_nodes.types"

#pragma once

#include "../error_codes.hpp"
#include "../lexer.hpp"
#include "../utils.hpp"
#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <vector>

namespace parser_nodes {

    // forward declarations
    struct Definition;
    struct FunctionDefinition;
    struct Expression;
    struct Block;

    struct Name final {
        explicit Name(std::vector<Token> tokens) : tokens{ std::move(tokens) } { }
        
        std::vector<Token> tokens;
        
        [[nodiscard]] std::string to_string() const {
        auto result = std::string{};
        for (const auto& token : tokens) {
        result = fmt::format("{}{}", result, token.location.ascii_lexeme());
        }
        return result;
        }
    };

    struct ReturnType final {
        ReturnType(Token tilde_arrow, Name type) : tilde_arrow{tilde_arrow}, type{std::move(type)} { }
        
        Token tilde_arrow;
        Name type;
    };

    struct TypeParameterList final {
        TypeParameterList(Token left_curly_brace,
        std::vector<Token> identifiers,
        Token right_curly_brace)
        : left_curly_brace{ left_curly_brace }
        , identifiers{ std::move(identifiers) }
        , right_curly_brace{ right_curly_brace }
        { }
        
        Token left_curly_brace;
        std::vector<Token> identifiers;
        Token right_curly_brace;
    };

    struct Parameter final {
        Parameter(Token identifier, Name type) : identifier{ identifier }, type{ std::move(type) } { }
        
        Token identifier;
        Name type;
    };

    struct ParameterList final {
        explicit ParameterList(std::vector<Parameter> parameters) : parameters{ std::move(parameters) } { }
        
        std::vector<Parameter> parameters;
    };

    struct ImportStatement final {
        ImportStatement(Token import_token, Name module_name, Token semicolon_token)
        : import_token{ import_token }, module_name{ std::move(module_name) }, semicolon_token{ semicolon_token }
        { }
        
        Token import_token;
        Name module_name;
        Token semicolon_token;
        
        [[nodiscard]] std::string to_string() const {
        return fmt::format("import {};", module_name.to_string());
        }
    };

    struct Program final {
        Program(std::vector<ImportStatement> imports, std::vector<std::unique_ptr<Definition>> definitions);
        
        std::vector<ImportStatement> imports;
        std::vector<std::unique_ptr<Definition>> definitions;
        
        [[nodiscard]] std::string to_string() const;
    };

    // Definition and its subtypes
    struct Definition {
    protected:
        Definition() = default;

    public:
        virtual ~Definition() = default;

        [[nodiscard]] virtual bool is_function_definition() const;

        [[nodiscard]] virtual tl::optional<const FunctionDefinition&> as_function_definition() const&;

        void as_function_definition() && = delete;

        [[nodiscard]] virtual std::string to_string() const = 0;

    };

    struct FunctionDefinition final : public Definition {
    private:
        Token m_function_keyword;
        Token m_identifier;
        tl::optional<TypeParameterList> m_type_parameters;
        Token m_left_parenthesis;
        tl::optional<ParameterList> m_parameters;
        Token m_right_parenthesis;
        tl::optional<ReturnType> m_return_type;
        std::unique_ptr<Block> m_body;
#ifdef DEBUG_BUILD
        bool m_function_keyword_is_valid = true;
        bool m_identifier_is_valid = true;
        bool m_type_parameters_is_valid = true;
        bool m_left_parenthesis_is_valid = true;
        bool m_parameters_is_valid = true;
        bool m_right_parenthesis_is_valid = true;
        bool m_return_type_is_valid = true;
        bool m_body_is_valid = true;
#endif

    public:
        explicit FunctionDefinition(Token function_keyword,
            Token identifier,
            tl::optional<TypeParameterList> type_parameters,
            Token left_parenthesis,
            tl::optional<ParameterList> parameters,
            Token right_parenthesis,
            tl::optional<ReturnType> return_type,
            std::unique_ptr<Block> body);

        FunctionDefinition(const FunctionDefinition&) = delete;
        FunctionDefinition(FunctionDefinition&& other) noexcept;
        FunctionDefinition& operator=(const FunctionDefinition&) = delete;
        FunctionDefinition& operator=(FunctionDefinition&& other) noexcept;

        [[nodiscard]] bool is_function_definition() const override;

        [[nodiscard]] tl::optional<const FunctionDefinition&> as_function_definition() const& override;

        [[nodiscard]] const Token& function_keyword() const&;
        void function_keyword() && = delete;

        [[nodiscard]] const Token& identifier() const&;
        void identifier() && = delete;

        [[nodiscard]] const tl::optional<TypeParameterList>& type_parameters() const&;
        void type_parameters() && = delete;
        [[nodiscard]] tl::optional<TypeParameterList> type_parameters_moved() &;
        void type_parameters_moved() && = delete;

        [[nodiscard]] const Token& left_parenthesis() const&;
        void left_parenthesis() && = delete;

        [[nodiscard]] const tl::optional<ParameterList>& parameters() const&;
        void parameters() && = delete;
        [[nodiscard]] tl::optional<ParameterList> parameters_moved() &;
        void parameters_moved() && = delete;

        [[nodiscard]] const Token& right_parenthesis() const&;
        void right_parenthesis() && = delete;

        [[nodiscard]] const tl::optional<ReturnType>& return_type() const&;
        void return_type() && = delete;
        [[nodiscard]] tl::optional<ReturnType> return_type_moved() &;
        void return_type_moved() && = delete;

        [[nodiscard]] const std::unique_ptr<Block>& body() const&;
        void body() && = delete;
        [[nodiscard]] std::unique_ptr<Block> body_moved() &;
        void body_moved() && = delete;

        [[nodiscard]] std::string to_string() const override;

#ifdef DEBUG_BUILD
    private:
        [[nodiscard]] bool all_members_valid() const;
#endif
    };

    // Expression and its subtypes
    struct Expression {
    protected:
        Expression() = default;

    public:
        virtual ~Expression() = default;

        [[nodiscard]] virtual bool is_block() const;

        [[nodiscard]] virtual tl::optional<const Block&> as_block() const&;

        void as_block() && = delete;

        [[nodiscard]] virtual std::string to_string() const = 0;

    };

    struct Block final : public Expression {
    private:
        Token m_left_curly_brace;
        Token m_right_curly_brace;
#ifdef DEBUG_BUILD
        bool m_left_curly_brace_is_valid = true;
        bool m_right_curly_brace_is_valid = true;
#endif

    public:
        explicit Block(Token left_curly_brace,
            Token right_curly_brace);

        Block(const Block&) = delete;
        Block(Block&& other) noexcept;
        Block& operator=(const Block&) = delete;
        Block& operator=(Block&& other) noexcept;

        [[nodiscard]] bool is_block() const override;

        [[nodiscard]] tl::optional<const Block&> as_block() const& override;

        [[nodiscard]] const Token& left_curly_brace() const&;
        void left_curly_brace() && = delete;

        [[nodiscard]] const Token& right_curly_brace() const&;
        void right_curly_brace() && = delete;

        [[nodiscard]] std::string to_string() const override;

#ifdef DEBUG_BUILD
    private:
        [[nodiscard]] bool all_members_valid() const;
#endif
    };

    inline Program::Program(std::vector<ImportStatement> imports, std::vector<std::unique_ptr<Definition>> definitions)
        : imports{ std::move(imports) },
          definitions{ std::move(definitions) } { }
    
    [[nodiscard]] inline std::string Program::to_string() const {
        auto result = std::string{};
        for (const auto& import_ : imports) {
            result += fmt::format("{}\n", import_.to_string());
        }
        if (not imports.empty()) {
            result += '\n';
        }
        for (const auto& definition : definitions) {
            result += fmt::format("{}\n", definition->to_string());
        }
        if (not definitions.empty()) {
            result += '\n';
        }
        return result;
    }

}
