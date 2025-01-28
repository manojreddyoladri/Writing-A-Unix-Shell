#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <stdexcept>

#include "command.hpp"

/// A parse error exception thrown when the input string is malformed.
struct parsing_error : public std::runtime_error {
    using runtime_error::runtime_error;
};

/// Parses a line of the terminal commands.
///
/// @return a vector of commands.
std::vector<shell_command> parse_command_string(const std::string& str);

#endif
