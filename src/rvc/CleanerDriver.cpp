#include "rvc/CleanerDriver.hpp"

namespace rvc {

CleanerDriver::CleanerDriver() : mode("off") {}

void CleanerDriver::initialize() {
    // TODO
}

void CleanerDriver::startCleaning() {
    mode = "normal";
}

void CleanerDriver::stopCleaning() {
    // TODO
}

void CleanerDriver::decideSetting(bool boost) {
    // TODO
}

}
