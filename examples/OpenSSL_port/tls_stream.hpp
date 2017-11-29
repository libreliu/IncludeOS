#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <net/inet4>

struct TLS_stream //: public net::Stream
{
  using Stream_ptr = net::Stream_ptr;
  using buffer_t = net::Stream::buffer_t;

  enum status_t {
    STATUS_OK,
    STATUS_WANT_IO,
    STATUS_FAIL
  };

  TLS_stream(SSL_CTX* ctx, Stream_ptr t);

  void write(buffer_t buffer);
  void write(const std::string& str);
  void close();

  delegate<void()>         on_connect = nullptr;
  delegate<void(buffer_t)> on_read    = nullptr;
  delegate<void()>         on_close   = nullptr;

  status_t status(int n) const noexcept;

private:
  void tls_read(buffer_t);
  int  tls_perform_stream_write();

  Stream_ptr m_transport = nullptr;
  SSL*       m_ssl    = nullptr;
  BIO*       m_bio_rd = nullptr;
  BIO*       m_bio_wr = nullptr;
};

inline TLS_stream::TLS_stream(SSL_CTX* ctx, Stream_ptr t)
  : m_transport(std::move(t))
{
  this->m_bio_rd = BIO_new(BIO_s_mem());
  this->m_bio_wr = BIO_new(BIO_s_mem());
  this->m_ssl = SSL_new(ctx);
  assert(this->m_ssl != nullptr);
  assert(status(0) == 0 && "Initializing SSL");
  // TLS server-mode
  SSL_set_accept_state(this->m_ssl);
  SSL_set_bio(this->m_ssl, this->m_bio_rd, this->m_bio_wr);
  // callbacks
  m_transport->on_read(8192, {this, &TLS_stream::tls_read});
}

inline void TLS_stream::write(buffer_t buffer)
{
  assert(SSL_is_init_finished(this->m_ssl));

  int n = SSL_write(this->m_ssl, buffer->data(), buffer->size());
  auto status = this->status(n);
  if (status == STATUS_FAIL) {
    this->close();
    return;
  }

  do {
    n = tls_perform_stream_write();
  } while (n > 0);
}
inline void TLS_stream::write(const std::string& str)
{
  write(net::tcp::construct_buffer(str.data(), str.data() + str.size()));
}

inline void TLS_stream::tls_read(buffer_t buffer)
{
  printf("tls_read %lu\n", buffer->size());
  uint8_t* buf = buffer->data();
  int      len = buffer->size();

  while (len > 0)
  {
    int n = BIO_write(this->m_bio_rd, buf, len);
    if (UNLIKELY(n < 0)) {
      printf("BIO_write(1) failed\n");
      this->close();
      return;
    }
    buf += n;
    len -= n;

    // if we aren't finished initializing session
    if (UNLIKELY(!SSL_is_init_finished(this->m_ssl)))
    {
      int num = SSL_accept(this->m_ssl);
      auto status = this->status(num);
      // OpenSSL wants to write
      if (status == STATUS_WANT_IO)
      {
        tls_perform_stream_write();
      }
      else if (status == STATUS_FAIL)
      {
        printf("SSL_accept failed\n");
        this->close();
        return;
      }
      // nothing more to do if still not finished
      if (!SSL_is_init_finished(this->m_ssl)) return;
    }

    // read decrypted data
    do {
      int  len = SSL_pending(this->m_ssl);
      if (len == 0) break;
      auto buf = net::tcp::construct_buffer(len);
      n = SSL_read(this->m_ssl, buf->data(), buf->size());
      if (n > 0) {
        buf->resize(n);
        this->on_read(std::move(buf));
      }
    } while (n > 0);

    auto status = this->status(len);
    // did peer request stream renegotiation?
    if (status == STATUS_WANT_IO)
    {
      do {
        n = tls_perform_stream_write();
      } while (n > 0);
    }
    else if (status == STATUS_FAIL)
    {
      this->close();
      return;
    }
  } // while it < end
} // tls_read()

inline int TLS_stream::tls_perform_stream_write()
{
  char buffer[8192];
  int n = BIO_read(this->m_bio_wr, buffer, sizeof(buffer));
  if (UNLIKELY(n < 0)) {
    this->close();
    return n;
  }
  else if (UNLIKELY(!BIO_should_retry(this->m_bio_wr)))
  {
    this->close();
    return -1;
  }
  m_transport->write(buffer, n);
  return n;
}

inline void TLS_stream::close()
{
  m_transport->close();
}

inline TLS_stream::status_t TLS_stream::status(int n) const noexcept
{
  int error = SSL_get_error(this->m_ssl, n);
  printf("Status: %d\n", error);
  switch (error)
  {
  case SSL_ERROR_NONE:
      return STATUS_OK;
  case SSL_ERROR_WANT_WRITE:
  case SSL_ERROR_WANT_READ:
      return STATUS_WANT_IO;
  default:
      return STATUS_FAIL;
  }
}
