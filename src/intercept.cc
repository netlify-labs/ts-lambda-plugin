
#include <chrono>

#include <aws/core/Aws.h>
#include <aws/core/utils/memory/stl/SimpleStringStream.h>
#include <aws/lambda/model/InvokeRequest.h>

#include <tscpp/api/Async.h>
#include <tscpp/api/InterceptPlugin.h>

#include "client.h"
#include "intercept.h"
#include "metrics.h"

namespace tsLambda
{
void Intercept::consume(const std::string &data, InterceptPlugin::RequestDataType type)
{
    if (type == InterceptPlugin::REQUEST_BODY)
    {
        *m_request_body << data;
    }
}

void Intercept::handleInputComplete()
{
    Request request(getRequestHeaders());
    request.SetFunctionName(m_function_arn);
    request.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
    request.SetBody(m_request_body);

    InvokeAsync *invoke_async = new InvokeAsync(m_client, request);
    atscppapi::Async::execute<InvokeAsync>(this, invoke_async, getMutex());
}

void Intercept::handleAsyncComplete(InvokeAsync &invoke_async)
{
    auto response = invoke_async.getResponseStream();
    produce(response.str());
    setOutputComplete();

    const auto end = std::chrono::system_clock::now();
    const auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

    tsLambda::measureLatency(latency.count());
    tsLambda::incrementInvocations();
}
} // namespace tsLambda