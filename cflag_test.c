#include "cflag.h"

typedef struct config_t {
    int                 nt;
    int                 debug;
    char               *model_path;
    double              pi;
    unsigned            ip;
    unsigned short      port;
    struct sockaddr_in  addr;
} config_t;

int main(int argc, char **argv) {
    cflagset_t flag;
    cflag_init(&flag, argv[0], exit_on_error);

    config_t config = {};

    cflag_t sets[] = {
        {&config.nt,         "nt",         "0",     "Maximum number of threads",  cflag_int},
        {&config.debug,      "debug",      "false", "Open the server debug mode", cflag_bool},
        {&config.model_path, "model_path", "./",    "xxx engine directory",       cflag_str},
        {&config.pi,         "pi",         "0",     "pi",                         cflag_double},
        {&config.ip,         "ip",         "0",     "Remote server ip",           cflag_ip},
        {&config.port,       "port",       "0",     "Remote server port",         cflag_port},
        {&config.addr,       "addr",       "",      "Log server addr",            cflag_addr},
        {NULL, NULL, NULL, NULL, NULL},
    };

    cflag_parse(&flag, sets, argv+1);

    printf("config.nt         = %d\n",  config.nt);
    printf("config.debug      = %d\n",  config.debug);
    printf("config.model_path = %s\n",  config.model_path);
    printf("config.pi         = %lf\n", config.pi);
    printf("config.ip         = %x\n",  config.ip);
    printf("config.port       = %x\n",  config.port);
    printf("config.addr       = ip(%x), port(%x)\n",  config.addr.sin_addr.s_addr, config.addr.sin_port);

    free(config.model_path);

    cflag_free(&flag);
    return 0;
}
