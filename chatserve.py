#!/usr/bin/env python3

"""
Author: Joshua Kluthe
Description:
This program implements a simple TCP server that allows a client to connect and chat.
"""

from sys import argv
import socket
from os import popen


"""
handle() takes an established TCP connection as a parameter and has no return value.
It gets a user name, and then sends and receives messages with the client
until the user or the client inputs "\quit".
"""
def handle(conn):
    print("Received connection from client. Initiating chat.")
    #get the users desired username
    name = False
    while not name:
        chat_name = input("Input your name: ")
        if (not chat_name) or (len(chat_name) > 10):
            print("USAGE ERROR: Name must be ten or fewer characters, and cannot be empty")
        else:
            name = True
    #chat with the client until "\quit" is entered by either party
    while True:
        if not write_from_conn(conn):
            print("\033[0;37mClient closed connection.".format(end=""))
            conn.close()
            return
        if not write_to_conn(conn, chat_name):
            print("\033[0;37mClosing connection.".format(end=""))
            conn.close()
            return


"""
write_from_conn() takes an established TCP connection as a parameter.
It receives a string message from the client and prints it to the screen in red, right-aligned.
If the message received is "\quit", it returns False, else True.
"""            
def write_from_conn(conn):
    #example for getting terminal width from http://stackoverflow.com/questions/566746/how-to-get-console-window-width-in-python
    rows, columns = popen('stty size', 'r').read().split()
    data = conn.recv(1024).decode()
    print("\033[0;31m{:>{width}}".format(data, width=columns))
    if data[data.find(">") + 2:] == "\\quit":
        return False
    else:
        return True


"""
write_to_conn() takes an established TCP connection and a chat username as parameters.
It receives a string message from the client and prints it to the screen in green.
If the message input is "\quit", it returns False, else True.
"""            
def write_to_conn(conn, chat_name):
    retval = True
    to_send = input("\033[0;32m{}> ".format(chat_name))
    if to_send == "\\quit":
        retval = False
    to_send = "{}> {}\n".format(chat_name, to_send)
    conn.sendall(to_send.encode())
    return retval
    

"""
make_server() takes a port number as a parameter, creates a TCP socket, and binds the 
socket to the host address and port. It returns the socket.
"""
def make_server(port):
    host = socket.gethostname()
    print("Hostname: {}".format(host))
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((host, port))
    return server


"""
Create a server socket object and listen forever, calling handle() to deal with connections.
"""
if __name__ == "__main__":
    if not (len(argv) > 1):
        print("USAGE ERROR: Must supply a port number, format 'chatserve.py PORT'")
        exit(1)
    server = make_server(int(argv[1]))
    #listen forever
    while True:
        server.listen(1)
        conn, address = server.accept()
        handle(conn)

