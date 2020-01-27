#pragma once

#include <aws/core/Aws.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <tscpp/api/Headers.h>

using namespace atscppapi;
using namespace Aws::Lambda::Model;

namespace tsLambda
{
class Request : public InvokeRequest
{
public:
    Request(Headers &headers) : InvokeRequest(), m_headers(headers)
    {
    }

    inline Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override
    {
        auto headers = InvokeRequest::GetRequestSpecificHeaders();

        for (auto &&iter : m_headers)
        {
            HeaderField hf = iter;
            headers.emplace(hf.name().str(), hf.values());
        }

        return headers;
    }

private:
    atscppapi::Headers &m_headers;
};

} // namespace tsLambda