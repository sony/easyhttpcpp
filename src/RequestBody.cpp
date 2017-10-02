/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/RequestBody.h"

#include "RequestBodyForByteBuffer.h"
#include "RequestBodyForStream.h"
#include "RequestBodyForString.h"

namespace easyhttpcpp {

RequestBody::~RequestBody()
{
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

void RequestBody::setMediaType(MediaType::Ptr pMediaType)
{
    m_pMediaType = pMediaType;
}

} /* namespace easyhttpcpp */
