// libhttpserver SSL HTTP Server Implementation
// Copyright © 2020 Harsath
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <asm-generic/socket.h>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "helper_functions.hpp"
#include "logger_helpers.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <netinet/tcp.h>

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif


using json = nlohmann::json;


class stream_sock_test;
namespace Socket::inetv4 {
	class stream_sock {
		private:
			friend class ::stream_sock_test;
			struct internal_errors {
				bool index_file_not_found = 0;
				bool file_permission_err = 0;
				bool html_file_not_found = 0;
			};
			std::unique_ptr<LoggerHelper> _access_log = LoggerFactory::MakeLog("access.log", LoggerFactory::Log::Access);
			LogMessage _log_msg_helper;
			std::unordered_map<std::string, bool> _flags;
			internal_errors _error_flags;
			std::string _ipv4_addr;
			std::uint16_t _port;
			std::size_t _buffer_size;
			int _sock_fd;
			int _backlog;
			struct sockaddr_in _sock_addr, _client_addr;
			std::string _read_buffer;
			constexpr static std::size_t _c_read_buff_size = 2048;
			char _client_read_buffer[_c_read_buff_size + 1] = "";
			HTTP_STATUS _http_status;
			std::string _html_body; // Other HTML Page constents
			std::string _index_file_path;
			std::string _index_html_content;
			std::vector<std::pair<std::string, std::string>> _page_routes;
			std::unordered_map<std::string, std::string> _post_request_print; // { Post_print_Endpoint, Responce_Text }
			std::unordered_map<std::string, std::string> _post_endpoint; // <Post_endpoint, Post_print_endpoints>
			std::unordered_map<std::string, std::function<std::string(const std::string&)>> _endpoint_call_back_functions;
			std::unordered_map<std::string, std::vector<Post_keyvalue>> _key_value_post; // <Post_endpoint, Key=>value> for x_www_form_urlencoded_parset
		public:
			explicit stream_sock(const std::string ipv4_addr, std::uint16_t port, std::size_t buffer_size, int backlog, const std::string& index_file_path, 
				    const std::string& route_config_filepath, std::function<std::string(const std::string&)>&& call_back_function = nullptr) 
				:   _ipv4_addr(ipv4_addr), _port(port), _buffer_size(buffer_size), 
				    _backlog(backlog), _index_file_path(index_file_path){

				route_conf_parser(route_config_filepath);
				memset(&_sock_addr, 0, sizeof(struct sockaddr_in));
				this->_sock_addr.sin_family = AF_INET;
				inet_pton(AF_INET, this->_ipv4_addr.c_str(), &_sock_addr.sin_addr);
				this->_sock_addr.sin_port = htons(this->_port);

				this->_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
				err_check(this->_sock_fd, "socket");

				int optval = 1;
				int sock_opt = setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&optval), sizeof(optval));
				err_check(sock_opt, "SO_REUSEADDR socket option error");

				int flag = 1;
				sock_opt = setsockopt(this->_sock_fd, SOL_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flag), sizeof(flag));
				err_check(sock_opt, "TCP_NODELAY socket option error");

				int bind_ret = bind(this->_sock_fd, reinterpret_cast<struct sockaddr*>(&this->_sock_addr), sizeof(struct sockaddr_in));
				err_check(bind_ret, "bind");

			} 
			int stream_accept();
			void origin_server_side_responce(const char* client_request, int& client_fd, std::string& http_responce);
			void index_file_reader();
			void html_file_reader(std::string& html_body);
			void route_conf_parser(std::string conf_file_path);
			void create_post_endpoint(std::string&& post_endpoint, std::string&& print_endpoint, 
							bool include_location_header, std::vector<Post_keyvalue>& useragent_post_form_parse,
							std::function<std::string(const std::string&)>&&);
			// DS : <Exists in conf file, HTML file name, Path to file>
			Useragent_requst_resource route_path_exists(std::string client_request_html);
			~stream_sock();
	};
} // End namespace Socket::inetv4

void Socket::inetv4::stream_sock::create_post_endpoint(std::string&& post_endpoint, std::string&& print_endpoint, 
							bool include_location_header, std::vector<Post_keyvalue> &useragent_post_form_parse,
							std::function<std::string(const std::string&)>&& call_back_function = nullptr){
	this->_post_endpoint.emplace(post_endpoint, print_endpoint);
	this->_post_request_print.emplace(std::move(print_endpoint), ""); // length 0 indicates, it has not been filled yet
	this->_flags[post_endpoint] = include_location_header;
	if(call_back_function){
		this->_endpoint_call_back_functions.emplace(std::move(post_endpoint), std::move(call_back_function));
	}else{
		this->_endpoint_call_back_functions.emplace(std::move(post_endpoint), nullptr);
	}
}

Socket::inetv4::stream_sock::~stream_sock(){
	// delete[] _client_read_buffer;
}

// Check the user agent requested HTML's path  config file
Useragent_requst_resource Socket::inetv4::stream_sock::route_path_exists(std::string client_request_html) {
	bool flag = 0;
	std::string path_html;
	std::string html_filename;
	for(const std::pair<std::string, std::string>& pair: _page_routes) {
		if(client_request_html == pair.second) {
			flag = 1;
			path_html = pair.first;
			html_filename = pair.second;
		}
	}
	if(!flag) {
		this->_http_status = NOT_FOUND;
	}

	return {flag, path_html, html_filename};
}

// Parser for HTML Page routes config file
void Socket::inetv4::stream_sock::route_conf_parser(std::string conf_file_path) {
	std::ifstream conf_file(conf_file_path);
	if(conf_file.is_open()) {
		std::string lines;
		while(std::getline(conf_file, lines)) {
			char* original_line = strdup(lines.c_str());
			char* token;
			char* state;
			std::pair<std::string, std::string> temp_pusher;
			if((token = strtok_r(original_line, " ", &state))) {
				temp_pusher.first = token;
				if((token = strtok_r(original_line, " ", &state))) {
					temp_pusher.second = token;
				}
			}
			this->_page_routes.push_back(temp_pusher);
		}				
	}else{
		std::cout << "Cannot open the configuration file\n";
		exit(1);
	}
}

// Index.html parser
void Socket::inetv4::stream_sock::index_file_reader() {
	this->_index_html_content = ""; 
	std::ifstream index_page(_index_file_path);	
	if(index_page.is_open()) {
		std::string html_line;
		while(std::getline(index_page, html_line)) {
			this->_index_html_content += html_line;
		}
	}else{
		this->_error_flags.index_file_not_found = 1;		
	}
}

// reader for requested HTML file
void Socket::inetv4::stream_sock::html_file_reader(std::string& file_path) {
	_html_body = "";
	std::ifstream index_page(file_path);	
	if(index_page.is_open()) {
		std::string html_line;
		while(std::getline(index_page, html_line)) {
			_html_body += html_line;
		}
	}else{
		_error_flags.html_file_not_found = 1;		
	}
}


// Server responc (Called everytime the client requests a resource)
void Socket::inetv4::stream_sock::origin_server_side_responce(const char* client_request, int& client_fd, std::string& http_responce) {
	std::vector<std::string> client_request_http = client_request_html_split(client_request);
	std::vector<std::string> client_request_line = client_request_line_parser(client_request_http[0]);
	std::vector<std::string> client_headers = split_client_header_from_body(client_request);
	_http_status = OK;
	
	// Getting a handle to `User-Agent` pair and filling filling the Logging object
	std::vector<std::pair<std::string, std::string>> client_header_pair = header_field_value_pair(client_headers, _http_status);
	std::function<bool(const std::pair<std::string, std::string>&)> user_agent_capture_fn = [](const std::pair<std::string, std::string>& values) -> bool {
		return values.first == "User-Agent" && values.second != "";
	};
	auto user_agent_iter = std::find_if(std::begin(client_header_pair), std::end(client_header_pair), user_agent_capture_fn);
	if(user_agent_iter != std::end(client_header_pair)){ this->_log_msg_helper.useragent = user_agent_iter->second; }else{ 
		this->_log_msg_helper.useragent = "Unknown";
	}
	this->_log_msg_helper.resource = client_request_http[0];

	if(client_request_line[0] == "GET") {
		std::string _http_header, bad_request;
		switch(_http_status) {
			case BAD_REQUEST: {
				bad_request = "<h2>Something went wrong, 400 Bad Request</h2>";
				_http_header = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length:" + 
						std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
			case OK: {
				if(client_request_line[1] == "/" || client_request_line[1] == "/index.html") {
					index_file_reader();
					std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + 
									std::to_string(_index_html_content.length()) + "\r\n\r\n" + _index_html_content;
					http_responce = std::move(_http_header);
				}else if(_post_request_print.contains(client_request_line[1])){ // print endpoint
					if(_post_request_print[client_request_line[1]].length() != 0){ // checking if not 0 ( !=0 for clarity )
						std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:"
										+ std::to_string(_post_request_print[client_request_line[1]].length()) + 
											"\r\n\r\n" + _post_request_print[client_request_line[1]];

						http_responce = std::move(_http_header);
					}else{
						std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:"
										+ std::to_string(_post_request_print[client_request_line[1]].length()) + "\r\n\r\n" + "";
						http_responce = std::move(_http_header);
					}
				}else{
					Useragent_requst_resource useragent_req_resource = route_path_exists(client_request_line[1]);
					if(useragent_req_resource.file_exists){
						html_file_reader(useragent_req_resource.resource_path);
						std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + 
										std::to_string(_html_body.length()) + "\r\n\r\n" + _html_body;
						http_responce = std::move(_http_header);
					}else{
					bad_request = "<h2>Something went wrong, 404 File Not Found!</h2>";
					std::string _http_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:" + 
									std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
					http_responce = std::move(_http_header);
					break;
					}
				}
				break;
			}
			case FORBIDDEN: {
				bad_request = "<h2>You dont have authorization to access the requested resource, 403 Forbidden</h2>";
				std::string _http_header = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length:" + 
								std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
			default: {
				bad_request = "<h2>Something went wrong, 404 File Not Found!</h2>";
				_http_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:" + 
						std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
		}
		this->_log_msg_helper.log_message = "Successfully served the user-agent";
		this->_log_msg_helper.date = ::get_today_date_full();
		write_log_to_file(this->_access_log, this->_log_msg_helper);
		send(client_fd, http_responce.c_str(), http_responce.length(), 0);
		http_responce = ""; 

	}else if(client_request_line[0] == "POST"){ // Parsed result structure: std::vector<std::pair<std::string, std::string>> for the key-value pair in HTML body
		// responce: 201 Created
		std::string post_client_request_body = client_body_split(client_request);				

		if(_post_endpoint.contains(client_request_line[1])){ // checking if the endpoint exists
			std::pair<std::string, std::string> targer = std::make_pair("Content-Type", "application/x-www-form-urlencoded");

			auto iter_handler = std::find_if(client_header_pair.begin(), client_header_pair.end(), 
							[&](const std::pair<std::string, std::string>& _capture){
								if(_capture.first == "Content-Type")
								{ return true; } else { return false; }
							}); // Checking for the right content type on POST which is supported by the parser

			if(iter_handler != std::end(client_header_pair) && iter_handler->second == "application/x-www-form-urlencoded"){ //handler for URLENCODED type
				x_www_form_urlencoded_parset(post_client_request_body, client_request_line[1], _key_value_post);
				std::string tmp_responce = "";
				for(const Post_keyvalue& key_val : _key_value_post[client_request_line[1]]){
					tmp_responce += key_val.key + " => " + key_val.value + "\n";
				} 
				_post_request_print[_post_endpoint[client_request_line[1]]] = std::move(tmp_responce);
				if(_endpoint_call_back_functions[client_request_line[1]] != nullptr){ //Checking if user callback function entry
					std::string tmp_responce = _endpoint_call_back_functions[client_request_line[1]](post_client_request_body);
					http_responce = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: " 
							+ std::to_string(tmp_responce.length()) + "\r\n\r\n" + std::move(tmp_responce);
				}else{
					std::string created_responce_payload = "Request has been successfully parsed and resource has been created";
					if(this->_flags[client_request_line[1]]){ // checking to see to include Location header
						http_responce = "HTTP/1.1 201 Created\r\nLocation: " + _post_endpoint[client_request_line[1]] + "\r\n" + 
							"Content-Type: text/plain\r\nContent-Length: " + std::to_string(created_responce_payload.length()) + 
							"\r\n\r\n" + created_responce_payload;
					}else{
						http_responce = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: " 
								+ std::to_string(created_responce_payload.length()) + "\r\n\r\n" + created_responce_payload;
					}
				}
			}else if(iter_handler != std::end(client_header_pair) && iter_handler->second == "application/json"){ // handling for JSON content type
				if(this->_endpoint_call_back_functions[client_request_line[1]] != nullptr){ // checking if any callback fn exists on lookup table
					// TODO: create std::pair<bool, string> and check if the request is Successfully parsed or we need to return correct headers on error
					std::string tmp_responce = this->_endpoint_call_back_functions[client_request_line[1]](post_client_request_body);
					http_responce = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: " 
							+ std::to_string(tmp_responce.length()) + "\r\n\r\n" + std::move(tmp_responce);
				}else{
					std::string tmp_responce = "JSON has been parsed by the origin server";
					if(this->_flags[client_request_line[1]]){
						http_responce = "HTTP/1.1 201 Created\r\nLocation: " + _post_endpoint[client_request_line[1]] + "\r\n" + 
							"Content-Type: text/plain\r\nContent-Length: " + std::to_string(tmp_responce.length()) + 
							"\r\n\r\n" + std::move(tmp_responce);
					}else{
						http_responce = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(tmp_responce.length()) + 
								"\r\n\r\n" + std::move(tmp_responce);
					}
				}
			}else{
				std::string not_acc = "Content-Type is not supported by the server, please request in application/x-www-form-urlencoded Content-Type";
				http_responce = "HTTP/1.1 406 Not Acceptable\r\nContent-Type: text/plain\r\nContent-Length: " + 
					std::to_string(not_acc.length()) + "\r\n\r\n" + not_acc;
			}

		}else{ // endpoint does not exist (invalid endpoint)
				std::string not_acc = "Invalid POST endpoint";
				http_responce = "HTTP/1.1 406 Not Acceptable\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(not_acc.length()) + "\r\n\r\n" + not_acc;
		}

		this->_log_msg_helper.log_message = "Successfully served the user-agent";
		this->_log_msg_helper.date = ::get_today_date_full();
		write_log_to_file(this->_access_log, this->_log_msg_helper);
		send(client_fd, http_responce.c_str(), http_responce.length(), 0);
	}
}

int Socket::inetv4::stream_sock::stream_accept() {
	int listen_ret = listen(this->_sock_fd, this->_backlog);
	err_check(listen_ret, "listen");

	int addr_len = sizeof(_ipv4_addr);
	for(;;) {
		int new_client_fd = accept(this->_sock_fd, reinterpret_cast<struct sockaddr*>(&this->_client_addr), reinterpret_cast<socklen_t*>(&addr_len));
		err_check(new_client_fd, "client socket");

		this->_log_msg_helper.client_ip = inet_ntoa(this->_client_addr.sin_addr);
		
		int read_len = recv(new_client_fd, this->_client_read_buffer, this->_c_read_buff_size, 0);
		err_check(read_len, "client read");

		origin_server_side_responce(this->_client_read_buffer, new_client_fd, this->_html_body);
		
		std::cout << "Msg sent\n";
		close(new_client_fd);
	}
}
