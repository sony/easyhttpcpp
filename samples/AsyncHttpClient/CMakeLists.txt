set(SAMPLE_NAME "${CMAKE_PROJECT_NAME}-AsyncHttpClient")

set(SAMPLE_SRCS "AsyncHttpClient.cpp")

# dependent libraries
find_package(Poco REQUIRED Foundation Data DataSQLite Net NetSSL Crypto CONFIG)
find_package(easyhttpcpp REQUIRED CONFIG)

add_executable(${SAMPLE_NAME} ${SAMPLE_SRCS})
target_link_libraries(${SAMPLE_NAME}
                      easyhttpcpp::easyhttp
                      Poco::Foundation
                      Poco::Data Poco::DataSQLite
                      Poco::Net Poco::NetSSL Poco::Crypto
                      OpenSSL::SSL OpenSSL::Crypto
                      )
