from parser import GeneratorDescription, Member
import re


def emit(input_filename: str, generator_description: GeneratorDescription, base_filename: str, namespace: str) -> None:
    header_filename = f"{base_filename}.hpp"
    source_filename = f"{base_filename}.cpp"
    save_file(generate_header_file(input_filename, generator_description, namespace), header_filename)
    save_file(generate_source_file(input_filename, generator_description, header_filename, namespace), source_filename)


def save_file(contents: str, filename: str) -> None:
    with open(filename, "w") as file:
        file.write(contents)


def file_was_generated_info(input_filename: str) -> str:
    return f'// this file was automatically generated from the source file "{input_filename}"\n\n'


def generate_header_file(input_filename: str, generator_description: GeneratorDescription, namespace: str) -> str:
    result = file_was_generated_info(input_filename)
    result += f"#pragma once\n\n{generator_description.includes}\n\nnamespace {namespace} {{\n\n"
    result += generate_type_definitions(generator_description)
    result += forward_declarations(generator_description)
    result += "\n"
    result += abstract_type_declarations(generator_description)

    result += "}\n"
    return result


def generate_source_file(input_filename: str, generator_description: GeneratorDescription, header_filename: str,
                         namespace: str) -> str:
    result = file_was_generated_info(input_filename)
    result += f'#include "{header_filename}"\n\nnamespace {namespace} {{\n\n'
    result += abstract_type_definitions(generator_description)
    result += "}\n"
    return result


def generate_type_definitions(generator_description: GeneratorDescription) -> str:
    result = ""
    for name, definition in generator_description.type_definitions:
        members = [line.strip() for line in definition.split("\n")]
        members_string = "        " + "\n        ".join(members)
        result += f"    struct {name} final {{\n{members_string}\n    }};\n\n"
    return result


def forward_declarations(generator_description: GeneratorDescription) -> str:
    result = "    // forward declarations\n"
    for abstract_type in generator_description.abstract_types.values():
        for type_ in abstract_type.sub_types.keys():
            result += f"    struct {type_};\n"
    return result


def parameter_list(members: list[Member]) -> str:
    return f",\n            ".join(f"{member.type_} {member.name}" for member in members)


def initializer_list(members: list[Member], from_other: bool) -> str:
    def parameter_value(member: Member) -> str:
        member_name = f"other.m_{member.name}" if from_other else member.name
        if member.by_move:
            return f"std::move({member_name})"
        return member_name

    return f"\n        , ".join(f"m_{member.name}{{ {parameter_value(member)} }}" for member in members)


def abstract_type_declarations(generator_description: GeneratorDescription) -> str:
    result = ""
    # outer loop iterates over the abstract parent class
    for abstract_type_name, abstract_type in generator_description.abstract_types.items():
        result += f"    // {abstract_type_name} and its subtypes\n"
        result += f"    struct {abstract_type_name} {{\n"
        result += f"    protected:\n        {abstract_type_name}() = default;\n\n    public:\n"
        result += f"        virtual ~{abstract_type_name}() = default;\n\n"

        # member functions of the abstract parent class
        for sub_type_name, sub_type in abstract_type.sub_types.items():
            result += f"        [[nodiscard]] virtual bool is_{to_snake_case(sub_type_name)}() const;\n\n"
            result += f"        [[nodiscard]] virtual tl::optional<const {sub_type_name}&> as_{to_snake_case(sub_type_name)}() const&;\n\n"
            result += f"        void as_{to_snake_case(sub_type_name)}() && = delete;\n\n"

        for function in abstract_type.pure_virtual_functions:
            result += f"        [[nodiscard]] virtual {function.return_type} {function.name}() const = 0;\n\n"

        result += "    };\n\n"

        # this loop iterates over the subclasses
        for sub_type_name, sub_type in abstract_type.sub_types.items():
            result += f"    struct {sub_type_name} final : public {abstract_type_name} {{\n"
            result += f"    private:\n"

            # member definitions
            for member in sub_type.members:
                result += f"        {member.type_} m_{member.name};\n"

            result += f"#ifdef DEBUG_BUILD\n"
            for member in sub_type.members:
                result += f"        bool m_{member.name}_is_valid = true;\n"
            result += f"#endif\n\n"

            result += f"    public:\n"

            # constructor
            result += f"        explicit {sub_type_name}({parameter_list(sub_type.members)});\n\n"

            # copy constructor
            result += f"        {sub_type_name}(const {sub_type_name}&) = delete;\n"

            # move constructor
            result += f"        {sub_type_name}({sub_type_name}&& other) noexcept;\n"

            # copy assignment
            result += f"        {sub_type_name}& operator=(const {sub_type_name}&) = delete;\n"

            # move assignment
            result += f"        {sub_type_name}& operator=({sub_type_name}&& other) noexcept;\n\n"

            # polymorphic type accessors
            result += f"        [[nodiscard]] bool is_{to_snake_case(sub_type_name)}() const override;\n\n"
            result += f"        [[nodiscard]] tl::optional<const {sub_type_name}&> as_{to_snake_case(sub_type_name)}() const& override;\n\n"

            # getter member functions
            for member in sub_type.members:
                result += f"        [[nodiscard]] const {member.type_}& {member.name}() const&;\n"
                result += f"        void {member.name}() && = delete;\n"
                if member.by_move:
                    result += f"        [[nodiscard]] {member.type_} {member.name}_moved() &;\n"
                    result += f"        void {member.name}_moved() && = delete;\n"
                result += f"\n"

            # implementations
            for implementation in sub_type.implementations:
                return_type = [virtual_function for virtual_function in sub_type.pure_virtual_functions if
                               virtual_function.name == implementation.name][0].return_type
                result += f"        [[nodiscard]] {return_type} {implementation.name}() const override;\n\n"

            result += f"#ifdef DEBUG_BUILD\n"
            result += f"    private:\n"
            result += f"        [[nodiscard]] bool all_members_valid() const;\n"
            result += f"#endif\n"

            result += f"    }};\n\n"
    return result


def abstract_type_definitions(generator_description: GeneratorDescription) -> str:
    result = ""
    for abstract_type_name, abstract_type in generator_description.abstract_types.items():
        result += f"    // definitions for {abstract_type_name}\n"

        # definitions for abstract parent class
        for sub_type_name, sub_type in abstract_type.sub_types.items():
            result += f"    [[nodiscard]] bool {abstract_type_name}::is_{to_snake_case(sub_type_name)}() const {{\n"
            result += f"        return false;\n"
            result += f"    }}\n\n"

            result += f"    [[nodiscard]] tl::optional<const {sub_type_name}&> {abstract_type_name}::as_{to_snake_case(sub_type_name)}() const& {{\n"
            result += f"        return {{}};\n"
            result += f"    }}\n\n"

        # definitions for polymorphic subtypes
        for sub_type_name, sub_type in abstract_type.sub_types.items():
            result += f"    // definitions for {sub_type_name}\n"

            # constructor
            result += f"    {sub_type_name}::{sub_type_name}({parameter_list(sub_type.members)})\n        : {initializer_list(sub_type.members, from_other=False)}\n"
            result += f"    {{ }}\n\n"

            # move constructor
            result += f"    {sub_type_name}::{sub_type_name}({sub_type_name}&& other) noexcept\n        : {initializer_list(sub_type.members, from_other=True)}\n"
            result += f"    {{\n"
            result += f"#ifdef DEBUG_BUILD\n"
            result += f"        if (this == std::addressof(other)) {{\n"
            result += f"            return;\n"
            result += f"        }}\n"
            result += f'        assert(other.all_members_valid() and "move out of partially moved-from value");\n'
            for member in sub_type.members:
                result += f"        m_{member.name}_is_valid = true;\n"
            result += f"#endif\n"
            result += f"    }}\n\n"

            # move assignment
            result += f"    {sub_type_name}& {sub_type_name}::operator=({sub_type_name}&& other) noexcept {{\n"
            result += move_assignment_body(sub_type.members)
            result += f"        return *this;\n"
            result += f"    }}\n\n"

            # type query
            result += f"    [[nodiscard]] bool {sub_type_name}::is_{to_snake_case(sub_type_name)}() const {{\n"
            result += f"        return true;\n"
            result += f"    }}\n\n"

            # type conversion
            result += f"    [[nodiscard]] tl::optional<const {sub_type_name}&> {sub_type_name}::as_{to_snake_case(sub_type_name)}() const& {{\n"
            result += f"        return *this;\n"
            result += f"    }}\n\n"

            # getters
            for member in sub_type.members:
                result += f"    [[nodiscard]] const {member.type_}& {sub_type_name}::{member.name}() const& {{\n"
                result += f"#ifdef DEBUG_BUILD\n"
                result += f'        assert(m_{member.name}_is_valid and "accessing a moved-from value");\n'
                result += f"#endif\n"
                result += f"        return m_{member.name};\n"
                result += f"    }}\n\n"

                if member.by_move:
                    result += f"    [[nodiscard]] {member.type_} {sub_type_name}::{member.name}_moved() & {{\n"
                    result += f"#ifdef DEBUG_BUILD\n"
                    result += f'        assert(m_{member.name}_is_valid and "trying to move out of a moved-from value");\n'
                    result += f"        m_{member.name}_is_valid = false;\n"
                    result += f"#endif\n"
                    result += f"        return std::move(m_{member.name});\n"
                    result += f"    }}\n\n"

            # implementations
            for implementation in sub_type.implementations:
                return_type = [virtual_function for virtual_function in sub_type.pure_virtual_functions if
                               virtual_function.name == implementation.name][0].return_type
                result += f"    [[nodiscard]] {return_type} {sub_type_name}::{implementation.name}() const {{\n"
                lines = [line.strip() for line in implementation.body.split("\n")]
                for line in lines:
                    result += f"        {line}\n"
                result += f"    }}\n\n"

            result += f"#ifdef DEBUG_BUILD\n"
            result += f"    [[nodiscard]] bool {sub_type_name}::all_members_valid() const {{\n"
            newline = "\n"
            result += f"        return {f'{newline}            and '.join(f'm_{member.name}_is_valid' for member in sub_type.members)};\n"
            result += f"    }}\n"
            result += f"#endif\n\n"

    return result


def move_assignment_body(members: list[Member]) -> str:
    result = ""
    result += f"        if (this == std::addressof(other)) {{\n"
    result += f"            return *this;\n"
    result += f"        }}\n"
    result += f"#ifdef DEBUG_BUILD\n"
    result += f'        assert(other.all_members_valid() and "move out of partially moved-from value");\n'
    result += f"#endif\n"
    for member in members:
        if member.by_move:
            result += f"        m_{member.name} = std::move(other.m_{member.name});\n"
        else:
            result += f"        m_{member.name} = other.m_{member.name};\n"
    result += f"#ifdef DEBUG_BUILD\n"
    for member in members:
        result += f"        m_{member.name}_is_valid = true;\n"
    result += f"#endif\n"
    return result


def to_snake_case(pascal_case_string: str) -> str:
    return re.sub(r'(?<!^)(?=[A-Z])', '_', pascal_case_string).lower()
