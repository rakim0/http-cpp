# HTTP Server from Scratch

A simple HTTP server built using raw TCP sockets as a learning project.  
The goal was to understand how the web works under the hood by implementing the basics of the HTTP protocol without relying on external frameworks.

---

## Features
- Handles **GET** and **POST** requests
- Parses **HTTP headers** and **request body**
- Responds with proper **status codes** and **message bodies**
- Serves files from the local directory
- Supports **multiple concurrent client connections**

---

## Stages Implemented
1. Bind to a port and accept TCP connections  
2. Respond with `200 OK` for valid requests  
3. Extract and parse the **URL path**  
4. Send back response bodies  
5. Parse and handle **HTTP headers**  
6. Enable **concurrent connections** using threading  
7. Serve **static files**  
8. Read and process **POST request bodies**

---

## Tech Stack
- **Language:** C++
- **Concepts:** TCP sockets, networking, HTTP protocol, concurrency  

---

## How to Run
1. Clone the repository  
   ```bash
   git clone https://github.com/your-username/http-server.git
   cd http-server
