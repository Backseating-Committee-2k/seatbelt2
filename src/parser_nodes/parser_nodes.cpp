// this file was automatically generated from the source file "parser_nodes.types"

#include "../../src/parser_nodes/parser_nodes.hpp"

namespace parser_nodes {

    // definitions for Definition
    [[nodiscard]] bool Definition::is_function_definition() const {
        return false;
    }

    [[nodiscard]] tl::optional<const FunctionDefinition&> Definition::as_function_definition() const& {
        return {};
    }

    // definitions for FunctionDefinition
    FunctionDefinition::FunctionDefinition(Token function_keyword,
            Token identifier,
            tl::optional<TypeParameterList> type_parameters,
            Token left_parenthesis,
            tl::optional<ParameterList> parameters,
            Token right_parenthesis,
            tl::optional<ReturnType> return_type,
            std::unique_ptr<Block> body)
        : m_function_keyword{ function_keyword }
        , m_identifier{ identifier }
        , m_type_parameters{ std::move(type_parameters) }
        , m_left_parenthesis{ left_parenthesis }
        , m_parameters{ std::move(parameters) }
        , m_right_parenthesis{ right_parenthesis }
        , m_return_type{ std::move(return_type) }
        , m_body{ std::move(body) }
    { }

    FunctionDefinition::FunctionDefinition(FunctionDefinition&& other) noexcept
        : m_function_keyword{ other.m_function_keyword }
        , m_identifier{ other.m_identifier }
        , m_type_parameters{ std::move(other.m_type_parameters) }
        , m_left_parenthesis{ other.m_left_parenthesis }
        , m_parameters{ std::move(other.m_parameters) }
        , m_right_parenthesis{ other.m_right_parenthesis }
        , m_return_type{ std::move(other.m_return_type) }
        , m_body{ std::move(other.m_body) }
    {
#ifdef DEBUG_BUILD
        if (this == std::addressof(other)) {
            return;
        }
        assert(other.all_members_valid() and "move out of partially moved-from value");
        m_function_keyword_is_valid = true;
        m_identifier_is_valid = true;
        m_type_parameters_is_valid = true;
        m_left_parenthesis_is_valid = true;
        m_parameters_is_valid = true;
        m_right_parenthesis_is_valid = true;
        m_return_type_is_valid = true;
        m_body_is_valid = true;
#endif
    }

    FunctionDefinition& FunctionDefinition::operator=(FunctionDefinition&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }
#ifdef DEBUG_BUILD
        assert(other.all_members_valid() and "move out of partially moved-from value");
#endif
        m_function_keyword = other.m_function_keyword;
        m_identifier = other.m_identifier;
        m_type_parameters = std::move(other.m_type_parameters);
        m_left_parenthesis = other.m_left_parenthesis;
        m_parameters = std::move(other.m_parameters);
        m_right_parenthesis = other.m_right_parenthesis;
        m_return_type = std::move(other.m_return_type);
        m_body = std::move(other.m_body);
#ifdef DEBUG_BUILD
        m_function_keyword_is_valid = true;
        m_identifier_is_valid = true;
        m_type_parameters_is_valid = true;
        m_left_parenthesis_is_valid = true;
        m_parameters_is_valid = true;
        m_right_parenthesis_is_valid = true;
        m_return_type_is_valid = true;
        m_body_is_valid = true;
#endif
        return *this;
    }

    [[nodiscard]] bool FunctionDefinition::is_function_definition() const {
        return true;
    }

    [[nodiscard]] tl::optional<const FunctionDefinition&> FunctionDefinition::as_function_definition() const& {
        return *this;
    }

    [[nodiscard]] const Token& FunctionDefinition::function_keyword() const& {
#ifdef DEBUG_BUILD
        assert(m_function_keyword_is_valid and "accessing a moved-from value");
#endif
        return m_function_keyword;
    }

    [[nodiscard]] const Token& FunctionDefinition::identifier() const& {
#ifdef DEBUG_BUILD
        assert(m_identifier_is_valid and "accessing a moved-from value");
#endif
        return m_identifier;
    }

    [[nodiscard]] const tl::optional<TypeParameterList>& FunctionDefinition::type_parameters() const& {
#ifdef DEBUG_BUILD
        assert(m_type_parameters_is_valid and "accessing a moved-from value");
#endif
        return m_type_parameters;
    }

    [[nodiscard]] tl::optional<TypeParameterList> FunctionDefinition::type_parameters_moved() & {
#ifdef DEBUG_BUILD
        assert(m_type_parameters_is_valid and "trying to move out of a moved-from value");
        m_type_parameters_is_valid = false;
#endif
        return std::move(m_type_parameters);
    }

    [[nodiscard]] const Token& FunctionDefinition::left_parenthesis() const& {
#ifdef DEBUG_BUILD
        assert(m_left_parenthesis_is_valid and "accessing a moved-from value");
#endif
        return m_left_parenthesis;
    }

    [[nodiscard]] const tl::optional<ParameterList>& FunctionDefinition::parameters() const& {
#ifdef DEBUG_BUILD
        assert(m_parameters_is_valid and "accessing a moved-from value");
#endif
        return m_parameters;
    }

    [[nodiscard]] tl::optional<ParameterList> FunctionDefinition::parameters_moved() & {
#ifdef DEBUG_BUILD
        assert(m_parameters_is_valid and "trying to move out of a moved-from value");
        m_parameters_is_valid = false;
#endif
        return std::move(m_parameters);
    }

    [[nodiscard]] const Token& FunctionDefinition::right_parenthesis() const& {
#ifdef DEBUG_BUILD
        assert(m_right_parenthesis_is_valid and "accessing a moved-from value");
#endif
        return m_right_parenthesis;
    }

    [[nodiscard]] const tl::optional<ReturnType>& FunctionDefinition::return_type() const& {
#ifdef DEBUG_BUILD
        assert(m_return_type_is_valid and "accessing a moved-from value");
#endif
        return m_return_type;
    }

    [[nodiscard]] tl::optional<ReturnType> FunctionDefinition::return_type_moved() & {
#ifdef DEBUG_BUILD
        assert(m_return_type_is_valid and "trying to move out of a moved-from value");
        m_return_type_is_valid = false;
#endif
        return std::move(m_return_type);
    }

    [[nodiscard]] const std::unique_ptr<Block>& FunctionDefinition::body() const& {
#ifdef DEBUG_BUILD
        assert(m_body_is_valid and "accessing a moved-from value");
#endif
        return m_body;
    }

    [[nodiscard]] std::unique_ptr<Block> FunctionDefinition::body_moved() & {
#ifdef DEBUG_BUILD
        assert(m_body_is_valid and "trying to move out of a moved-from value");
        m_body_is_valid = false;
#endif
        return std::move(m_body);
    }

    [[nodiscard]] std::string FunctionDefinition::to_string() const {
        return "todo";
    }

#ifdef DEBUG_BUILD
    [[nodiscard]] bool FunctionDefinition::all_members_valid() const {
        return m_function_keyword_is_valid
            and m_identifier_is_valid
            and m_type_parameters_is_valid
            and m_left_parenthesis_is_valid
            and m_parameters_is_valid
            and m_right_parenthesis_is_valid
            and m_return_type_is_valid
            and m_body_is_valid;
    }
#endif

    // definitions for Expression
    [[nodiscard]] bool Expression::is_block() const {
        return false;
    }

    [[nodiscard]] tl::optional<const Block&> Expression::as_block() const& {
        return {};
    }

    // definitions for Block
    Block::Block(Token left_curly_brace,
            Token right_curly_brace)
        : m_left_curly_brace{ left_curly_brace }
        , m_right_curly_brace{ right_curly_brace }
    { }

    Block::Block(Block&& other) noexcept
        : m_left_curly_brace{ other.m_left_curly_brace }
        , m_right_curly_brace{ other.m_right_curly_brace }
    {
#ifdef DEBUG_BUILD
        if (this == std::addressof(other)) {
            return;
        }
        assert(other.all_members_valid() and "move out of partially moved-from value");
        m_left_curly_brace_is_valid = true;
        m_right_curly_brace_is_valid = true;
#endif
    }

    Block& Block::operator=(Block&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }
#ifdef DEBUG_BUILD
        assert(other.all_members_valid() and "move out of partially moved-from value");
#endif
        m_left_curly_brace = other.m_left_curly_brace;
        m_right_curly_brace = other.m_right_curly_brace;
#ifdef DEBUG_BUILD
        m_left_curly_brace_is_valid = true;
        m_right_curly_brace_is_valid = true;
#endif
        return *this;
    }

    [[nodiscard]] bool Block::is_block() const {
        return true;
    }

    [[nodiscard]] tl::optional<const Block&> Block::as_block() const& {
        return *this;
    }

    [[nodiscard]] const Token& Block::left_curly_brace() const& {
#ifdef DEBUG_BUILD
        assert(m_left_curly_brace_is_valid and "accessing a moved-from value");
#endif
        return m_left_curly_brace;
    }

    [[nodiscard]] const Token& Block::right_curly_brace() const& {
#ifdef DEBUG_BUILD
        assert(m_right_curly_brace_is_valid and "accessing a moved-from value");
#endif
        return m_right_curly_brace;
    }

    [[nodiscard]] std::string Block::to_string() const {
        return "{}";
    }

#ifdef DEBUG_BUILD
    [[nodiscard]] bool Block::all_members_valid() const {
        return m_left_curly_brace_is_valid
            and m_right_curly_brace_is_valid;
    }
#endif

}
