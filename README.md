# TrafficServer AWS Lambda Plugin

This is a plugin for [Apache TrafficServer](https://trafficserver.apache.org) that allows you to use TrafficServer as proxy/cache for AWS Lambda invocations.

## Requirements

This plugin requires you to have the [AWS SDK CPP](https://github.com/aws/aws-sdk-cpp/) installed in development and production machines.

This plugin also requires Traffic Server 8.0.x or above installed in development and production machines.

## Building

There is a very simple Makefile that compiles a dynamically linked object that you can deploy on Traffic Server. Run `make` to compile this plugin from the root directory of this repository.

## Configuration

This plugin is enabled as a Remap plugin, so you need to add it to your `remap.config` file:

```
map http://localhost http://localhost \
    @plugin=ts-lambda-plugin.so @pparam=lambda-config.json
```

To map urls with AWS Lambda functions, you have to create a configuration file in JSON format that includes functions and credentials mappings. You can see an example of this file in [config-example.json](config-example.json).

The configuration file allows you to set different client credentials if you have functions segregated in several AWS accounts. To map a function to a client, you need to set the same `client_id` option, and the plugin will invoke the function in the right account:

```json
{
    "clients": [
        {
            "region": "us-east-1",
            "client_id": "aws_access_key_1",
            "secret_token": "aws_secret_code_1"
        },
        {
            "region": "us-west-1",
            "client_id": "aws_access_key_2",
            "secret_token": "aws_secret_code_2"
        }
    ],
    "functions": [
        {
            "path": "/hello-world",
            "arn": "aws_function_arn",
            "client_id": "aws_access_key_1"
        },
        {
            "path": "/goodbye-world",
            "arn": "aws_function_arn",
            "client_id": "aws_access_key_2"
        }
    ]
}
```