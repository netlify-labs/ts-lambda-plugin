#pragma once

#include <aws/core/utils/Outcome.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <ts/ts.h>
#include <tscpp/api/Async.h>

#include "request.h"

namespace tsLambda
{

class InvokeAsync : public atscppapi::AsyncProvider
{
public:
    InvokeAsync(std::shared_ptr<Client> client, Request &request);
    ~InvokeAsync();

    void run();
    void wait();
    void cancel();

    int dispatch();
    std::ostringstream getResponseStream();

private:
    std::shared_ptr<Client> m_client;
    Request &m_request;
    InvokeOutcomeCallable m_outcome;

    TSCont m_continuation = nullptr;
    TSAction m_action = nullptr;
};
} // namespace tsLambda