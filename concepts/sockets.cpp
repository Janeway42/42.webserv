#include <sys/socket.h>

int server_fd = socket(domain, type, protocol);

// domain, or address family:
// communication domain in which the socket should be created. 
// Some of address families are AF_INET (IP), AF_INET6 (IPv6), AF_UNIX (local channel, similar to pipes), 
// AF_ISO (ISO protocols), and AF_NS (Xerox Network Systems protocols).

// type :
// type of service. This is selected according to the properties required by the application: 
// SOCK_STREAM (virtual circuit service), SOCK_DGRAM (datagram service), SOCK_RAW (direct IP service). 
// Check with your address family to see whether a particular service is available.

// protocol:
// indicate a specific protocol to use in supporting the sockets operation. 
// This is useful in cases where some families may have more than one protocol to support a given type of service. 
// The return value is a file descriptor (a small integer). 
// The analogy of creating a socket is that of requesting a telephone line from the phone company.


//identify (name) a socket - assign a transport addrress
int bind(int socket, const struct sockaddr *address, socklen_t address_len);

// The first parameter, socket, is the socket that was created with the socket system call.

// For the second parameter, the structure sockaddr is a generic container that just allows the OS 
// to be able to read the first couple of bytes that identify the address family. 
// The address family determines what variant of the sockaddr struct to use that contains elements 
// that make sense for that specific communication type. For IP networking, we use struct sockaddr_in, 
// which is defined in the header netinet/in.h. This structure defines:

// struct sockaddr_in 
// { 
//     __uint8_t         sin_len; 
//     sa_family_t       sin_family; 
//     in_port_t         sin_port; 
//     struct in_addr    sin_addr; 
//     char              sin_zero[8]; 
// };

// Before calling bind, we need to fill out this structure. The three key parts we need to set are:

// sin_family
// The address family we used when we set up the socket. In our case, it’s AF_INET.

// sin_port
// The port number (the transport address). You can explicitly assign a transport address (port) 
// or allow the operating system to assign one. If you’re a client and won’t be receiving incoming connections, 
// you’ll usually just let the operating system pick any available port number by specifying port 0. 
// If you’re a server, you’ll generally pick a specific number since clients will need to know a port number to connect to.

// sin_addr
// The address for this socket. This is just your machine’s IP address. 
// With IP, your machine will have one IP address for each network interface. 
// For example, if your machine has both Wi-Fi and ethernet connections, that machine will have two addresses, 
// one for each interface. Most of the time, we don’t care to specify a specific interface 
// and can let the operating system use whatever it wants. The special address for this is 0.0.0.0, defined by the symbolic constant INADDR_ANY.

// Since the address structure may differ based on the type of transport used, the third parameter 
// specifies the length of that structure. This is simply sizeof(struct sockaddr_in).

// The code to bind a socket looks like this:

// #include <sys/socket.h> 

// struct sockaddr_in address;
// const int PORT = 8080; //Where the clients can reach at/* htonl converts a long integer (e.g. address) to a network representation */ /* htons converts a short integer (e.g. port) to a network representation */ memset((char *)&address, 0, sizeof(address)); 
// address.sin_family = AF_INET; 
// address.sin_addr.s_addr = htonl(INADDR_ANY); 
// address.sin_port = htons(PORT); if (bind(server_fd,(struct sockaddr *)&address,sizeof(address)) < 0) 
// { 
//     perror(“bind failed”); 
//     return 0; 
// }