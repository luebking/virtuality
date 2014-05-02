#include <QString>
#include "revision.h"

const QString virtuality_revision()
{
#define STR(str) #str
#define STRING(str) STR(str)
    return STRING(GIT_REVISION);
}