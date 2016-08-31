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

# Usage
On the client machine:
```
./hop <client-hop-port> <server-host-name> <server-hop-port>
```

On the target machine:
```
./hop <server-hop-port> localhost <target-port>
```

In case of SSH, the target-port would be 22.
Now once these 2 are running, to SSH you would run the following:

```
ssh <target-machine-username>@localhost -p <client-hop-port>
```

# Bugs
* Currently uses a 100ms sleep after every send/receive cycle to allow for synchonizations.
* HTTP Responses may come before HTTP Requests. Let me know if you know of some proxy which blocks such responses.

# Planned features
* Better and more adaptive buffering
* More logging levels
* Better CLI flags interface
