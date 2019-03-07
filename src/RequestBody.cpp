/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/RequestBody.h"

#include "RequestBodyForByteBuffer.h"
#include "RequestBodyForSharedPtrByteBuffer.h"
#include "RequestBodyForSharedPtrStream.h"
#include "RequestBodyForSharedPtrString.h"
#include "RequestBodyForStream.h"
#include "RequestBodyForString.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {

RequestBody::RequestBody(MediaType::Ptr pMediaType) : m_pMediaType(pMediaType)
{
}

RequestBody::~RequestBody()
{
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, Poco::SharedPtr<std::istream> pContent)
{
    return new RequestBodyForSharedPtrStream(pMediaType, pContent);
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, Poco::SharedPtr<std::string> pContent)
{
    return new RequestBodyForSharedPtrString(pMediaType, pContent);
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, Poco::SharedPtr<ByteArrayBuffer> pContent)
{
    return new RequestBodyForSharedPtrByteBuffer(pMediaType, pContent);
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, std::istream& content)
{
    return new RequestBodyForStream(pMediaType, content);
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, const std::string& content)
{
    return new RequestBodyForString(pMediaType, content);
}

RequestBody::Ptr RequestBody::create(MediaType::Ptr pMediaType, const easyhttpcpp::common::ByteArrayBuffer& content)
{
    return new RequestBodyForByteBuffer(pMediaType, content);
}

MediaType::Ptr RequestBody::getMediaType() const
{
    return m_pMediaType;
}

} /* namespace easyhttpcpp */
