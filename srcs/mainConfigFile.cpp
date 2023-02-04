#include "includes/ConfigFileParser.hpp"

int main(int ac, char **av, char **env) {
    (void)env;
    if (ac == 2) {
        std::cout << std::boolalpha;
        std::cout << "Configuration file name: " << av[1] << std::endl;
        std::cout << "-----------------------------------------------------------------------------------" << std::endl;
        ConfigFile configFileData;
        std::map<data::Server*, std::vector<data::Location> > const & serverDataMap = configFileData.parseFile(av[1]);
        std::cout << "-----------------------------------------------------------------------------------" << std::endl;

        if (serverDataMap.empty()) {
            std::cout << RED_BG << "Map is empty!" << BACK << std::endl;
        }

        /** begin() returns an iterator to beginning while cbegin() returns a const_iterator to beginning. */
        std::map<data::Server*, std::vector<data::Location> >::const_iterator it_server;
        for (it_server = serverDataMap.begin(); it_server != serverDataMap.cend(); ++it_server) {
            std::cout << RED << "map size: [" << serverDataMap.size() << "]" << BACK << std::endl;
            std::cout << RED << "location size: [" << it_server->second.size() << "]" << BACK << std::endl;

            /****************************************** server block data *****************************************/
            std::cout << std::endl << "Starting server block " << std::endl;
//            getServerBlockData();

            /** Keep in mind that "first" is the key of the map (i.e. the server block data) */

            std::string serverName = it_server->first->getServerName();
            std::cout << GRE << "Value for \"server_name\" on main: [" << serverName << "]" << BACK << std::endl;

            unsigned int listensTo = it_server->first->getListensTo();
            std::cout << GRE << "Value for \"listens_to\" on main: [" << listensTo << "]" << BACK << std::endl;

            std::string ipAddress = it_server->first->getIpAddress();
            std::cout << GRE << "Value for \"ip_address\" on main: [" << ipAddress << "]" << BACK << std::endl;

            std::string rootDirectory = it_server->first->getRootDirectory();
            std::cout << GRE << "Value for \"root_directory\" on main: [" << rootDirectory << "]" << BACK << std::endl;

            std::string indexFile = it_server->first->getIndexFile();
            std::cout << GRE << "Value for \"index_file\" on main: [" << indexFile << "]" << BACK << std::endl;

            unsigned int clientMaxBodySize = it_server->first->getClientMaxBodySize();
            std::cout << GRE << "Value for \"client_max_body_size\" on main: [" << clientMaxBodySize << "]" << BACK << std::endl;

            std::string errorPage = it_server->first->getErrorPage();
            std::cout << GRE << "Value for \"error_page\" on main: [" << errorPage << "]" << BACK << std::endl;

            unsigned int portRedirection = it_server->first->getPortRedirection();
            std::cout << GRE << "Value for \"port_redirection\" on main: [" << portRedirection << "]" << BACK << std::endl;

            /** Keep in mind that "second" is the value of the map (i.e. the data of the location block(s) inside
             * the current server block).
             * As it is a vector (so it can keep more than one server block if needed, we have to loop on this
             * vector to retrieve all location blocks (i.e. iterate on the it->second).
             * */
            std::vector<data::Location>::const_iterator it_location;

            for (it_location = it_server->second.cbegin(); it_location != it_server->second.cend(); ++it_location) {

                if (it_location->is_location_cgi()) {
                    /************************************* cgi location block data ************************************/
                    std::cout << std::endl << "Starting cgi location block " << std::endl;

                    std::string cgiLocationRootDirectory = it_location->getRootDirectory();
                    std::cout << BLU << "Value for \"root_directory\" on main: [" << cgiLocationRootDirectory << "]" << BACK << std::endl;

                    std::string interpreterPath = it_location->getInterpreterPath();
                    std::cout << BLU << "Value for \"interpreter_path\" on main: [" << interpreterPath << "]" << BACK << std::endl;

                    std::string scriptExtension = it_location->getScriptExtension();
                    std::cout << BLU << "Value for \"script_extension\" on main: [" << scriptExtension << "]" << BACK << std::endl;

                    std::string cgiLocationIndexFile = it_location->getIndexFile();
                    std::cout << BLU << "Value for \"index_file\" on main: [" << cgiLocationIndexFile << "]" << BACK << std::endl;
                } else {
                    /*************************************** location block data **************************************/
                    std::cout << std::endl << "Starting location block " << std::endl;

                    std::string locationRootDirectory = it_location->getRootDirectory();
                    std::cout << BLU << "Value for \"root_directory\" on main: [" << locationRootDirectory << "]" << BACK << std::endl;

                    std::vector<AllowMethods> locationAllowMethods = it_location->getAllowMethods();
                    std::cout << BLU << "Value for \"allow_methods\" on main: [";
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
                    std::cout << "]" << BACK << std::endl;

                    std::string locationIndexFile = it_location->getIndexFile();
                    std::cout << BLU << "Value for \"index_file\" on main: [" << locationIndexFile << "]" << BACK << std::endl;

                    bool locationAutoIndex = it_location->getAutoIndex();
                    std::cout << BLU << "Value for \"auto_index\" on main: [" << locationAutoIndex << "]" << BACK << std::endl;
                }
            }
        }
    }

    std::string n = "12345";
    std::cout << stod(n,) << std::endl;
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
