/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CRLCHECKPOLICY_H_INCLUDED
#define EASYHTTPCPP_CRLCHECKPOLICY_H_INCLUDED

namespace easyhttpcpp {

enum CrlCheckPolicy {
    CrlCheckPolicyNoCheck = 0,
    CrlCheckPolicyCheckSoftFail,
    CrlCheckPolicyCheckHardFail
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CRLCHECKPOLICY_H_INCLUDED */
