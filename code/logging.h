#ifndef LOGGING
#define LOGGING

#include <stdlib.h>
#include <stdio.h>

struct printer
{
    int count;
};

printer __printer__ = {};

printer operator,(printer p, const char* s)
{
    __printer__.count += printf("%s", s);
    return __printer__;
}

printer operator,(printer p, char* s)
{
    __printer__.count += printf("%s", s);
    return __printer__;
}

printer& operator,(printer p, int a)
{
    __printer__.count += printf("%d", a);
    return __printer__;
}

printer& operator,(printer p, uint a)
{
    __printer__.count += printf("%u", a);
    return __printer__;
}

printer& operator,(printer p, size_t a)
{
    __printer__.count += printf("%zu", a);
    return __printer__;
}

printer& operator,(printer p, float a)
{
    __printer__.count += printf("%f", a);
    return __printer__;
}

printer& operator,(printer p, DWORD a)
{
    __printer__.count += printf("%li", a);
    return __printer__;
}

/* printer& operator,(printer p, bool a) */
/* { */
/*     __printer__.count += printf("%s", a ? "true" : "false"); */
/*     return __printer__; */
/* } */

#define log_warning(...) (__printer__, "warning: ", __VA_ARGS__, "\n");

#define log_error(...) {(__printer__, __FILE__, "(", __LINE__, "): error: ", __VA_ARGS__, "\n"); exit(EXIT_FAILURE);}

#define log_output(...) (__printer__, __VA_ARGS__);

#define assert(this_is_true, ...) if(!(this_is_true)) {log_error(__VA_ARGS__);}/* exit(EXIT_FAILURE);} */

#endif //LOGGING
