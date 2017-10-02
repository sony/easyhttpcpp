/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED

namespace easyhttpcpp {
namespace testutil {

#define EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS  (S_IRWXU | S_IRWXG | S_IRWXO)
#define EASYHTTPCPP_FILE_PERMISSION_OTHERS_EXECUTE_ONLY S_IXOTH
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY (S_IRUSR | S_IRGRP | S_IROTH)
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_WRITE_ONLY (S_IWUSR | S_IWGRP | S_IWOTH)
#define EASYHTTPCPP_FILE_PERMISSION_ALLUSER_EXECUTE_ONLY (S_IXUSR | S_IXGRP | S_IXOTH)

class TestFileUtil {
public:

    static void changeAccessPermission(const std::string& absolutePath, unsigned int mode)
    {
        chmod(absolutePath.c_str(), mode);
    }
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTFILEUTIL_H_INCLUDED */
