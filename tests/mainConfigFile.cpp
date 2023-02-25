#include "ConfigFileParser.hpp"

int main(int ac, char **av) {
    if (ac == 2) {
        std::cout << std::boolalpha;
        std::cout << "Configuration file name: " << av[1] << std::endl;
        std::cout << "-----------------------------------------------------------------------------------" << std::endl;
        ConfigFileParser configFileData = ConfigFileParser(av[1]);
        try {
            std::cout << "-----------------------------------------------------------------------------------" << std::endl;
            std::cout << RED << "Number of server block(s): " << configFileData.numberOfServerBlocks() << BACK << std::endl;
            std::cout << RED << "Number of location + cgi block(s): " << configFileData.numberOfLocationBlocks() << BACK << std::endl;

            /* begin() returns an iterator to beginning while cbegin() returns a const_iterator to beginning. */
            std::vector<ServerData>::const_iterator it_server;
            for (it_server = configFileData.getServers().begin(); it_server != configFileData.getServers().cend(); ++it_server) {

                /****************************************** server block data *****************************************/
                std::cout << std::endl << "Starting server block " << std::endl;

                std::string serverName = it_server->getServerName();
                std::cout << GRE << std::left << std::setw(30) << "\"server_name\": " << serverName << BACK << std::endl;

                unsigned int listensTo = it_server->getListensTo();
                std::cout << GRE << std::left << std::setw(30) << "\"listens_to\": " << listensTo << BACK << std::endl;

                std::string ipAddress = it_server->getIpAddress();
                std::cout << GRE << std::left << std::setw(30) << "\"ip_address\": " << ipAddress << BACK << std::endl;

                std::string rootDirectory = it_server->getRootDirectory();
                std::cout << GRE << std::left << std::setw(30) << "\"root_directory\": " << rootDirectory << BACK << std::endl;

                std::string indexFile = it_server->getIndexFile();
                std::cout << GRE << std::left << std::setw(30) << "\"index_file\": " << indexFile << BACK << std::endl;

                unsigned int clientMaxBodySize = it_server->getClientMaxBodySize();
                std::cout << GRE << std::left << std::setw(30) << "\"client_max_body_size\": " << clientMaxBodySize << BACK << std::endl;

                std::string errorPage = it_server->getErrorPage();
                std::cout << GRE << std::left << std::setw(30) << "\"error_page\": " << errorPage << BACK << std::endl;

                unsigned int portRedirection = it_server->getPortRedirection();
                std::cout << GRE << std::left << std::setw(30) << "\"port_redirection\": " << portRedirection << BACK << std::endl;


                std::vector<ServerLocation>::const_iterator it_location;
                for (it_location = it_server->getLocationBlocks().cbegin(); it_location != it_server->getLocationBlocks().cend(); ++it_location) {

                    if (it_location->isLocationCgi()) {
                        /************************************* cgi location block data ************************************/
                        std::cout << std::endl << "Starting location block" << std::endl;
                        std::cout << "Is this location a CGI location: " << it_location->isLocationCgi() << std::endl;

                        std::string cgiLocationRootDirectory = it_location->getRootDirectory();
                        std::cout << GRE << std::left << std::setw(30) << "\"root_directory\": " << cgiLocationRootDirectory << BACK << std::endl;

                        std::string interpreterPath = it_location->getInterpreterPath();
                        std::cout << GRE << std::left << std::setw(30) << "\"interpreter_path\": " << interpreterPath << BACK << std::endl;

                        std::string scriptExtension = it_location->getScriptExtension();
                        std::cout << GRE << std::left << std::setw(30) << "\"script_extension\": " << scriptExtension << BACK << std::endl;

                        std::string cgiLocationIndexFile = it_location->getIndexFile();
                        std::cout << GRE << std::left << std::setw(30) << "\"index_file\": " << cgiLocationIndexFile << BACK << std::endl;
                    } else {
                        /*************************************** location block data **************************************/
                        std::cout << std::endl << "Starting location block " << std::endl;
                        std::cout << "Is this location a CGI location: " << it_location->isLocationCgi() << std::endl;

                        std::string locationRootDirectory = it_location->getRootDirectory();
                        std::cout << GRE << std::left << std::setw(30) << "\"root_directory\": " << locationRootDirectory << BACK << std::endl;

                        std::vector<AllowMethods> locationAllowMethods = it_location->getAllowMethods();
                        std::cout << GRE << std::left << std::setw(30) << "\"allow_methods\": ";
                        std::vector<AllowMethods>::const_iterator i;
                        for (i = locationAllowMethods.cbegin(); i != locationAllowMethods.cend(); i++) {
                            switch (i.operator*()) {
                                case GET:
                                    std::cout << "GET";
                                    break;
                                case POST:
                                    std::cout << "POST";
                                    break;
                                case DELETE:
                                    std::cout << "DELETE";
                                    break;
                                default:
                                    break;
                            }
                            std::cout << " ";
                        }
                        std::cout << BACK << std::endl;

                        std::string locationIndexFile = it_location->getIndexFile();
                        std::cout << GRE << std::left << std::setw(30) << "\"index_file\": " << locationIndexFile << BACK << std::endl;

                        bool locationAutoIndex = it_location->getAutoIndex();
                        std::cout << GRE << std::left << std::setw(30) << "\"auto_index\": " << locationAutoIndex << BACK << std::endl;
                    }
                }
            }
        } catch(std::exception const & ex) {
            std::cout << RED << ex.what() << BACK << std::endl;
        }
    }
    return EXIT_SUCCESS;
}
/* TESTS
#include <iostream>
class Test {
public:
    struct {
        int test_var;
    } test_struct_untagged; // not a type, so it can't be used to make a copy of this struct
    struct name {
        int test_var;
    } test_struct_tagged;
    struct name test_struct_tagged_second_declaration;
    struct same_name {
        int test_var;
    } same_name;
    // initialized "manually"
    struct test_struct_tagged_not_declared_2 {
        int test_var;
    };
    test_struct_tagged_not_declared_2 test_struct_tagged_declared_2;
};
struct test_struct_tagged_not_declared {
    int test_var;
};// not declaring
// also not declaring "manually
int main() {
    Test test;
    test.test_struct_untagged.test_var = 1;
    std::cout << test.test_struct_untagged.test_var << std::endl;
    test.test_struct_tagged.test_var = 2;
    std::cout << test.test_struct_tagged.test_var << std::endl;
    test.same_name.test_var = 3;
    std::cout << test.same_name.test_var << std::endl;
    test.test_struct_tagged_declared_2.test_var = 4;
    std::cout << test.test_struct_tagged_declared_2.test_var << std::endl;
    struct test_struct_tagged_not_declared joyce = {};
    joyce.test_var = 5;
    std::cout << joyce.test_var << std::endl;
    return 0;
}
*/
