
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <set>
#include <cctype>

#include "httprequest.hpp"

// This is the heart of the HTTP parsing.

// You should first take the request string and break it up
// into substrings wherever there is a '\r\n' pair.  The 
// easiest way to do this is to iterate through the entire input,
// keeping track of the current character, where in the string you are
// and the last character. When you get a '\n' when the previous was '\r',
// you have found the end of the substring (well, 2 past the end 
// the substring), allowing you to use std::string's substr function.
//
// But at the same time you shouldn't ever have a '\n' without a prior '\r', or 
// a '\r' without a subsequent '\n', however this is something we are not going to
// require you to check for.
// 
// (Note that substr takes the starting index and the length, so you will
// want to keep track of the current index, the last start point, and possibly
// the length as well).

// Finally, if you have '\r\n\r\n' in a row that is the end of the headers and you
// can use a break to exit the loop.  This ending must be present and if it isn't you
// should raise a MalformedRequestException.  

// Anything past the \r\n\r\n is the "payload".

// Now that it is split up, you can then parse both the command (the first part)
// and then the headers.



// 去除左侧空格
std::string stripl(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start])) {
        ++start;
    }
    return s.substr(start);
}

// 去除右侧空格
std::string stripr(const std::string &s) {
    size_t end = s.size();
    while (end > 0 && std::isspace(s[end - 1])) {
        --end;
    }
    return s.substr(0, end);
}

// 去除两侧空格
std::string strip(const std::string &s) {
    return stripr(stripl(s));
}

// 将字符串转换为小写
std::string to_lower(const std::string &s) {
    std::string lower_case;
    lower_case.reserve(s.size());
    std::transform(s.begin(), s.end(), std::back_inserter(lower_case), [](unsigned char c) { return std::tolower(c); });
    return lower_case;
}



HTTPRequest::HTTPRequest(std::string request){

  if (request.empty()) {
    throw MalformedRequestException("Must have at least a command");
  }
  std::istringstream stream(request);
  std::string line;
  if (std::getline(stream, line)) {
        parse_command(line);
  }

  bool first_line = true;
  bool headers_done = false;

  while (std::getline(stream, line)) {

    if (line.back() == '\r') {

      line.pop_back();
    }

    if (line.empty()) {
      headers_done = true;
      break;  // 结束头部解析
    }

    if (first_line) {
      parse_command(line);
      first_line = false;
    } else {

      auto pos = line.find(':');
      if (pos != std::string::npos) {
                // 打印解析的头部
      std::cout << "Parsing header: " << line << std::endl;
      std::string header_name = to_lower(stripl(line.substr(0, pos)));
      std::string header_value = strip(line.substr(pos + 1));
      headers[header_name] = std::make_shared<HTTPHeader>(header_name + ": " + header_value);
    } else {

      throw MalformedRequestException("Header missing ':'");
    }
   }
 }

    if (!headers_done) {
        throw MalformedRequestException("Headers not properly terminated");
    }

    // 读取负载
    std::string payload_line;
    while (std::getline(stream, payload_line)) {

      if (!payload_line.empty() || headers_done) { 
         // 确保仅在头部结束后读取负载
         payload += payload_line + "\n";
        }
    }
    if (!payload.empty() && payload.back() == '\n') {

      payload.pop_back();
    }
}



auto valid_commands = {"GET", "POST", "HEAD"};

void HTTPRequest::parse_command(std::string &command_string){
  std::istringstream stream(command_string);
  stream >> command;
  stream >> resource;

  command = strip(command);   // 修改：移除命令前后的空格
  resource = strip(resource); // 修改：移除资源路径前后的空格

  if (std::find(valid_commands.begin(), valid_commands.end(), command) == valid_commands.end()) {
    throw MalformedRequestException("Invalid command: " + command);
  }

 
}

// This is the portion for a header.  The name should be the
// header name (the part before the :) in all lower case, and
// whitespace stripped out.  It technically should also not include
// any internal whitespace but we arent' checking for that.

// The value is ALL the data after the :, including any whitespace!
HTTPHeader::HTTPHeader(std::string s){
    auto pos = s.find(':');
    if (pos == std::string::npos) {
      throw MalformedRequestException("Header missing ':'");
    }
    name = to_lower(stripl(s.substr(0, pos))); // 转换为小写
    value = stripr(s.substr(pos + 1));
    
}