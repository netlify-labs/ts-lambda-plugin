#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/base64/Base64.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <ts/ts.h>

#include "client.h"
#include "http.h"
#include "invoke.h"

namespace tsLambda
{

int handleInvokeAsyncEvent(TSCont cont, TSEvent event, void *edata)
{
    InvokeAsync *async = static_cast<InvokeAsync *>(TSContDataGet(cont));
    async->wait();

    if (!async->dispatch())
    {
        delete async;
    }
    return 0;
}

InvokeAsync::InvokeAsync(std::shared_ptr<Client> client, Request &request) : m_client(client), m_request(request)
{
    m_continuation = TSContCreate(tsLambda::handleInvokeAsyncEvent, TSMutexCreate());
    TSContDataSet(m_continuation, static_cast<void *>(this));
}

InvokeAsync::~InvokeAsync()
{
    cancel();
}

void InvokeAsync::run()
{
    m_outcome = m_client->invoke(m_request);

#if TS_VERSION_MAJOR >= 9
    m_action = TSContScheduleOnPool(m_continuation, m_client->getTimeout(), TS_THREAD_POOL_TASK);
#else
    m_action = TSContSchedule(m_continuation, m_client->getTimeout(), TS_THREAD_POOL_TASK);
#endif
}

void InvokeAsync::wait()
{
    m_outcome.wait();
}

int InvokeAsync::dispatch()
{
    return getDispatchController()->dispatch();
}

void InvokeAsync::cancel()
{
    TSCont contp{m_continuation};
    if (contp != nullptr)
    {
        return;
    }

    auto mutex{TSContMutexGet(contp)};
    TSMutexLock(mutex); // prevent event dispatch for the continuation during this cancel.

    if (m_action != nullptr)
    {
        TSActionCancel(m_action);
    }

    m_continuation = nullptr;

    TSMutexUnlock(mutex);
    TSContDestroy(contp);
}

std::ostringstream InvokeAsync::getResponseStream()
{
    auto outcome = m_outcome.get();

    std::ostringstream oss("HTTP/1.1 ");

    if (outcome.IsSuccess())
    {
        auto result = outcome.GetResultWithOwnership();
        auto response = Aws::Utils::Json::JsonValue(result.GetPayload()).View();
        int status = response.ValueExists("statusCode") ? response.GetInteger("statusCode") : 502;

        auto reason = result.GetFunctionError();
        if (reason.empty())
        {
            reason = httpReasonPhrase(status);
            oss << " " << reason;
        }
        oss << status << " " << reason << "\r\n";

        if (response.ValueExists("headers"))
        {
            auto headers = response.GetObject("headers");
            for (auto &&header : headers.GetAllObjects())
            {
                oss << header.first << ": " << header.second.AsString() << "\r\n";
            }
        }
        oss << "\r\n";

        if (response.ValueExists("body"))
        {
            auto body = response.GetString("body");
            if (response.GetBool("isBase64Encoded"))
            {
                Aws::Utils::Base64::Base64 decoder;
                oss << decoder.Decode(body).GetUnderlyingData();
            }
            else
            {
                oss << body;
            }
        }
    }
    else
    {
        const auto error = outcome.GetError();

        int status = static_cast<int>(error.GetResponseCode());
        oss << status << " " << httpReasonPhrase(status) << "\r\n";

        for (auto &&header : error.GetResponseHeaders())
        {
            oss << header.first << ": " << header.second << "\r\n";
        }
        oss << "\r\n"
            << error.GetExceptionName()
            << "\r\n"
            << error.GetMessage();
    }

    return oss;
}

} // namespace tsLambda