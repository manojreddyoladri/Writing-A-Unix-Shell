#include <sstream>
#include <regex>

#include "parser.hpp"

namespace {
    enum class shell_token_type {
        text,
        redirect_cin,
        redirect_cout,
        append_cout,
        pipe,
        logical_and,
        logical_or,
        semicolon
    };

    shell_token_type get_shell_token_type(const std::string& token)
    {
        switch (token.size()) {
        case 1:
            switch (token[0]) {
            case '<':
                return shell_token_type::redirect_cin;
            case '>':
                return shell_token_type::redirect_cout;
            case '|':
                return shell_token_type::pipe;
            case ';':
                return shell_token_type::semicolon;
            }
        case 2:
            if (token == ">>") {
                return shell_token_type::append_cout;
            }
            else if (token == "&&") {
                return shell_token_type::logical_and;
            }
            else if (token == "||") {
                return shell_token_type::logical_or;
            }
        }

        return shell_token_type::text;
    }
}

std::vector<shell_command> parse_command_string(const std::string& str)
{
    std::vector<shell_command> commands(1);

	// The semicolon can legally be at the end of a command OR have another
	// command following it. The original code assumed a semicolon *must* be
	// followed by a command. This boolean helps ensure this correct behavior.
	bool ending_semicolon = false;

    enum class parser_state {
        need_any_token,
        need_new_command,
        need_in_path,
        need_out_path
    } state = parser_state::need_new_command;

	// put spaces around operators > < >> && || ; |
    std::string new_str;
    for (int i = 0; i < (int)str.length(); i++) {
		// first look for && || and >>
        if ((str[i] == '>' && str[i+1] == '>') ||
			(str[i] == '&' && str[i+1] == '&') ||
			(str[i] == '|' && str[i+1] == '|')){
				new_str += std::string(" ")+str[i]+str[i+1]+" ";
				i++; // consumed 2 chars, so advance the counter
		}
		// next look for > < ; |
		else if(str[i] == ';' ||
				str[i] == '>' ||
				str[i] == '<' ||
				str[i] == '|') {
				new_str += std::string(" ")+str[i]+" ";
		}
		// no operator, so just add the character
		else {
				new_str += str[i];
		}
    }
	// std::cout << "str: " << str << std::endl;
	// std::cout << "new_str: " << new_str << std::endl;		

	// Now parse and iterate over the tokens
	std::istringstream iss(new_str);
    std::string token;
    while (iss >> token) {
        auto token_type = get_shell_token_type(token);

        switch (state) {				
        case parser_state::need_any_token:
            switch (token_type) {
            case shell_token_type::text:
                commands.back().args.push_back(token);
                break;

            case shell_token_type::redirect_cin:
                if (commands.back().cin_mode == istream_mode::pipe) {
                    throw parsing_error("Ambiguous input redirect.");
                }
                commands.back().cin_mode = istream_mode::file;
                state = parser_state::need_in_path;
                break;

            case shell_token_type::redirect_cout:
                commands.back().cout_mode = ostream_mode::file;
                state = parser_state::need_out_path;
                break;

            case shell_token_type::append_cout:
                commands.back().cout_mode = ostream_mode::append;
                state = parser_state::need_out_path;
                break;

            case shell_token_type::pipe:
                if (commands.back().cout_mode != ostream_mode::term) {
                    throw parsing_error("Ambiguous output redirect.");
                }
                commands.back().cout_mode = ostream_mode::pipe;
                commands.emplace_back();
                commands.back().cin_mode = istream_mode::pipe;
                state = parser_state::need_new_command;
                break;

            case shell_token_type::logical_and:
                commands.back().next_mode = next_command_mode::on_success;
                commands.emplace_back();
                state = parser_state::need_new_command;
                break;

            case shell_token_type::logical_or:
                commands.back().next_mode = next_command_mode::on_fail;
                commands.emplace_back();
                state = parser_state::need_new_command;
                break;

            case shell_token_type::semicolon:
                commands.emplace_back();
                state = parser_state::need_new_command;
				ending_semicolon = true; // ; might not be followed by a command
                break;
            }
            break;

        case parser_state::need_new_command:
			if (token_type != shell_token_type::text) {
                throw parsing_error("Invalid NULL command");
            }
            commands.back().cmd = token;
            state = parser_state::need_any_token;
			ending_semicolon = false; // change this back to false if ; isn't at the end
            break;

        case parser_state::need_in_path:
            if (token_type != shell_token_type::text) {
                throw parsing_error("Expecting an input path");
            }
            commands.back().cin_file = token;
            state = parser_state::need_any_token;
            break;

        case parser_state::need_out_path:
				
            if (token_type != shell_token_type::text) {
                throw parsing_error("Expecting an output path");
            }
            commands.back().cout_file = token;
            state = parser_state::need_any_token;
            break;
        }
    }

	// This is a little hack-ey. Ideally the while loop above would
	// execute one last time so the state machine could finish. But that's not
	// easily doable given the architecture. So I just execute the state switch
	// one last time for the states that could throw an error.
	// switch on the state one last time
	switch (state) {				
	case parser_state::need_new_command:
		if(ending_semicolon == false) // no semicolon at end
			 throw parsing_error("Invalid NULL command");
		break;

	case parser_state::need_in_path:
		throw parsing_error("Expecting an input path");
		break;

	case parser_state::need_out_path:
		throw parsing_error("Expecting an output path");
		break;
	default: // do nothing
		break;
	}

	if (commands.back().cmd == "") {
			commands.pop_back();
    }

    return commands;
}
