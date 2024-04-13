# **42.Webserv**


Run:
----
- make
- ./webserv file.config  *(where file is the configuration file for the webserver)* 
-------------------------------------------------------------------

[42 The Network](https://www.42network.org/) Assignment:
-----------

Writing your own HTTP [server](https://en.wikipedia.org/wiki/Web_server) from scratch in C++ 98

<img src="https://github.com/Janeway42/webserv/blob/main/resources/images/webserver-arch.png" width=900 >

---
- Allowed external functions: Everything in C++ 98.
execve, dup, dup2, pipe, strerror, gai_strerror, errno, dup, dup2, fork, socketpair, htons, htonl, ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl, epoll_wait), kqueue (kqueue, kevent), socket, accept, listen, send, recv, chdir bind, connect, getaddrinfo, freeaddrinfo, setsockopt, getsockname, getprotobyname, fcntl, close, read, write, waitpid, kill, signal, access, stat, open, opendir, readdir and closedir.

-------------------------------------------------------------------

Useful info:
--------
- [HTTP (Hypertext Transfer Protocol) ](https://www.techtarget.com/whatis/definition/HTTP-Hypertext-Transfer-Protocol)
- [Basic HTTP server](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)

Ports
- [Service Name and Transport Protocol Port Number Registry](https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml?search=http-alt)
- [About port numbers](https://www.techtarget.com/searchnetworking/definition/port-number)
- [Difference between Port 80 and Port 8080](https://ourtechroom.com/tech/difference-between-port80-port8080/)

Sockets
- Socket Address is a combination of **IP address** and **port number**. Socket uniquely identifies a Transport Protocol Port (TCP, UDP, DCCP, and SCTP) connection.
