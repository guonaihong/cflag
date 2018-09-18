#include "cflag.h"

typedef struct config_t {
    int nt;
} config_t;

int main(int argc, char **argv) {
    cflagset_t flagset;
    cflag_init(&flag, argc, argv);

    config_t config = {};

    cflag_t []sets = {
        {&config.nt, "nt", "0", "Maximum number of threads", cflag_int},
        NULL,
    };

    cflag_parse(&flagset, sets)
    return 0;
}
