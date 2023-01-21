//
// Created by Joyce Macksuele on 18/01/2023.
//

#ifndef WEBSERVER_CONFIGFILEPARSER_H
#define WEBSERVER_CONFIGFILEPARSER_H

#include <string>

namespace parser {

    class ConfigFile {
        private:
            struct data {
                struct root {
                    std::string server_name;
                } Root;

                /** As more than 1 location block can be added, a linked list makes more sense */
                struct location {
                    
                    struct location* next;
                } Location;

                /** As more than 1 server block can be added, a linked list makes more sense */
                struct data* next{};
            } ServerData;

        public:
    };

} // parser

#endif //WEBSERVER_CONFIGFILEPARSER_H
