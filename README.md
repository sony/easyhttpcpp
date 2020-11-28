EasyHttp
============

[![Job Status](https://inspecode.rocro.com/badges/github.com/sony/easyhttpcpp/status?token=ylDg4TQtxeggrtYDr-GP3NMzKW3J0EUw6bLJdTyNVso)](https://inspecode.rocro.com/jobs/github.com/sony/easyhttpcpp/latest?completed=true)
[![Build Status](https://travis-ci.org/sony/easyhttpcpp.svg?branch=master)](https://travis-ci.org/sony/easyhttpcpp)
[![Build status](https://ci.appveyor.com/api/projects/status/a8bike297ad96dsy/branch/master?svg=true&passingText=Windows%20OK&failingText=Windows%20NG)](https://ci.appveyor.com/project/shekharhimanshu/easyhttpcpp-45djp/branch/master)

A cross-platform HTTP client library with a focus on usability and speed.
Under its hood, EasyHttp uses [POCO C++ Libraries](https://github.com/pocoproject/poco) and derives many of its 
design inspirations from [okHttp](https://github.com/square/okhttp), a well known HTTP client for Android and Java 
applications. Please check out [Wiki](https://github.com/sony/easyhttpcpp/wiki) for details.

Why another HTTP client?
--------------------------
Modern network applications need a powerful HTTP client. While we already have many well known C++ HTTP clients like, 
[Poco::Net](https://pocoproject.org/docs/Poco.Net.html), 
[Boost.Asio](http://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio.html), 
[cpprestsdk](https://github.com/Microsoft/cpprestsdk) to name a few, they often lack features like a powerful 
response cache, HTTP connection pooling, debuggability etc which we all take for granted for libraries targeted 
towards Android or iOS platforms. EasyHttp tries to fill that gap. 

Features
----------------
- Powerful and easy to use HTTP client with synchronous and asynchronous apis.
- HTTP connection pooling support to reduce latency.
- HTTP response caching to optimize repeat requests.
- Hackable HTTP request/response with a concept of [okHttp](https://github.com/square/okhttp) style 
[Interceptors](https://github.com/sony/easyhttpcpp/wiki/Recipe:-Interceptors).
- Cross-platform and highly portable to [many platforms](https://github.com/sony/easyhttpcpp/wiki/Supported-platforms).
- Automatic recovery during faulty network connections.
- Secure by default. Obsolete protocols like SSLv2, SSLv3 are disabled by default.
- Comprehensibly tested and is used internally in various Sony projects.

Getting Started
-----------------
#### Installation
See [Installing EasyHttp](https://github.com/sony/easyhttpcpp/wiki/Installing-EasyHttp).

#### Samples
See [samples](https://github.com/sony/easyhttpcpp/tree/master/samples).
Also checkout common [use-cases](https://github.com/sony/easyhttpcpp/wiki/Recipes).

#### Api doc
See [wiki](https://github.com/sony/easyhttpcpp/wiki/Build-options#building-docs) for building api doc.

#### Library build options
See [Build options](https://github.com/sony/easyhttpcpp/wiki/Build-options).

License
---------
This library is distributed under [The MIT license](https://opensource.org/licenses/MIT). 
See [LICENSE](https://github.com/sony/easyhttpcpp/blob/master/LICENSE) and
[NOTICE](https://github.com/sony/easyhttpcpp/blob/master/NOTICE) for more information.
