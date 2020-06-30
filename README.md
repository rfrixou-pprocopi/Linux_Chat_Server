# Linux_Chat_Server
This is the implementation for a chat application using pthreads on Linux.

Please Note that the two .cpp files provided need to be built separately with the -pthread argument as a part of the gpp compile command.

The chat server is responsible for providing a mechanism for the clients to communicate. This includes allowing clients to login so they can be identified by username and forwarding messages from one client to another. This also includes listing the usernames of the clients that are currently logged in to facilitate communication. The chat application allows multiple clients to connect to a chat server. 

The connection of both client <-> server and client <-> client was achieved using socket programming in a hybrid c/c++ implementation.

In detail, the server is responsible of establishing a connection and responding to the following commands:
  •	login <username>: to establish a connection
  •	list: to provide all connected *to the server* clients
  •	initiate <name>: to establish a direct connection through a different port to the client <name>
  •	active: since we have opted for a single terminal implementation this command was considered unnecessary; thus we chose not to        include it
  •	bye: to terminate both the client<->client and client<->server conversations

The client is responsible of acquiring the user’s input and relaying it to either the server or user according to the state of our implementation. 
