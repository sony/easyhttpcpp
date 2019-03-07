/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <AclAPI.h>
#include <TCHAR.h>

namespace easyhttpcpp {
namespace testutil {

class TestFileUtilImpl {
public:

    static void changeAccessPermission(const std::string& absolutePath, unsigned int mode);

private:
    static void changePermission(LPCTSTR pPath, LPCTSTR pUser, DWORD permission, ACCESS_MODE mode);
    static PACL addACLExplicitAccess(PACL pAclOld, EXPLICIT_ACCESS& access);
    static bool setDACL(LPTSTR pPath, PACL pAcl);
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTFILEUTILIMPL_H_INCLUDED */
