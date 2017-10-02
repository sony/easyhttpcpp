/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEUPDATECHECKPOLICY_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEUPDATECHECKPOLICY_H_INCLUDED

namespace easyhttpcpp {

/*
 * @enum HttpCacheUpdateCheckPolicy HttpCacheUpdateCheckPolicy.h "easyhttpcpp/HttpCacheUpdateCheckPolicy.h"
 * 
 * Defines the policy to be used when a network request couldn't be made to check for updates to the cached data.
 * This maybe due to network error, server being down or any other error.
 */
enum HttpCacheUpdateCheckPolicy {
    /**
     * Directs the Http to return the cached data (if available) when a network request couldn't be made to
     * check for updates to the cached data. The cached data might not be the same as the latest data present on the
     * remote server.
     */
    HttpCacheUpdateCheckPolicyReturnCacheOnError,
    /**
     * Directs the Http to abort when a network request couldn't be made to check for updates to the
     * cached data.
     */
    HttpCacheUpdateCheckPolicyAbortOnError
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEUPDATECHECKPOLICY_H_INCLUDED */
