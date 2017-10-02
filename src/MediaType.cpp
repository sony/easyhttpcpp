/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/MediaType.h"

namespace easyhttpcpp {

static const std::string Tag = "MediaType";

MediaType::MediaType(const std::string& contentType)
{
    m_contentType = contentType;
}

MediaType::~MediaType()
{
}

std::string MediaType::toString()
{
    return m_contentType;
}

} /* namespace easyhttpcpp */
