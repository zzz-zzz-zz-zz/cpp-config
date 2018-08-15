#include "dmatter/config/config.hpp"


namespace dmatter {


Config::Config(const std::string& path, ConfigFlags f)
{
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    const char *prefix = "Config::Config(std::string, ConfigFlags) - ";
#endif

    path_ = path;
    save_on_exit_ = f & ConfigFlags::SaveOnExit;

    if (f & ConfigFlags::LoadFromFile) {
        if (!try_load(path) && !(f & ConfigFlags::CreateNewFile)) {
            DMATTER_CONFIG_THROW(ConfigException(std::string("File \"") + path_ + std::string("\" cannot be opened")));
        }
    } 
    if (!ok_ && (f & ConfigFlags::CreateNewFile)) {
        std::ifstream i(path_);

        if (ok_ = !i.good()) {
            // Attempt to create new file
            std::ofstream o(path_);
            ok_ = o.is_open();
            o.close();

            if (ok_) {
                // Deleting empty created file because it's not what we want to save
                if (remove(path_.c_str()) != 0) {
                    ok_ = false;
                    DMATTER_CONFIG_THROW(ConfigException("Cannot delete empty tmp file"));
                }
            }
        } else { // If LoadFromFile is 1 - we cannot reach this branch
            DMATTER_CONFIG_THROW(ConfigException(std::string("File \"") + path_ + std::string("\" exists but shouldn't")));
        }

#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        if (ok_) {
            std::cerr << prefix <<  "CreateNewFile - Success" << std::endl;
        } else {
            std::cerr << prefix << "CreateNewFile - Error(file must not exist)" << std::endl;
        }
#endif
    }

    if (!ok_) {
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        std::cerr << "Must specify LoadFromFile or CreateNewFile flag!" << std::endl;
#endif
        DMATTER_CONFIG_THROW(ConfigException("Must specify LoadFromFile or CreateNewFile flag!"));
    }

#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    if (ok_) {
        std::cerr << prefix << "Successfully loaded config(" << path << ")!" << std::endl;
    } else {
        std::cerr << prefix << "Error loading config(" << path << ")!" << std::endl;
    }
#endif
}


Config::Config(const char *path, ConfigFlags f):
    Config(std::string(path), f) {}

Config Config::load_or_init_with(const std::string& path, const nlohmann::json& j, bool save_on_exit)
{
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    const char *prefix = "Config::load_or_init_with(string, json) - ";
#endif

    Config cfg;
    cfg.path_ = path;
    cfg.save_on_exit_ = save_on_exit;

#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    std::cerr << prefix << "Trying to load from file(" << path << ")..." << std::endl;
#endif

    if (!cfg.try_load(path)) {
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        std::cerr << prefix << "try_load failed! Loading from j parameter and saving..." << std::endl;
#endif
        cfg.json_ = j;
        cfg.ok_ = true; // try_load turns ok_ to false but we're ok(save() don't work when ok_ is false)
        
        cfg.ok_ = cfg.save(); // ok_ will be real after save()
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        if (cfg.ok_) {
            std::cerr << prefix << "Saved in file loaded json from j parameted successfully!" << std::endl;
        } else {
            std::cerr << prefix << "Error saving loaded json from j parameter!" << std::endl;
        }
#endif
    }
    else {
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        std::cerr << prefix << "try_load successfull!" << std::endl;
#endif
        //DMATTER_CONFIG_THROW(ConfigException(std::string("File \"") + path + std::string("\" cannot be opened")))
    }

    return cfg;
}


bool Config::try_load(const std::string& path)
{
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    const char *prefix = "dmatter::Config::try_load(string) - ";
#endif
    std::ifstream i(path);

    if (ok_ = i.is_open()) {
        i >> json_;
    }
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    if (ok_) {
        std::cerr << prefix << "loaded from file(" << path << ")" << std::endl;
    } else {
        std::cerr << prefix << "no such file(" << path << ")" << std::endl;
    }
#endif

    return ok_;
}


bool Config::is_ok()
{
    return ok_;
}


bool Config::save()
{
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    const char *prefix = "Config::save() - ";
#endif

    if (ok_) {
        std::ofstream o(path_, std::ios::out);

        if (o.is_open()) {
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
            std::cerr << prefix << "Saved successfully" << std::endl;
#endif
            o << std::setw(4) << json_ << std::endl;
            return true;
        }
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
        std::cerr << prefix << "Error opening file \"" << path_ << "\"" << std::endl;
#endif
    }
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    std::cerr << prefix << "Errors occured :(" << std::endl;
#endif
    return false;
}


nlohmann::json& Config::operator() ()
{
    return json_;
}


Config::~Config()
{
#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    const char *prefix = "Config::~Config() - ";
#endif

    if (save_on_exit_) {
        save();
    }

#ifdef DMATTER_CONFIG__ENABLE_DEBUG
    std::cerr << prefix << "Object deleted" << std::endl;
#endif
}


#ifdef DMATTER_CONFIG__ENABLE_TESTS
namespace tests {


bool test_all()
{
    const char *prefix = "dmatter::tests::test_all() - ";
    int tests_passed = 0;
    const int tests_count = 2;

    std::cerr << prefix << "TESTS ARE ABOUT TO START" << std::endl;;

    if (test_create_and_load()) tests_passed++;
    if (test_load_or_init_with()) tests_passed++;

    if (tests_passed == tests_count) {
        std::cerr << prefix << "====[SUCCESS! " << tests_passed 
                    << "/" << tests_count << " TESTS ARE PASSED]====" << std::endl;
        return true;
    } else {
        std::cerr << prefix << "====[ERRORS! PASSED " << tests_passed << " of " 
                                << tests_count << " tests ]====" << std::endl;
        return false;
    }
}

bool test_create_and_load()
{
    const char *prefix = "dmatter::tests::test_create_and_load() - ";

    bool ok = false;
    const char *tmp_filename = ".DMATTER_CONFIG_test_create_and_load.tmp";

    std::cerr << prefix << "TEST STARTED" << std::endl << std::endl;

    try {
        std::cerr << prefix << "Creating empty new file without saving:" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::CreateNewFile);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
        }
        // file successfully created(but not saved) and Config destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Creating empty new file and saving:" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::CreateNewFile|ConfigFlags::SaveOnExit);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
        }
        // file successfully created and Config destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Loading created file:" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::LoadFromFile);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
        }
        // file successfully opened and Config destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Loading again(and saving with value):" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::LoadFromFile|ConfigFlags::SaveOnExit);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
            cfg()["test-passed"] = "25%";
        }
        // file successfully opened, rewritten with same content+NEW and destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Loading again(and saving with changed value):" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::LoadFromFile|ConfigFlags::SaveOnExit);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
            if (cfg()["test-passed"] != "25%") {
                DMATTER_CONFIG_THROW(ConfigException("TEST FAILED on 25%"));
                throw std::exception();
            }

            cfg()["test-passed"] = "50%";
        }
        // file successfully opened, rewritten with same content+NEW and destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Loading again(and saving with changed value one more time):" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::LoadFromFile|ConfigFlags::SaveOnExit);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
            if (cfg()["test-passed"] != "50%") {
                DMATTER_CONFIG_THROW(ConfigException("TEST FAILED on 50%"));
                throw std::exception();
            }

            cfg()["test-passed"] = "75%";
        }
        // file successfully opened, rewritten with same content+NEW and destructed
        std::cerr << "OK!" << std::endl << std::endl;

        std::cerr << prefix << "Loading again(and saving with changed value even one more time):" << std::endl;
        { 
            Config cfg(tmp_filename, ConfigFlags::LoadFromFile|ConfigFlags::SaveOnExit);
#ifndef DMATTER_CONFIG__ENABLE_THROW
            if (!cfg.is_ok()) throw std::exception();
#endif
            if (cfg()["test-passed"] != "75%") {
                DMATTER_CONFIG_THROW(ConfigException("TEST FAILED on 75%"));
                throw std::exception();
            }

            cfg()["test-passed"] = "100%";
        }
        // file successfully opened, rewritten with same content+NEW and destructed
        std::cerr << "OK!" << std::endl << std::endl;

        ok = true;

    } catch (ConfigException& e) {
        std::cerr << prefix << "Expected an exception " << e.what() << std::endl;
        ok = false;
    } catch (nlohmann::json::exception& e) {
        std::cerr << prefix << "Expected an json exception " << e.what() << std::endl;
        ok = false;
    } catch (std::exception& e) {
        std::cerr << prefix << "Expected an exception " << e.what() << std::endl;
        ok = false;
    }

    if (remove(tmp_filename) != 0) { // We'll attemp to delete anyway
        if (ok) { // If it supposed to be created but errors - show message
            std::cerr << prefix <<  "Cannot delete tmp file \"" << tmp_filename << "\"" << std::endl;
        }
        ok = false;
    }

    std::cerr << prefix << (ok? "TEST PASSED":"TEST FAILED") << std::endl;

    return ok;
}

bool test_load_or_init_with()
{
    const char *prefix = "dmatter::tests::test_load_or_init_with - ";
    bool ok;
    const char* tmp_filename = ".DMATTER_CONFIG_test_load_or_init_with.tmp";

    const char* dummy_key = "DMATTER-dummy-key";
    const char* dummy_value = "DMATTER-dummy-value";

    std::cerr << prefix << "TEST STARTED" << std::endl;

    try {
        
        std::cerr << prefix << "Init from json(file must not exist):" << std::endl;
        {
            auto cfg = Config::load_or_init_with(tmp_filename, [&]() {
                nlohmann::json j;

                j[dummy_key] = dummy_value;

                return j;
            }());

            if (cfg()[dummy_key] != dummy_value) {
                DMATTER_CONFIG_THROW(ConfigException("Error getting value"));
                throw std::exception();
            }
        }
        std::cerr << "OK" << std::endl << std::endl;

        std::cerr << prefix << "Loading from file(must exist):" << std::endl;
        {
            auto cfg = Config::load_or_init_with(tmp_filename, nlohmann::json());

            if (cfg()[dummy_key] != dummy_value) {
                DMATTER_CONFIG_THROW(ConfigException("Error loading from file"));
                throw std::exception();
            }
        }
        std::cerr << "OK" << std::endl << std::endl;

        ok = true;
    } catch (ConfigException& e) {
        std::cerr << prefix << "Expected an exception " << e.what() << std::endl;
        ok = false;
    } catch (nlohmann::json::exception& e) {
        std::cerr << prefix << "Expected an json exception " << e.what() << std::endl;
        ok = false;
    } catch (std::exception& e) {
        std::cerr << prefix << "Expected an exception " << e.what() << std::endl;
        ok = false;
    }

    if (remove(tmp_filename) != 0) { // We'll attemp to delete anyway
        if (ok) { // If it supposed to be created but errors - show message
            std::cerr << prefix <<  "Cannot delete tmp file \"" << tmp_filename << "\"" << std::endl;
        }
        ok = false;
    }

    std::cerr << prefix << (ok? "TEST PASSED":"TEST FAILED") << std::endl;

    return ok;
}


} // namespace tests
#endif // DMATTER_CONFIG__ENABLE_TESTS


} // namespace dmatter
