#pragma once

#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/Scheme.h>
#include <aws/core/platform/Platform.h>
#include <aws/core/utils/ratelimiter/DefaultRateLimiter.h>
#include <aws/core/utils/ratelimiter/RateLimiterInterface.h>
#include <aws/lambda/LambdaClient.h>

#include "request.h"

using namespace Aws::Auth;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Lambda;
using namespace Aws::Lambda::Model;
using namespace Aws::Utils::RateLimits;

namespace tsLambda
{
static const char ALLOCATION_TAG[] = "TSLambda";
static const char USER_AGENT[] = "trafficserver/lambda";

class Client
{
public:
    explicit Client(const std::string &region, const std::string &client_id, const std::string &secret_token, int timeout = 60000) : m_timeout(timeout)
    {
        ClientConfiguration config;

        config.region = region;
        config.userAgent = USER_AGENT;
        config.scheme = Scheme::HTTPS;
        config.connectTimeoutMs = timeout;
        config.requestTimeoutMs = timeout;

        AWSCredentials creds;
        creds.SetAWSAccessKeyId(Aws::String(client_id));
        creds.SetAWSSecretKey(Aws::String(secret_token));

        m_client = Aws::MakeShared<LambdaClient>(ALLOCATION_TAG, creds, config);
    }

    inline InvokeOutcomeCallable invoke(const Request &request) const
    {
        return m_client->InvokeCallable(request);
    }

    inline int getTimeout() const
    {
        return m_timeout;
    }

private:
    std::shared_ptr<LambdaClient> m_client;
    int m_timeout;
};
} // namespace tsLambda