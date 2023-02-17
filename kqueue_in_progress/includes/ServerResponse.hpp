#ifndef SERVERRESPONSE_HPP
#define SERVERRESPONSE_HPP

#include "./Parser.hpp"
#include "./RequestData.hpp"
#include "./Server.hpp"

namespace data {
	
	class Response: public Parser
	{
		private:
			std::string _responseHeader;
			std::string _responseBody;
			std::string _fullResponse;
			std::string _responsePath;

		public:
			Response();
			virtual ~Response();

			std::string streamFile(std::string file);

			// getters
			std::string getHeader();
			std::string getBody();
			std::string getFullResponse();
			std::string getResponsePath();

			//setters
			void setResponse(struct kevent& event);

			std::string setResponseStatus(struct kevent& event);
			void setResponseHeader(std::string val);
			void setResponseBody(std::string file);
			void setResponsePath(std::string file);

			std::string setImage(std::string imagePath);
	};
}
#endif