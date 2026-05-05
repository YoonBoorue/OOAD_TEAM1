#pragma once
#include <string>

namespace rvc {

class CleanerDriver {
private:
    bool status;
    std::string mode;

public:
    CleanerDriver();
    void initialize();
    void startCleaning();
    void stopCleaning();
    void decideSetting(bool boost);
};

}