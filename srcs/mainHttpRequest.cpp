
#include "../includes/RequestParser.hpp"

void printStoredRequestData(data::Request &request)
{

	data::RequestData reqData = request.getRequestData();

	// PRINT FIRST LINE HEADER
	std::cout << "\nFIRST LINE:  [" RED << reqData.getRequestMethod() << ", "
										<< reqData.getHttpPath() << ", "
										<< reqData.getHttpVersion() << RES "]\n\n";
	// PRINT OTHER HEADERS	
	std::cout << "HEADER FIELDS:\n" BLU;
	std::cout << "Host:           [" << reqData.getRequestHost() << "]\n";
	std::cout << "Accept:         [" << reqData.getRequestAccept() << "]\n";
	std::cout << "Content-Length: [" << reqData.getRequestContentLength() << "]\n";
	std::cout << "Content-Type:   [" << reqData.getRequestContentType() << "]\n\n" RES;

	// PRINT BODY
	std::cout << "REQUEST BODY:\n[" PUR << request.getRequestBody() << RES "]\n";
}



int main(int ac, char **av, char **env) {
	(void)ac;
	(void)av;
	(void)env;


	// DUMMY REQUEST STRINGS
	std::string str1 = 	"GET /index.html HTTP/1.1\n"
						"Host: kostja.se\n"
						"Accept: te";
	std::string str2 = 	"xt/html\n"
						"Content-Type: text/plain\n"
						"Content-Length: 54\r\n\r\n"
						"This is re";
	std::string str3 = "quest body block\nThis is request body block\n";


	const char *CorinasBuffer1 = str1.c_str();
	const char *CorinasBuffer2 = str2.c_str();
	const char *CorinasBuffer3 = str3.c_str();

	data::Request	request;
	request.appendToRequest(CorinasBuffer1); // main
	request.appendToRequest(CorinasBuffer2); // main
	request.appendToRequest(CorinasBuffer3); // main


	// After reading is done, check if all vars are valid
	//		- body lenght == Content-Lenght
	request.checkStoredVars();

	printStoredRequestData(request);

	return EXIT_SUCCESS;
}