# HTTP-Server (webserv)

An event-driven HTTP/1.1 server written in C++98, inspired by NGINX. Single-threaded,
built around one `epoll` loop, config-file driven, with CGI support. 1337 (42 Network) project.

## What it does

- **Methods**: GET, POST, DELETE.
- **Config file**: YAML (`config.yaml`), one `server:` block per virtual server. Each
  server has its own `listen` (ip:port), `server_names`, `max_client_body_size`,
  `error_page` overrides, and per-`location` blocks (`allow_methods`, `root`, `index`,
  `autoindex`, `cgi_path`, `upload_path`, `return`).
- **Multiple servers / ports**: the `Cluster` can bind several virtual servers on
  different ip:port pairs. When two servers share the same ip:port, the `Host`
  header picks between them.
- **CGI**: PHP/Python/Ruby scripts run via `fork` + `execve`, output captured to a
  temp file, with a 5-second timeout enforced by polling the child with `waitpid`.
- **Autoindex**: directory listing when no index file is found and `autoindex: on`.
- **Uploads**: POST bodies (regular, chunked, and multipart/form-data) written
  directly into `upload_path`.
- **Redirects**: `return` directive for simple HTTP redirects.
- **Custom error pages**: per-status HTML pages in `error_pages/`, overridable per
  server in the config.
- **I/O model**: event-driven (one `epoll` loop, one read/write per readiness
  event), `EPOLLIN`/`EPOLLOUT` driven, with per-client buffering (`Binary`
  read/write buffers).

## How it works

```
main.cpp -> ConfigParser (parses config.yaml into ClusterConfig)
         -> Cluster (owns the epoll fd, one Server per virtual host)
              -> Server (binds/listens, one listening socket)
              -> ClientHandler (one per connected fd, extends RequestParser)
```

Event loop (`Cluster::eventLoop`) calls `epoll_wait`, then either accepts a new
connection on a listening socket or routes the event to the matching
`ClientHandler`. Each `ClientHandler`:

1. Reads bytes as they become available, buffers them until the full header
   block is present (`RequestParser::loadHeaders`).
2. Parses the request line, headers and URI, resolves the virtual server by
   `Host` header, and matches the request against the configured `location`.
3. Dispatches to `GetMethod` / `PostMethod` / `DeleteMethod`, or to `execCGI`
   if the target maps to a CGI extension.
4. Builds a response (`setResponseParams` / `generateHeaders` / `SendResponse`)
   and streams it back once the socket is writable.
5. HTTP errors are thrown as `HttpError` exceptions and turned into the
   matching status code and error page.

## My part

Built with Hamza Barrak and Haytham Bouabdallaoui. I worked on `Cluster`
(epoll loop, multi-server setup, accept/dispatch), `Server` (socket setup),
the `ClientHandler`/`RequestParser` structure, and fixes to CGI termination,
POST body reading, response sending and URI parsing.

## Build & run

```sh
make            # builds ./webserv (c++98, -Wall -Wextra -Werror, ASan enabled)
./webserv                 # uses ./config.yaml
./webserv path/to/config.yaml
```

## What I learned

- Writing an event-driven loop with `epoll` instead of one-thread-per-client.
- Running external programs safely with `fork`/`execve`/`waitpid`, including
  timing out and killing a hung CGI child.
- Streaming HTTP request/response handling with partial reads and writes, since
  a request or response is rarely available in a single `recv`/`send` call.
- Mapping a config file to per-virtual-host routing (`Host` header, location
  matching, per-location method/CGI/upload rules).
