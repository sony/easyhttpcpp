/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include "Poco/Path.h"

#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

#ifdef _WIN32
#define EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS  0
#define EASYHTTPCPP_FILE_PERMISSION_OTHERS_EXECUTE_ONLY 1
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY 2
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_WRITE_ONLY 3
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_EXECUTE_ONLY 4
#define EASYHTTPCPP_FILE_PERMISSION_DISABLE_READ_WITE EASYHTTPCPP_FILE_PERMISSION_OTHERS_EXECUTE_ONLY
#else
#define EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS  (S_IRWXU | S_IRWXG | S_IRWXO)
#define EASYHTTPCPP_FILE_PERMISSION_OTHERS_EXECUTE_ONLY S_IXOTH
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY (S_IRUSR | S_IRGRP | S_IROTH)
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_WRITE_ONLY (S_IWUSR | S_IWGRP | S_IWOTH)
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_EXECUTE_ONLY (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

class EASYHTTPCPP_TESTUTIL_API TestFileUtil {
public:
    static void changeAccessPermission(const Poco::Path& absolutePath, unsigned int mode);
    static void setReadOnly(const Poco::Path& path);
    static void setFullAccess(const Poco::Path& path);
    static void appendLongPathDir(Poco::Path& path);
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED */
