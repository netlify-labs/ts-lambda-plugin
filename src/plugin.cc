#include <unordered_map>
#include <aws/core/Aws.h>

#include <ts/ts.h>
#include <ts/remap.h>
#include "tscpp/api/RemapPlugin.h"

#include "plugin.h"
#include "client.h"
#include "config.h"
#include "intercept.h"
#include "metrics.h"

namespace
{
static char DEFAULT_CONFIG[] = "ts_lambda.json";
RemapPlugin *remapPlugin;
} // namespace

namespace tsLambda
{
class Remap : public atscppapi::RemapPlugin
{
public:
    explicit Remap(void **instance_handle, const std::string &config_name = DEFAULT_CONFIG) : RemapPlugin(instance_handle), m_config(config_name)
    {
        for (auto &c : m_config.getClients())
        {
            m_clients[c.client_id] = std::make_shared<tsLambda::Client>(c.region, c.client_id, c.secret_token);
        }
    }

    Result
    doRemap(const Url &map_from_url, const Url &map_to_url, Transaction &transaction, bool &redirect) override
    {
        auto funs = m_config.getFunctions();
        auto search = funs.find(map_from_url.getPath());
        if (search != funs.end())
        {
            auto fn = search->second;
            auto &cl = m_clients[fn.client_id];
            if (cl == nullptr && fn.secret_token != std::nullopt && fn.region != std::nullopt)
            {
                cl = std::make_shared<tsLambda::Client>(fn.region.value(), fn.client_id, fn.secret_token.value());
            }

            if (cl != nullptr)
            {
                transaction.addPlugin(new Intercept(transaction, fn.arn, cl));
            }
        }

        return RESULT_DID_REMAP;
    }

private:
    Config m_config;
    std::unordered_map<std::string, std::shared_ptr<Client>> m_clients;
};
} // namespace tsLambda

TSReturnCode
TSRemapInit(TSRemapInterface *api_info ATSCPPAPI_UNUSED, char *errbuf ATSCPPAPI_UNUSED, int errbuf_size ATSCPPAPI_UNUSED)
{
    TSDebug(PLUGIN_NAME, "Init plugin");

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    if (tsLambda::initMetrics() == TS_ERROR)
    {
        return TS_ERROR;
    }

    return TS_SUCCESS;
}

void TSRemapDone(void)
{
    TSDebug(PLUGIN_NAME, "Terminate plugin");

    Aws::SDKOptions options;
    Aws::ShutdownAPI(options);
}

TSReturnCode
TSRemapNewInstance(int argc ATSCPPAPI_UNUSED, char *argv[] ATSCPPAPI_UNUSED, void **instance_handle, char *errbuf ATSCPPAPI_UNUSED,
                   int errbuf_size ATSCPPAPI_UNUSED)
{
    TSDebug(PLUGIN_NAME, "New Instance");

    if (argc == 0)
    {
        remapPlugin = new tsLambda::Remap(instance_handle);
    }
    else
    {
        remapPlugin = new tsLambda::Remap(instance_handle, argv[0]);
    }
    return TS_SUCCESS;
}