#include "rvc/CleanerDriver.hpp"

namespace rvc
{

    CleanerDriver::CleanerDriver()
        : mode("off") {}

    void CleanerDriver::initialize() { mode = "off"; }

    void CleanerDriver::startCleaning() { mode = "normal"; }

    void CleanerDriver::stopCleaning() { mode = "off"; }

    void CleanerDriver::decideSetting(bool boost) { mode = boost ? "boost" : "normal"; }

    bool CleanerDriver::isCleaning() const { return mode != "off"; }

    const std::string &CleanerDriver::currentMode() const { return mode; }

}