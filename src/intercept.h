#pragma once

#include <chrono>

#include <aws/core/utils/memory/stl/SimpleStringStream.h>
#include <tscpp/api/Async.h>
#include <tscpp/api/Headers.h>
#include <tscpp/api/InterceptPlugin.h>
#include <tscpp/api/Transaction.h>

#include "invoke.h"
#include "request.h"

using namespace atscppapi;

namespace tsLambda
{

class Intercept : public InterceptPlugin, public AsyncReceiver<InvokeAsync>
{
public:
    explicit Intercept(Transaction &transaction, std::string function_arn, std::shared_ptr<Client> client) : InterceptPlugin(transaction, InterceptPlugin::SERVER_INTERCEPT), m_function_arn(function_arn), m_client(client)
    {
        m_start = std::chrono::system_clock::now();
        m_request_body = Aws::MakeShared<Aws::StringStream>(ALLOCATION_TAG);
    }

    void consume(const std::string &data, InterceptPlugin::RequestDataType type) override;
    void handleInputComplete() override;
    void handleAsyncComplete(InvokeAsync &invoke_async) override;

private:
    std::shared_ptr<Aws::IOStream> m_request_body;
    Aws::String m_function_arn;
    std::shared_ptr<Client> m_client;

    std::chrono::system_clock::time_point m_start;
};
} // namespace tsLambda