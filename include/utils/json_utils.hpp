#pragma once

#include <memory>
#include <nlohmann/json.hpp>

struct JsonConfig
{
public:
    virtual ~JsonConfig() = default;
    virtual std::shared_ptr<const JsonConfig> clone() const = 0;

protected:
    JsonConfig() = default;
    virtual void loadFromJson(const nlohmann::json &data) = 0;
};