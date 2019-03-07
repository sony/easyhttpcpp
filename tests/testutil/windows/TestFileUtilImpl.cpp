/*
 * Copyright 2017 Sony Corporation
 */
 
#include "easyhttpcpp/common/CoreLogger.h"
#include "TestFileUtil.h"

#include "TestFileUtilImpl.h"

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestFileUtilImpl";

void TestFileUtilImpl::changeAccessPermission(const std::string& absolutePath, unsigned int mode)
{
    switch (mode) {
        case EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS:
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_ALL_ACCESS, SET_ACCESS);
            break;
        case EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY:
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_READ, SET_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_WRITE, DENY_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_EXECUTE, DENY_ACCESS);
            break;
        case EASYHTTPCPP_FILE_PERMISSION_ALLUSER_WRITE_ONLY:
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_WRITE, SET_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_READ, DENY_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_EXECUTE, DENY_ACCESS);
            break;
        case EASYHTTPCPP_FILE_PERMISSION_ALLUSER_EXECUTE_ONLY:
        case EASYHTTPCPP_FILE_PERMISSION_DISABLE_READ_WITE:
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_EXECUTE, SET_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_READ, DENY_ACCESS);
            changePermission(_T(absolutePath.c_str()), _T("Users"), FILE_GENERIC_WRITE, DENY_ACCESS);
            break;
        default:
            break;
    }
}
void TestFileUtilImpl::changePermission(LPCTSTR pPath, LPCTSTR pUser, DWORD permission, ACCESS_MODE mode)
{
    const int TRUSTEE_NAME_LEN = 256;
    TCHAR pathBuff[MAX_PATH + 1] = { 0 };
    _tcsncpy(pathBuff, pPath, MAX_PATH);

    TCHAR nameBuff[TRUSTEE_NAME_LEN + 1] = { 0 };
    _tcsncpy(nameBuff, pUser, TRUSTEE_NAME_LEN);

    DWORD result;

    // ファイルに設定されているアクセス権についての情報を取得.
    PACL pAcl = NULL;
    PSECURITY_DESCRIPTOR pDescriptor = NULL;

    result = GetNamedSecurityInfo(pathBuff,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        0,
        0,
        &pAcl,
        0,
        &pDescriptor);
    if (result != ERROR_SUCCESS) {
        EASYHTTPCPP_LOG_D(Tag, "Can not get named security info");
        return;
    }

    // 追加するアクセス権を作成
    EXPLICIT_ACCESS access;
    BuildExplicitAccessWithName(&access,
        nameBuff,
        permission,
        mode, // SET_ACCESS, DENY_ACCESS, GRANT_ACCESS
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);

    // アクセス権を追加.
    pAcl = addACLExplicitAccess(pAcl, access);
    if (pAcl == NULL) {
        EASYHTTPCPP_LOG_E(Tag, "Can not add permission");
        ::LocalFree(pDescriptor);
        return;
    }
    // アクセス権をファイルに設定
    if (!setDACL(pathBuff, pAcl)) {
        EASYHTTPCPP_LOG_E(Tag, "Can not set named security info");
        ::LocalFree(pAcl);
        ::LocalFree(pDescriptor);
        return;
    }

    ::LocalFree(pAcl);
    ::LocalFree(pDescriptor);
    return;
}

PACL TestFileUtilImpl::addACLExplicitAccess(PACL pAclOld, EXPLICIT_ACCESS& access) {
    PACL pAclNew = NULL;
    const DWORD result = SetEntriesInAcl(1, &access, pAclOld, &pAclNew);
    if (result != ERROR_SUCCESS) {
        return NULL;
    }
    return pAclNew;
}

bool TestFileUtilImpl::setDACL(LPTSTR pPath, PACL pAcl) {
    const DWORD result = SetNamedSecurityInfo(pPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, pAcl, 0);
    return result == ERROR_SUCCESS;
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
