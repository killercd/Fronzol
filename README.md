# Fronzol

Fronzol is a small terminal-based HTTP/HTTPS client written in C.

It reads URLs interactively, downloads the requested page, and prints the response body in the terminal. The input bar stays pinned to the top of the screen while the response scrolls underneath it, using ANSI terminal escape sequences instead of ncurses.

## Features

- Interactive URL prompt.
- Fixed top input bar using terminal escape codes.
- Scrollable response area below the prompt.
- HTTP support over plain TCP.
- HTTPS support through OpenSSL.
- TLS Server Name Indication (SNI) support.
- Dynamic response allocation, so pages are not truncated by a fixed output buffer.
- Response body extraction by stripping HTTP headers before printing.
- Optimized build flags through the Makefile.

## Requirements

- GCC
- Make
- OpenSSL development headers and libraries

On Debian/Ubuntu-like systems, the OpenSSL dependency is usually provided by:

```sh
sudo apt install libssl-dev
```

## Build

```sh
make
```

This produces the `fronzol` executable.

To rebuild from scratch:

```sh
make clean all
```

## Usage

Run the program without arguments:

```sh
./fronzol
```

Then type a URL into the top input bar:

```text
URL> https://www.google.com/robots.txt
```

The downloaded response body is printed below the input bar.

To exit:

- press Enter on an empty input line
- type `quit`
- type `exit`
- press `Ctrl-D`

## Current Behavior

The browser parses URLs in this form:

```text
http://example.com/path
https://example.com/path
```

For HTTPS URLs, Fronzol opens a TCP connection on port `443`, creates an OpenSSL TLS client, sets SNI to the requested host, performs the TLS handshake, sends a basic HTTP/1.1 GET request, reads the response until the server closes the connection, strips the HTTP headers, and returns the response body.

For HTTP URLs, it performs the same request flow over a plain TCP connection on port `80`.

## Known Limitations

- Only `http` and `https` URLs are supported.
- Custom ports are not parsed yet.
- IPv4 is currently forced in DNS resolution.
- HTTPS is currently restricted to TLS 1.3.
- Redirects are not followed automatically.
- Chunked transfer decoding is not implemented yet.
- The response is stored fully in memory before printing.
- The terminal UI uses ANSI escape sequences and expects a compatible terminal.

## Clean

```sh
make clean
```
