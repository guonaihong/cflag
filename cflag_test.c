#include "cflag.h"

typedef struct config_t {
    int   nt;
    int   debug;
    char *model_path;
} config_t;

int main(int argc, char **argv) {
    cflagset_t flag;
    cflag_init(&flag, argv[0], exit_on_error);

    config_t config = {};

    cflag_t sets[] = {
        {&config.nt,         "nt",         "0",       "Maximum number of threads",  cflag_int},
        {&config.debug,      "debug",      "false",   "Open the server debug mode", cflag_bool},
        {&config.model_path, "model_path", "./", "xxx engine directory",  cflag_str},
        {NULL, NULL, NULL, NULL, NULL},
    };

    cflag_parse(&flag, sets, argv+1);

    printf("config.nt         = %d\n", config.nt);
    printf("config.debug      = %d\n", config.debug);
    printf("config.model_path = %s\n", config.model_path);

    free(config.model_path);
    return 0;
}
