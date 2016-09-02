HTTP-Over-Protocol (HOP)
========================

# Introduction
**HOP** is a tool meant to tunnel any sort of traffic over a standard HTTP channel.

Useful for scenarios where there's a proxy filtering all traffic except standard HTTP(S) traffic.

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

# Bugs
* Currently uses a 100ms sleep after every send/receive cycle to allow for synchonizations.
* HTTP Responses may come before HTTP Requests. Let me know if you know of some proxy which blocks such responses.

# Planned features
* Better and adaptive buffering
* Better CLI flags interface
* Parsing of .ssh/config file for hosts
* Web interface for remote server admin
* Web interface for local host
* Daemon mode for certain configs
