# 🌐 Webserv

An HTTP server built from scratch in **C++**, inspired by **Nginx**, developed as part of the **1337/42 Network** curriculum.

---

## 👥 Team

- **Yasser Ait Nasser**
- **Ismail Aderdour**
- **Assma Sabir**

---

## ⚙️ Features

- **HTTP/1.1** request parsing and response generation
- **Supported methods:** `GET`, `POST`, and `DELETE` 
- **Multiple client handling** using non-blocking sockets and `poll()`  
- **Chunked transfer encoding**  
- **CGI execution**
- **Configurable virtual hosts**, routes, allowed methods, and error pages  
- **File uploads**, **autoindex (directory listing)**, and **redirects**  
- **Serving static and dynamic content**  
- Proper **status code** handling  

---

## 🏗️ Installation & Usage

### 1. Clone the repository
```bash
git clone https://github.com/assmasabir/webserv.git
cd webserv
```
### 2. Build the project
```bash
make
```
### 3. Run the server
```bash
./webserv config/config.conf
```
### 3. Access the server
Once the server is running, open a browser and go to:
```bash
http://localhost:8080
```
You can also test the server using curl:
```bash
curl -v http://localhost:8080
```
You can configure the config file as you want.

