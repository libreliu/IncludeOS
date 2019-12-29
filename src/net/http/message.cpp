
#include <net/http/message.hpp>

namespace http {

///////////////////////////////////////////////////////////////////////////////
Message::Message(const std::size_t limit) noexcept
  : header_fields_{limit}, headers_complete_{false}
{}

///////////////////////////////////////////////////////////////////////////////
Header& Message::header() noexcept {
  return header_fields_;
}

///////////////////////////////////////////////////////////////////////////////
const Header& Message::header() const noexcept {
  return header_fields_;
}

///////////////////////////////////////////////////////////////////////////////
Message& Message::add_body(const Message_body& message_body) {
  if (message_body.empty()) return *this;
  message_body_ = message_body;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
Message& Message::add_chunk(const std::string& chunk) {
  if (chunk.empty()) return *this;
  message_body_.append(chunk);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
bool Message::has_body() const noexcept {
  return not message_body_.empty();
}

///////////////////////////////////////////////////////////////////////////////
util::sview Message::body() const noexcept {
  return message_body_;
}

///////////////////////////////////////////////////////////////////////////////
Message& Message::clear_body() noexcept {
  message_body_.clear();
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
Message& Message::reset() noexcept {
  header().clear();
  return clear_body();
}

///////////////////////////////////////////////////////////////////////////////
std::string Message::to_string() const {
  std::ostringstream message;
  //-----------------------------------
  message << header_fields_
          << message_body_;
  //-----------------------------------
  return message.str();
}

///////////////////////////////////////////////////////////////////////////////
Message::operator std::string () const {
  return to_string();
}

///////////////////////////////////////////////////////////////////////////////
util::sview Message::private_field() const noexcept {
  return field_;
}

///////////////////////////////////////////////////////////////////////////////
void Message::set_private_field(const char* base, const size_t length) noexcept {
  field_ = {base, length};
}

///////////////////////////////////////////////////////////////////////////////
Message& operator << (Message& message, const Header_set& headers) {
  for (const auto& field : headers) {
    message.header().add_field(field.first, field.second);
  }
  return message;
}

///////////////////////////////////////////////////////////////////////////////
std::ostream& operator << (std::ostream& output_device, const Message& message) {
  return output_device << message.to_string();
}

} //< namespace http