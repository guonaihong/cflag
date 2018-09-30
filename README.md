##### cflag

##### 主要功能如下
* 可以解析bool, double, int, str 类型
* 编写扩展类型很简单

##### 主要函数
* 初始化函数
```c
int cflag_init(cflagset_t *c,       //cflag核心对象，所以的工作都围绕这个对象
               char *name,          //进程名
               int error_handling); //出错时，可以控制是直接退出进程，还是abort，还是return返回
```
* 解析函数
```c
int cflag_parse(cflagset_t *c,
                cflag_t *cf,        //存放返回值，命令行选项名，帮助信息，默认值
                char **argv);       //命令行参数
```

* 销毁函数(c语言特色)
```c
void cflag_free(cflagset_t *c);
```
##### 示例
``` c
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
```

##### 编写扩展类型
cflag已经自带, int, bool, str, double, ip, port, addr等多个类型
但是真实世界是复杂的，很可能遇到cflag自带类型不能满足的需求,这时候就学要自己编写扩展类型了

``` c
static inline int cflag_xxx(cflag_t *flag, const char *val) {
    flag->isbool = 0;            //这行必须加上

    xxx new_val  = todo(val);    //对val进行一些处理, todo 代指用户自己的逻辑
    *(xxx *)flag->val = new_val; //把处理后的值赋值给调用端
}
```
