#include "includes/Parser.hpp"

//std::string Parser::removeSpaces(std::string content) {
//    while (!content.empty()) {
//        content.erase(std::find(content.begin(), content.end(), ' '));
//    }
//    return content;
//}

std::string Parser::getOneCleanValueFromKey(std::string & contentLine, std::string const & key) {
	std::string content = contentLine.substr(contentLine.find(key) + key.size());
	if (content.empty())	// added jaka, it was giving error abort, when the content was empty string
		return "Key is empty!\n";

	size_t i = 0;
	for (; content.at(i) == ' '; i++) {}
	std::string trimmed_content = content.substr(i);
    if (trimmed_content.find(';') != std::string::npos) {
        return trimmed_content.substr(0, trimmed_content.size() - 1);
    }
	return trimmed_content.substr(0, trimmed_content.size() - 0);
}

DataType Parser::getValueType(std::string & lineContent) {// Todo: Maybe not needed
	if (lineContent.find("server_name") != std::string::npos) {
		return STRING;
	} else if (lineContent.find("listens_to") != std::string::npos) {
		return PORT;
	} else if (lineContent.find("ip_address") != std::string::npos) {
		return IP_ADDRESS;
	} else if (lineContent.find("root_directory") != std::string::npos) {
		std::string value = getOneCleanValueFromKey(lineContent, "root_directory");
		if (value[0] == '/') {
			return ABSOLUTE_PATH;
		} else {
			/* Will be set to ./$server_name */
			return RELATIVE_PATH;
		}
	} else if (lineContent.find("index_file") != std::string::npos) {
		if (lineContent.find(".html") != std::string::npos) {
			return RELATIVE_PATH_HTML;
		}
	} else if (lineContent.find("client_max_body_size") != std::string::npos) {
		return BYTES;
	} else if (lineContent.find("error_page") != std::string::npos) {
		std::string value = getOneCleanValueFromKey(lineContent, "error_page");
		if (value[0] == '/') {
			return ABSOLUTE_PATH_HTML;
		} else {
			/* Will be set to ./$server_name */
			return RELATIVE_PATH_HTML;
		}
	} else if (lineContent.find("port_redirection") != std::string::npos) {
		return PORT;
	} else if (lineContent.find("location") != std::string::npos) {
		return RELATIVE_PATH;
	} else if (lineContent.find("allow_methods") != std::string::npos) {
		return HTTP_METHOD;
	} else if (lineContent.find("auto_index") != std::string::npos) {
		return BOOL;
	}
	return STRING;
	// todo: add cgi location?
}
