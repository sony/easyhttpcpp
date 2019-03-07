/*
 * Copyright 2019 Sony Corporation
 */

#include <iostream>
#include <stdexcept>

#include "Poco/Event.h"
#include "easyhttpcpp/EasyHttp.h"

void displayUsage(char** argv)
{
    std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
    std::cout << "       Fetches the resource identified by <url> and prints it to the standard output"
              << std::endl;
}

void dumpResponse(easyhttpcpp::Response::Ptr pResponse)
{
    std::cout << "Http status code: " << pResponse->getCode() << std::endl;
    std::cout << "Http status message: " << pResponse->getMessage() << std::endl;
    std::cout << "Http response headers:\n" << pResponse->getHeaders()->toString() << std::endl;

    // dump response body if text
    const std::string contentType = pResponse->getHeaderValue("Content-Type", "");
    if (Poco::isubstr<std::string>(contentType, "text/html") != std::string::npos) {
        std::cout << "Http response body:\n" << pResponse->getBody()->toString() << std::endl;
    }
}

class HttpClientCallback : public easyhttpcpp::ResponseCallback {
public:
    typedef Poco::AutoPtr<HttpClientCallback> Ptr;

    virtual ~HttpClientCallback() {};

    // called when response was returned by the remote server.
    void onResponse(easyhttpcpp::Response::Ptr pResponse) {
        if (!pResponse->isSuccessful()) {
            std::cout << "HTTP GET Error: (" << pResponse->getCode() << ")" << std::endl;
        } else {
            std::cout << "HTTP GET Success!" << std::endl;
        }

        // dump response
        dumpResponse(pResponse);

        m_completionWaiter.set();
    }

    // called when any error occurred in connecting with remote server or any other internal error.
    void onFailure(easyhttpcpp::HttpException::Ptr pWhat) {
        // some error occurred; might be due to network error etc.
        // for more information about the error, use pWhat->getCause() (can be NULL as well)
        std::cout << "Error occurred: " << pWhat->what() << std::endl;

        m_completionWaiter.set();
    }

    bool waitForCompletion() {
        return m_completionWaiter.tryWait(10 * 1000 /* milliseconds*/);
    }

private:
    Poco::Event m_completionWaiter;
};

int main(int argc, char** argv)
{
    // need a url to execute easyhttpcpp http client
    if (argc < 2) {
        displayUsage(argv);
        return 1;
    }
    std::string url = argv[1];

    // HTTP GET the url
    std::cout << "HTTP GET url: " << url << std::endl;

    try {
        // cache dir = current working dir; cache size = 100 KB
        easyhttpcpp::HttpCache::Ptr pCache = easyhttpcpp::HttpCache::createCache(Poco::Path::current(), 1024 * 100);

        // a default http connection pool
        easyhttpcpp::ConnectionPool::Ptr pConnectionPool = easyhttpcpp::ConnectionPool::createConnectionPool();

        // configure http cache and connection pool instance (optional but recommended)
        easyhttpcpp::EasyHttp::Builder httpClientBuilder;
        httpClientBuilder.setCache(pCache)
                .setConnectionPool(pConnectionPool);

        // create http client
        easyhttpcpp::EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

        // create a new request and execute asynchronously
        easyhttpcpp::Request::Builder requestBuilder;
        easyhttpcpp::Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        easyhttpcpp::Call::Ptr pCall = pHttpClient->newCall(pRequest);

        // actual download will be executed on a worker thread inside a threadpool
        HttpClientCallback::Ptr pResponseCb = new HttpClientCallback();
        pCall->executeAsync(pResponseCb);

        // wait for response
        if(!pResponseCb->waitForCompletion()) {
            std::cout << "Could not get response in time." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error occurred: " << e.what() << std::endl;
    }

    return 0;
}
