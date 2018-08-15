#ifndef DMATTER_CONFIG_H
#define DMATTER_CONFIG_H

#define DMATTER_CONFIG__VERSION_MAJOR 1
#define DMATTER_CONFIG__VERSION_MINOR 0
// #define DMATTER_CONFIG__VERSION_PATCH 0 // May be not defined

#if __cplusplus >= 201402L
    // #define DMATTER_CONFIG__CPP14DEFINED // for feature [deprecated] use in future
#endif

/// ========================= Main compile options ================================== ///
// #define DMATTER_CONFIG__ENABLE_DEBUG // define to receive std::cerr debug messages
#define DMATTER_CONFIG__ENABLE_THROW // define to receive exceptions(slower but safer)
#define DMATTER_CONFIG__ENABLE_TESTS // define to enable tests in dmatter::tests namespace 
/// ========================= ==== ======= ======= ================================== ///

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    #define DMATTER_CONFIG_DEBUG(code) if (true) { code }
#else
    #define DMATTER_CONFIG_DEBUG(code) if (false) { code }
#endif

#ifdef DMATTER_CONFIG__ENABLE_THROW
    #include <exception>

    #define DMATTER_CONFIG_THROW(exception) { throw exception; }
#else
    #define DMATTER_CONFIG_THROW(exception) ;
#endif

#include <stdio.h>


#include "nlohmann/json.hpp"


namespace dmatter {

/**
 * Flags to specify config behavior
 * 
 * @since 1.0
 */
enum ConfigFlags {
    /**
     * Save preferences to file when ~Config called
     */
    SaveOnExit                          = 0x01,

    /**
     * Load config from file.
     * If config cannot be loaded from file and CreateNewFile not also specified
     * (file not exist or previleges error) - It will be unable to properly use config object
     */
    LoadFromFile                        = 0x02,

    /**
     * Create empty config and write to new file.
     * If file with same name exists - it will not be rewritten(e.g. settings will be lost)
     * 
     */
    CreateNewFile                       = 0x08,

    /**
     * Open file if exist, otherwise create new file.
     */ 
    LoadFileIfExist                     = 0x08 | 0x02

    // TODO: ReadOnly attribute(File must exist, saves not 
                    // allowed, no ability to set parameters - only read)

    // TODO: NoFile mode(No file path, only local 
                    // json file(maybe copy from another json), no save allowed)
};

inline ConfigFlags operator| (ConfigFlags a, ConfigFlags b)
{
    return static_cast<ConfigFlags>(static_cast<int>(a) | static_cast<int>(b));
}
inline ConfigFlags operator& (ConfigFlags a, ConfigFlags b)
{
    return static_cast<ConfigFlags>(static_cast<int>(a) & static_cast<int>(b));
}

class ConfigException: public std::exception
{
public:
    ConfigException(std::string message, const char *file, const char *function, int line):
        message_(std::string("ConfigException(file: ") + std::string(file) 
                    + std::string(", function: ") + std::string(function)
                    + std::string(", line: ") + std::to_string(line)
                    + std::string(") - ") + message) {}

    ConfigException(std::string message):
            message_(std::string("ConfigException: ") + message) {}

    const char* what() const throw()
    {
        return message_.c_str();
    }
protected:
    std::string prefix_;

private:
    std::string message_;
};

class Config {
public:

    /**
     * Creates config object from json file `path`.
     * @param path          - Path to the json file
     * @param f             - Config options(flags)
     * 
     * @since 1.0
     */
    Config(const std::string& path, ConfigFlags f);

    /**
     * See Config(const std::string, ConfigFlags f)
     * 
     * @since 1.0
     */
    Config(const char *path, ConfigFlags f);

    /**
     * Returns config object filled with content of file specified in path or
     * with nlohmann::json object
     * @param path - path to config file
     * @param j - json object which use if file not exists
     * @param save_on_exit - do need save when ~Config() called
     * 
     * @return Config object
     * 
     * @since 1.0
     */
    static Config load_or_init_with(const std::string& path, const nlohmann::json& j, bool save_on_exit=true);

    Config(const Config&) = default;
    Config(Config&&) = default;

    ~Config();

    /**
     * @return Is config successfully loaded
     * 
     * @since 1.0
     */
    bool is_ok();

    /**
     * Attempt to save the file
     * @return Is json successfully saved
     * 
     * @since 1.0
     */
    bool save();

    /**
     * Get reference to json object
     * ((WARNING: Reference expires after object deletion))
     * 
     * @return reference to json object
     * 
     * @since 1.0
     */
    nlohmann::json& operator() ();

    /**
     *  Get config version string
     * 
     *  @return version string
     * 
     *  @since 1.0
     */ 
    static std::string version_string() {
        std::string ver;

        ver += std::to_string(DMATTER_CONFIG__VERSION_MAJOR);
        ver += std::string(".") + std::to_string(DMATTER_CONFIG__VERSION_MINOR);
#ifdef DMATTER_CONFIG__VERSION_PATCH
        ver += std::string(".") + std::to_string(DMATTER_CONFIG__VERSION_PATCH);
#endif // DMATTER_CONFIG__VERSION_PATCH

        return ver;
    }
    
private:
    Config() {} // Used factory function
    bool try_load(const std::string& path);

    std::string     path_;          // path to config file
    nlohmann::json  json_;          // json object with config parameters loaded
    bool            ok_;            // is json_ valid
    bool            save_on_exit_;  // do call save() in destructor?
};

#ifdef DMATTER_CONFIG__ENABLE_TESTS
namespace tests {


bool test_all();
bool test_create_and_load();
bool test_load_or_init_with();


} // namespace tests
#endif // DMATTER_CONFIG__ENABLE_TESTS

} // namespace dmatter

#endif // DMATTER_CONFIG_H
