#pragma once

#include <fstream>
#include <unordered_map>

#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <ts/ts.h>

namespace tsLambda
{

struct FunctionConfig
{
    std::string arn;
    std::string client_id;
    std::optional<std::string> secret_token;
    std::optional<std::string> region;
};

struct ClientConfig
{
    std::string region;
    std::string client_id;
    std::string secret_token;
};

class Config
{
public:
    explicit Config(const std::string &config_name)
    {
        if (!config_name.empty())
        {
            const std::string config_path = TSConfigDirGet() + config_name;
            Aws::IFStream ifs(config_path);

            auto doc = Aws::Utils::Json::JsonValue(ifs).View();

            if (doc.ValueExists("clients"))
            {
                const auto clients = doc.GetArray("clients");
                for (unsigned idx = 0; idx < clients.GetLength(); ++idx)
                {
                    const auto v = clients.GetItem(idx);
                    if (!v.IsObject())
                    {
                        continue;
                    }

                    const auto obj = v.AsObject();

                    m_clients.push_back({obj.GetString("region").c_str(),
                                         obj.GetString("client_id").c_str(),
                                         obj.GetString("secret_token").c_str()});
                }
            }

            if (doc.ValueExists("functions"))
            {
                const auto functions = doc.GetArray("functions");
                for (unsigned idx = 0; idx < functions.GetLength(); ++idx)
                {
                    const auto v = functions.GetItem(idx);
                    if (!v.IsObject())
                    {
                        continue;
                    }

                    const auto obj = v.AsObject();
                    const auto path = obj.GetString("path").c_str();

                    m_functions[path].arn = obj.GetString("arn").c_str();
                    m_functions[path].client_id = obj.GetString("client_id").c_str();
                    if (obj.ValueExists("secret_token"))
                    {
                        m_functions[path].secret_token = obj.GetString("secret_token").c_str();
                    }

                    if (obj.ValueExists("region"))
                    {
                        m_functions[path].region = obj.GetString("region").c_str();
                    }
                }
            }
        }
    }

    inline std::unordered_map<std::string, FunctionConfig> getFunctions()
    {
        return m_functions;
    }

    inline std::vector<ClientConfig> getClients()
    {
        return m_clients;
    }

private:
    std::vector<ClientConfig> m_clients;
    std::unordered_map<std::string, FunctionConfig> m_functions;
};
} // namespace tsLambda