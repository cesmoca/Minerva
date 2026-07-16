#include <Kernel/EndController.h>
#include <Kernel/Logger.h>
#include <Kernel/Parsers/MSLPreprocessor.h>

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int preprocessFile(const std::string& path) {
    std::stringstream output;
    MSLPreprocessor preprocessor;
    preprocessor.start(path, output);

    std::cout << "--- Preprocessed MSL ---\n";
    std::cout << output.str();
    std::cout << "--- End MSL ---\n";
    return 0;
}

int runCoreSmokeCheck() {
    Logger::getInstance()->out("Starting Minerva dependency-free smoke check.");

    EndController* endController = EndController::getInstance();
    if (!endController->isRunning()) {
        Logger::getInstance()->error(
            "EndController did not start in the running state.");
        return 1;
    }

    endController->end();
    if (endController->isRunning()) {
        Logger::getInstance()->error(
            "EndController did not transition to the stopped state.");
        return 1;
    }

    Logger::getInstance()->out(
        "Core smoke check passed: logging, lifecycle control, and the "
        "MSL preprocessor compile successfully.");
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    int result = 0;

    try {
        if (argc == 1) {
            result = runCoreSmokeCheck();
        } else if (argc == 2) {
            result = preprocessFile(argv[1]);
        } else {
            std::cerr << "Usage: minerva_smoke [source.mrv]\n";
            result = 2;
        }
    } catch (const std::string& error) {
        Logger::getInstance()->error(error);
        result = 1;
    } catch (const char* error) {
        Logger::getInstance()->error(error);
        result = 1;
    } catch (const std::exception& error) {
        Logger::getInstance()->error(error.what());
        result = 1;
    }

    EndController::destroy();
    Logger::destroy();
    return result;
}
