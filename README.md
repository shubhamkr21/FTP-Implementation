# FTP-Implementation

Two separate programs, client.c and server.c, to implement a simplified version of FTP.
The server process works in connection-oriented and concurrent-server mode.
It first starts the server process in a server host and publish its host name and port number.
A user on another host can then issue a command like;
%myftp server-host-name server-port-number

The following ftp commands are considered:
(a) myftp>fput filename to upload a file named filename to the server,
(b) myftp>fget filename to download a file named filename from the server,
(c) myftp>servls to list the files under the present directory of the server,
(d) myftp>servcd to change the present working directory of the server,
(e) myftp>servpwd to display the present working directory of the server
(f) myftp>clils to list the files under the present directory of the client,
(g) myftp>clicd to change the present directory of the client,
(h) myftp>clipwd to display the present working directory of the client,
(i) myftp>quit to quit from ftp session and return to Unix prompt
