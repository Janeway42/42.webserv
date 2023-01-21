//
// Created by Joyce Macksuele on 18/01/2023.
//

#ifndef WEBSERVER_CONFIGFILEPARSER_H
#define WEBSERVER_CONFIGFILEPARSER_H

#include <string>

namespace parser {

    class Data {
//        typedef struct root_server {
//            ...
//        } RootServer;
        struct root {
            std::string server_name;

        };
        struct location {
            struct location* next;
        };

    };

    class ConfigFile {
    private:
        struct data {
            struct root {

            };
            struct location {
                struct location* next;
            };

        };
    public:
    };

} // parser

#endif //WEBSERVER_CONFIGFILEPARSER_H
