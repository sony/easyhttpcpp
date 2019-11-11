# Change Log

## [2.1.0](https://github.com/sony/easyhttpcpp/releases/tag/2.1.0) (Nov 12, 2019)
#### Added
Features added:
- Improved Windows support
- Support for cache database corruption listener
- Minor bug fixes
- More tests

## [2.0.0](https://github.com/sony/easyhttpcpp/releases/tag/2.0.0) (March 07, 2019)
#### Added
Features added:
- Windows support
- Async APIs and corresponding sample project, AsyncHttpClient
- `executorservice/QueuedThreadPool` for thread pooling (used by Async APIs)
- RequestBody::create(MediaType::Ptr, Poco::SharedPtr<std::string>)
- RequestBody::create(MediaType::Ptr, Poco::SharedPtr<std::istream>)
#### Changed
- `common/CoreLogger::getInstance()` now returns pointer instead of reference
#### Deprecated
- `RequestBody::create(MediaType::Ptr, std::istream&)`
  - Use `RequestBody::create(MediaType::Ptr, Poco::SharedPtr<std::istream>)` instead.
- `RequestBody::create(MediaType::Ptr, const std::string&)`
  - Use `RequestBody::create(MediaType::Ptr, Poco::SharedPtr<std::string>)` instead.
#### Removed
N/A
#### Fixed
- https://github.com/sony/easyhttpcpp/issues/1
- https://github.com/sony/easyhttpcpp/issues/2

## [1.0.0](https://github.com/sony/easyhttpcpp/releases/tag/1.0.0) (October 05, 2017)
#### Added
Initial release. Features added:
- Http/Https client with REST support
- Http response cache
- Http connection pooling
- Interceptors
- Integration and Unit tests
- Doxygen api doc
- Tested on Linux(Ubuntu) & OSX
#### Changed
N/A
#### Deprecated
N/A
#### Removed
N/A
#### Fixed
N/A
