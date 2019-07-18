#include "cous_a.h"


#ifdef COUS_USE_CURL


#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#define CURL_STATICLIB
#include <curl/curl.h>



static COUS_Context ctx[1] = { 0 };




static curl_socket_t COUS_onOpensocket(void* clientp, curlsocktype purpose, struct curl_sockaddr* address)
{
    SOCKET s = socket(address->family, address->socktype, address->protocol);
    return s;
}

static size_t COUS_onHeader(char* buffer, size_t size, size_t nitems, void* userdata)
{
    if (!ctx->handshaked)
    {
        ctx->handshaked = true;

        printf("[COUS] handshaked\n");
    }
    printf("%s", buffer);

    size_t numBytes = size * nitems;
    //curl_easy_pause(ctx->handle, CURLPAUSE_ALL);
    //curl_easy_setopt(ctx->handle, CURLOPT_FORBID_REUSE, 1);
    //curl_easy_setopt(ctx->handle, CURLOPT_CONNECT_ONLY, 1);
    return numBytes;
}

static size_t COUS_onWrite(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t numBytes = size * nmemb;
    COUS_onData(ctx, ptr, (u32)numBytes);
    return numBytes;
}

static size_t COUS_onRead(char* buffer, size_t size, size_t nitems, void* userdata)
{
    size_t numBytes = size * nitems;
    return numBytes;
}






bool COUS_connect(const char* host, u32 port, const char* uri)
{
    stzncpy(ctx->host, host, HOST_NAME_MAX);
    ctx->port = port;
    stzncpy(ctx->uri, uri, MAX_REQUEST_PATH_LENGTH);

    CURLcode res;

    struct curl_slist* headerList;
    CURL* handle = curl_easy_init();
    if (!handle)
    {
        return false;
    }
    ctx->handle = handle;
    headerList = curl_slist_append(NULL, "HTTP/1.1 101 WebSocket Protocol Handshake");
    headerList = curl_slist_append(headerList, "Upgrade: WebSocket");
    headerList = curl_slist_append(headerList, "Connection: Upgrade");
    headerList = curl_slist_append(headerList, "Sec-WebSocket-Version: 13");
    {
        u32 key[WS_KEY_SIZE / 4];
        for (u32 i = 0; i < WS_KEY_SIZE / 4; ++i)
        {
            key[i] = rand();
        }
        char keyStr[((WS_KEY_SIZE + 2) / 3 * 4) + 1];
        base64encode(keyStr, (char*)key, WS_KEY_SIZE);

        const char* requestFmt = "Sec-WebSocket-Key: %s";
        s32 n = snprintf(NULL, 0, requestFmt, keyStr);
        vec_resize(ctx->sendBuf, n + 1);
        n = snprintf(ctx->sendBuf->data, ctx->sendBuf->length, requestFmt, keyStr);
        headerList = curl_slist_append(headerList, ctx->sendBuf->data);
    }

    u32 n = snprintf(NULL, 0, "http://%s:%u/%s", host, port, uri);
    vec_resize(ctx->sendBuf, n + 1);
    snprintf(ctx->sendBuf->data, ctx->sendBuf->length, "http://%s:%u%s", host, port, uri);

    curl_easy_setopt(handle, CURLOPT_URL, ctx->sendBuf->data);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(handle, CURLOPT_OPENSOCKETFUNCTION, COUS_onOpensocket);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, COUS_onHeader);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, COUS_onWrite);
    curl_easy_setopt(handle, CURLOPT_READFUNCTION, COUS_onRead);

    res = curl_easy_perform(handle);
    if (res != CURLE_OK)
    {
        const char* errStr = curl_easy_strerror(res);
        printf("[COUS] error: %s\n", errStr);
        curl_easy_cleanup(ctx->handle);
        return false;
    }
    ctx->connected = true;
    return true;
}








void COUS_cleanup(void)
{
    curl_easy_cleanup(ctx->handle);
    COUS_contextFree(ctx);
}








bool COUS_update(void)
{
    return true;
}


















void COUS_sendText(const char* text, u32 len)
{
    curl_easy_pause(ctx->handle, CURLPAUSE_ALL);

    size_t n;
    CURLcode res = curl_easy_send(ctx->handle, text, len, &n);
    if (res != CURLE_OK)
    {
        const char* errStr = curl_easy_strerror(res);
        printf("[COUS] error: %s\n", errStr);
    }
}

void COUS_sendBinrary(const char* data, u32 len)
{
}

void COUS_sendClose(void)
{
}










































#endif




































































