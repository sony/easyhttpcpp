/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED

namespace easyhttpcpp {
namespace testutil {

class TestFileUtilImpl {
public:

    static void changeAccessPermission(const std::string& absolutePath, unsigned int mode);

};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED */
