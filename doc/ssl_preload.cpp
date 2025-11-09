#include <openssl/bio.h>
#include <string>
#include <iostream>
#include <mutex>

#include <openssl/ssl.h>

struct IoBuffer
{
    uint8_t *buf;
    size_t buflen;
    size_t pos;
    size_t len;
};

class SslWrapper
{
private:
    struct stor
    {
        std::mutex mutex;
        SSL_CTX *ctx;
        SSL *ssl;
        BIO *bio;
    };
    stor *s;

public:
    SslWrapper();
    ~SslWrapper();
    int init(const std::string &s1, const std::string &s2, const std::string &s3);
    void shutdown();
    int handshake(const void *inbuf, size_t inlen, IoBuffer *outbuf);
    int verifyPeer();
    int encryptionPipelineEnqueue(void *buf, size_t len);
    int encryptionPipelineDequeue(void *buf, size_t len);
    int decryptionPipelineEnqueue(void *buf, size_t len);
    int decryptionPipelineDequeue(void *buf, size_t len);
};

SslWrapper::SslWrapper()
{
}

SslWrapper::~SslWrapper()
{
}

void keylog_callback(const SSL *ssl, const char *line)
{
    printf("%s\n", line);
}

int SslWrapper::init(const std::string &ca_pem, const std::string &our_cert_pem, const std::string &our_pk_pem)
{
    s = new stor();

    std::cout << "construct" << std::endl;
    SSL_library_init();
    s->ctx = SSL_CTX_new(TLS_client_method());

    BIO *our_cert_bio = BIO_new_mem_buf(our_cert_pem.c_str(), our_cert_pem.size());
    BIO *our_pk_bio = BIO_new_mem_buf(our_pk_pem.c_str(), our_pk_pem.size());
    X509 *our_cert = PEM_read_bio_X509(our_cert_bio, nullptr, 0, nullptr);
    EVP_PKEY *our_pk = PEM_read_bio_PrivateKey(our_pk_bio, nullptr, nullptr, nullptr);
    BIO *bio_ssl;

    SSL_CTX_use_certificate(s->ctx, our_cert);
    SSL_CTX_use_PrivateKey(s->ctx, our_pk);
    SSL_CTX_set_keylog_callback(s->ctx, keylog_callback);
    s->ssl = SSL_new(s->ctx);
    BIO_new_bio_pair(&s->bio, 1024 * 128, &bio_ssl, 1024 * 128);
    SSL_set_bio(s->ssl, bio_ssl, bio_ssl);
    SSL_set_connect_state(s->ssl);

    return 1;
}

void SslWrapper::shutdown()
{
    std::cout << "shutdown" << std::endl;
}

int SslWrapper::handshake(const void *inbuf, size_t inlen, IoBuffer *outbuf)
{
    std::lock_guard<std::mutex> guard(s->mutex);

    std::cout << "handshake" << inlen << std::endl;

    if (inlen > 0)
        BIO_write(s->bio, inbuf, inlen);

    int ret = SSL_do_handshake(s->ssl);

    if (ret != 1) {
        if (SSL_get_error(s->ssl, ret) == SSL_ERROR_WANT_READ) {
            int read_len = BIO_pending(s->bio);
            if (read_len > 0) {
                free(outbuf->buf);
                outbuf->buf = (uint8_t*)malloc(read_len + 2);
                outbuf->buflen = read_len + 2;
                outbuf->pos = 0;
                outbuf->buf[0] = 0;
                outbuf->buf[1] = 3;
                outbuf->len = read_len + 2;
                BIO_read(s->bio, &outbuf->buf[2], read_len);
            }
        }
    } else {
        std::cout << SSL_get_cipher_name(s->ssl) << std::endl;
    }

    return ret;
}

int SslWrapper::verifyPeer()
{
    std::cout << "verify" << std::endl;
    return 0;
}

int SslWrapper::encryptionPipelineEnqueue(void *buf, size_t len)
{
    std::lock_guard<std::mutex> guard(s->mutex);

    int ret = SSL_write(s->ssl, buf, len);
    if (ret < 1)
        return -1;

    return BIO_pending(s->bio);
}

int SslWrapper::encryptionPipelineDequeue(void *buf, size_t len)
{
    return BIO_read(s->bio, buf, len);
}

int SslWrapper::decryptionPipelineEnqueue(void *buf, size_t len)
{
    return BIO_write(s->bio, buf, len) == len;
}

int SslWrapper::decryptionPipelineDequeue(void *buf, size_t len)
{
    std::lock_guard<std::mutex> guard(s->mutex);

    int ret = SSL_read(s->ssl, buf, len);
    if (ret < 0 && SSL_get_error(s->ssl, ret) == SSL_ERROR_WANT_READ) {
        return 0;
    }

    return ret;
}
