#include "access_desc.hh"

void access_desc::message(int code, ...)
{
    va_list ap;
    va_start(ap, code);
    format_message(code, self_class->source_file, line, ap);
    va_end(ap);
}

