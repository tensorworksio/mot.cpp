#pragma once

#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>

inline bool isFileAccessible(const std::string &filename)
{
    if (filename.empty())
        return false;
    std::ifstream file(filename);
    return file.good();
}

struct JsonConfig
{
public:
    virtual ~JsonConfig() = default;
    virtual std::shared_ptr<const JsonConfig> clone() const = 0;

protected:
    JsonConfig() = default;
    virtual void loadFromJson(const nlohmann::json &data) = 0;
};