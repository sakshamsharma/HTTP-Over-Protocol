HTTP-Over-Protocol (HOP)
========================

```
  _   _  ___  ____  
 | | | |/ _ \|  _ \
 | |_| | | | | |_) |
 |  _  | |_| |  __/
 |_| |_|\___/|_|
```

**NOTE** Have a look at [go-hop](http://github.com/sakshamsharma/go-hop) for an under-development version in Golang.

# Introduction
A friend once told me that his university has an HTTP(S)-only proxy, and thus he is unable to SSH to hosts outside. 5 hours of intensive coding later using code borrowed from my course assignment, here's **HOP**.

**HOP** is a tool meant to tunnel any sort of traffic over a standard HTTP channel.

Useful for scenarios where there's a proxy filtering all traffic except standard HTTP(S) traffic. Unlike other tools which either require you to be behind a proxy which let's you pass arbitrary traffic (possibly after an initial CONNECT request), or tools which work only for SSH, this imposes no such restrictions.

# Working
Assuming you want to use SSH to connect to a remote machine where you don't have root privileges.

There will be 7 entities:

1. Client (Your computer, behind the proxy)
2. Proxy (Evil)
3. Target Server (The remote machine you want to SSH to, from Client)
4. Client HOP process
5. Target HOP process
6. Client SSH process
7. Target SSH process

If there was no proxy, the communication would be something like:
```
Client -> Client SSH process -> Target Server -> Target SSH process
```

In this scenario, here's the proposed method:
```
Client -> Client SSH process -> Client HOP process -> Proxy -> Target HOP process -> Target SSH process -> Target Server
```

**HOP** simply wraps all the data in HTTP packets, and buffers them accordingly.

Another even more complicated scenario would be if you have an external utility server, and need to access another server's
resources from behind a proxy. In this case, *hop* will still run on your external server, but instead of using `localhost`
in the second command (Usage section), use the hostname of the target machine which has the host.

# Usage
On the client machine:
```
./hop <client-hop-port> <server-host-name> <server-hop-port>
```

On the target machine:
```
./hop <server-hop-port> localhost <target-port> SERVER
```
(Note the keyword SERVER at the end)

In case of SSH, the target-port would be 22.
Now once these 2 are running, to SSH you would run the following:

```
ssh <target-machine-username>@localhost -p <client-hop-port>
```

*Note*: The keyword server tells *hop* which side of the connection has to be over HTTP.

# Contributing
Pull Requests are more than welcome! :smile:

I've put down a list of possible ideas if you would like to contribute.

# Planned features
* Better and adaptive buffering
* Better CLI flags interface
* Optional encrypting of data
* Parsing of .ssh/config file for hosts
* Web interface for remote server admin
* Web interface for local host
* Daemon mode for certain configs

# Bugs
* HTTP Responses may come before HTTP Requests. Let me know if you know of some proxy which blocks such responses.
* Logger seems to be non-thread-safe, despite locking. Suspected for memory errors, and thus disabled for now.

# Testing
```
make
./hop 8081 localhost 8091 SERVER
./hop 8083 localhost 8081
python -c 'print("\n".join([ str(i) for i in range(1000000, 2000000)]))' >! ff1
python -c 'print("\n".join([ str(i) for i in range(2000000, 3000000)]))' >! ff2
nc -l 8091 < ff1 >! oo1
nc localhost 8083 < ff2 >! oo2
wc -l oo*
```

This should say that both files contain a million lines. Any less would mean some data loss.
