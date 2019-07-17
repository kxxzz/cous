#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "cous.h"

#include <webby.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#include <threads.h>
#include <sleep.h>
#include <atomic.h>






#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))






static int mainReturn(int r)
{
#if !defined(NDEBUG) && defined(_WIN32)
    system("pause");
#endif
    return r;
}





enum
{
    TestServerConnection_MAX = 8
};

typedef struct TestServer
{
    struct WebbyServer* server;
    struct WebbyServerConfig config[1];
    void* memory;
    u32 memorySize;

    u32 connectionCount;
    struct WebbyConnection* connection[TestServerConnection_MAX];
    thrd_t thrd;
    int64_t shutdown[1];
} TestServer;

static TestServer srv[1] = { 0 };


static void TestServer_log(const char* text)
{
    printf("[TestServer] %s\n", text);
}

static int TestServer_dispatch(struct WebbyConnection* conn)
{
    if (0 == strcmp("/foo", conn->request.uri))
    {
        WebbyBeginResponse(conn, 200, 14, NULL, 0);
        WebbyWrite(conn, "Hello, world!\n", 14);
        WebbyEndResponse(conn);
        return 0;
    }
    else if (0 == strcmp("/bar", conn->request.uri))
    {
        WebbyBeginResponse(conn, 200, -1, NULL, 0);
        WebbyWrite(conn, "Hello, world!\n", 14);
        WebbyWrite(conn, "Hello, world?\n", 14);
        WebbyEndResponse(conn);
        return 0;
    }
    else if (0 == strcmp(conn->request.uri, "/quit"))
    {
        WebbyBeginResponse(conn, 200, -1, NULL, 0);
        WebbyPrintf(conn, "Goodbye, cruel world\n");
        WebbyEndResponse(conn);
        return 0;
    }
    else
    {
        return 1;
    }
}

static int TestServer_connect(struct WebbyConnection* conn)
{
    if (0 == strcmp(conn->request.uri, "/") && srv->connectionCount < TestServerConnection_MAX)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static void TestServer_connected(struct WebbyConnection* conn)
{
    printf("[TestServer] WebSocket connected\n");
    srv->connection[srv->connectionCount++] = conn;
}

static void TestServer_closed(struct WebbyConnection* conn)
{
    printf("[TestServer] WebSocket closed\n");

    for (u32 i = 0; i < srv->connectionCount; i++)
    {
        if (srv->connection[i] == conn)
        {
            int remain = srv->connectionCount - i;
            memmove(srv->connection + i, srv->connection + i + 1, remain * sizeof(srv->connection[0]));
            --srv->connectionCount;
            break;
        }
    }
}

static int TestServer_frame(struct WebbyConnection* conn, const struct WebbyWsFrame* frame)
{
    u32 i = 0;

    printf("[TestServer] WebSocket frame incoming\n");
    printf("  Frame OpCode: %d\n", frame->opcode);
    printf("  Final frame?: %s\n", (frame->flags & WEBBY_WSF_FIN) ? "yes" : "no");
    printf("  Masked?     : %s\n", (frame->flags & WEBBY_WSF_MASKED) ? "yes" : "no");
    printf("  Data Length : %d\n", (int)frame->payload_length);

    while (i < (u32)frame->payload_length)
    {
        unsigned char buffer[16];
        int remain = frame->payload_length - i;
        size_t read_size = remain > (int)sizeof(buffer) ? sizeof(buffer) : (size_t)remain;
        size_t k;

        printf("%08x ", (int)i);

        if (0 != WebbyRead(conn, buffer, read_size))
        {
            break;
        }
        for (k = 0; k < read_size; ++k)
        {
            printf("%02x ", buffer[k]);
        }
        for (k = read_size; k < 16; ++k)
        {
            printf("   ");
        }
        printf(" | ");
        for (k = 0; k < read_size; ++k)
        {
            printf("%c", isprint(buffer[k]) ? buffer[k] : '?');
        }
        printf("\n");
        i += (u32)read_size;
    }
    return 0;
}

static int TestServer_main(void* a)
{
    u32 frameCounter = 0;
    for (;;)
    {
        if (atomic_get(srv->shutdown))
        {
            break;
        }
        WebbyServerUpdate(srv->server);

        /* Push some test data over websockets */
        if (0 == (frameCounter & 0x7f))
        {
            for (u32 i = 0; i < srv->connectionCount; ++i)
            {
                WebbyBeginSocketFrame(srv->connection[i], WEBBY_WS_OP_TEXT_FRAME);
                WebbyPrintf(srv->connection[i], "Hello world over websockets!\n");
                WebbyEndSocketFrame(srv->connection[i]);
            }
        }
        sleep_ms(30);
        ++frameCounter;
    }
    return thrd_success;
}


static void startTestServer(void)
{
    struct WebbyServerConfig* config = srv->config;
    memset(config, 0, sizeof(srv->config));
    config->bind_address = "127.0.0.1";
    config->listening_port = 18081;
    config->flags = WEBBY_SERVER_WEBSOCKETS | WEBBY_SERVER_LOG_DEBUG;
    config->connection_max = 4;
    config->request_buffer_size = 2048;
    config->io_buffer_size = 8192;
    config->dispatch = TestServer_dispatch;
    config->log = TestServer_log;
    config->ws_connect = TestServer_connect;
    config->ws_connected = TestServer_connected;
    config->ws_closed = TestServer_closed;
    config->ws_frame = TestServer_frame;

    assert(!srv->memory);
    srv->memorySize = WebbyServerMemoryNeeded(config);
    srv->memory = malloc(srv->memorySize);
    srv->server = WebbyServerInit(config, srv->memory, srv->memorySize);

    thrd_create(&srv->thrd, (thrd_start_t)TestServer_main, NULL);
}


static void stopTestServer(void)
{
    if (!srv->memory)
    {
        return;
    }
    atomic_set(srv->shutdown, 1);
    thrd_join(srv->thrd, NULL);
    WebbyServerShutdown(srv->server);
    free(srv->memory);
    memset(srv, 0, sizeof(srv));
}












static void test(void)
{
    int r = COUS_connect("127.0.0.1", 18081, "/flex.exe");
    assert(r == 0);
    while (COUS_update())
    {
    }
    COUS_cleanup();
}














int main(int argc, char* argv[])
{
#if !defined(NDEBUG) && defined(_WIN32)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    srand((u32)time(NULL));
#ifdef _WIN32
    {
        WORD wsa_version = MAKEWORD(2, 2);
        WSADATA wsa_data;
        if (0 != WSAStartup(wsa_version, &wsa_data))
        {
            // todo report
            return;
        }
    }
#endif
    //startTestServer();
    test();
    //stopTestServer();
#ifdef _WIN32
    WSACleanup();
#endif
    mainReturn(0);
}



















































































