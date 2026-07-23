#include <Controllers/InputEventController.h>
#include <Controllers/PhysicsController.h>
#include <Factories/TrackingMethodFactory.h>
#include <Factories/VideoFactory.h>
#include <Kernel/EndController.h>
#include <Kernel/Logger.h>
#include <Kernel/Parsers/MSLPreprocessor.h>
#include <Kernel/World.h>

#include <SDL_image.h>
#include <boost/filesystem.hpp>
#include <lib3ds/file.h>
#include <opencv2/core/version.hpp>
#include <zip.h>

#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string formatVersion(const SDL_version* version) {
    std::ostringstream output;
    output << static_cast<int>(version->major) << '.'
           << static_cast<int>(version->minor) << '.'
           << static_cast<int>(version->patch);
    return output.str();
}

std::string openGlString(GLenum name) {
    const GLubyte* value = glGetString(name);
    return value == nullptr
               ? std::string("unavailable")
               : std::string(reinterpret_cast<const char*>(value));
}

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

bool tryCamera() {
    try {
#ifdef WIN32
        const int cameraBackend = cv::CAP_DSHOW;
        Logger::getInstance()->out(
            "Opening camera device 0 through DirectShow.");
#else
        const int cameraBackend = cv::CAP_ANY;
#endif
        VideoFactory::getInstance()->addVideoSource(
            "smoke-camera", 0, cameraBackend);
        VideoSource& camera = VideoFactory::getInstance()->getMainCamera();
        cv::Mat* frame = camera.grabFrame();

        if (frame == nullptr || frame->empty()) {
            Logger::getInstance()->warning(
                "Camera device 0 opened, but did not return a frame.");
        } else {
            std::ostringstream message;
            message << "OpenCV camera device 0 returned " << frame->cols << 'x'
                    << frame->rows << " pixels.";
            Logger::getInstance()->out(message.str());
        }
        return true;
    } catch (const std::string& error) {
        Logger::getInstance()->warning(
            "Camera initialization is optional: " + error);
    } catch (const char* error) {
        Logger::getInstance()->warning(
            "Camera initialization is optional: " + std::string(error));
    } catch (const cv::Exception& error) {
        Logger::getInstance()->warning(
            "Camera initialization is optional: " +
            std::string(error.what()));
    }
    return false;
}

bool drawCameraFrame(const cv::Mat& frame) {
    if (frame.empty() || frame.depth() != CV_8U ||
        (frame.channels() != 3 && frame.channels() != 4)) {
        return false;
    }

    const cv::Mat contiguousFrame =
        frame.isContinuous() ? frame : frame.clone();
    const GLenum format =
        contiguousFrame.channels() == 4 ? GL_BGRA : GL_BGR;
    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, contiguousFrame.cols,
                 contiguousFrame.rows, 0, format, GL_UNSIGNED_BYTE,
                 contiguousFrame.data);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 0.0F);
    glVertex2f(0.0F, 0.0F);
    glTexCoord2f(1.0F, 0.0F);
    glVertex2f(1.0F, 0.0F);
    glTexCoord2f(1.0F, 1.0F);
    glVertex2f(1.0F, 1.0F);
    glTexCoord2f(0.0F, 1.0F);
    glVertex2f(0.0F, 1.0F);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDeleteTextures(1, &texture);
    return true;
}

int runRuntimeSmokeCheck(Uint32 maximumRuntimeMs = 0U) {
    Logger::getInstance()->out("Starting Minerva runtime smoke check.");

    World::getInstance()->initWorld(640, 480);
    SDL_WM_SetCaption("Minerva runtime smoke", nullptr);

    Logger::getInstance()->out(
        "SDL " + formatVersion(SDL_Linked_Version()) + ", SDL_image " +
        formatVersion(IMG_Linked_Version()) + ", SDL_mixer " +
        formatVersion(Mix_Linked_Version()) + ", and SDL_ttf " +
        formatVersion(TTF_Linked_Version()) + " are linked.");

    const int requestedImageCodecs = IMG_INIT_JPG | IMG_INIT_PNG;
    const int initializedImageCodecs = IMG_Init(requestedImageCodecs);
    if ((initializedImageCodecs & requestedImageCodecs) !=
        requestedImageCodecs) {
        Logger::getInstance()->warning(
            "SDL_image could not initialize every requested codec: " +
            std::string(IMG_GetError()));
    } else {
        Logger::getInstance()->out(
            "SDL_image initialized its JPEG and PNG codecs.");
    }

    Logger::getInstance()->out(
        "OpenGL vendor: " + openGlString(GL_VENDOR) + "; renderer: " +
        openGlString(GL_RENDERER) + "; version: " +
        openGlString(GL_VERSION) + '.');

    const cv::Mat identity = cv::Mat::eye(4, 4, CV_32F);
    if (identity.rows != 4 || identity.cols != 4 ||
        identity.at<float>(0, 0) != 1.0F) {
        throw std::runtime_error("OpenCV matrix initialization failed.");
    }
    Logger::getInstance()->out(
        "OpenCV " + std::string(CV_VERSION) +
        " initialized an in-memory matrix.");

    PhysicsController* physics = PhysicsController::getInstance();
    physics->initPhysics();
    physics->_world->setGravity(btVector3(0.0F, -9.8F, 0.0F));
    physics->_world->stepSimulation(1.0F / 60.0F);
    Logger::getInstance()->out(
        "Bullet initialized and stepped an empty dynamics world.");

    TrackingMethodFactory::getInstance()->getTrackingMethodARTK();
    Logger::getInstance()->out(
        "ARToolKit initialized with Minerva's default camera parameters.");

    Lib3dsFile* file3ds = lib3ds_file_new();
    if (file3ds == nullptr) {
        throw std::runtime_error("lib3ds could not allocate an empty model.");
    }
    lib3ds_file_free(file3ds);
    Logger::getInstance()->out(
        "lib3ds allocated and released an empty model.");

    Logger::getInstance()->out(
        "libzip " + std::string(zip_libzip_version()) +
        " is available; current directory is " +
        boost::filesystem::current_path().string() + '.');

    const bool cameraAvailable = tryCamera();
    InputEventController* input = InputEventController::getInstance();
    EndController* endController = EndController::getInstance();

    if (maximumRuntimeMs == 0U) {
        Logger::getInstance()->out(
            "Runtime smoke is active. Close the window or press Escape to "
            "exit.");
    } else {
        std::ostringstream message;
        message << "Runtime smoke is active for "
                << static_cast<double>(maximumRuntimeMs) / 1000.0
                << " seconds. Close the window or press Escape to exit "
                   "early.";
        Logger::getInstance()->out(message.str());
    }

    const Uint32 startTime = SDL_GetTicks();
    while (endController->isRunning()) {
        input->pollEvents();

        int keyCount = 0;
        const Uint8* keys = SDL_GetKeyState(&keyCount);
        if (keys != nullptr && SDLK_ESCAPE < keyCount &&
            keys[SDLK_ESCAPE] != 0) {
            endController->end();
        }

        const float phase =
            static_cast<float>(SDL_GetTicks() % 4000U) / 4000.0F;
        const float pulse =
            0.5F + 0.5F * std::sin(phase * 6.2831853071795864769F);
        glClearColor(0.04F, 0.08F + 0.12F * pulse,
                     0.14F + 0.18F * (1.0F - pulse), 1.0F);
        bool drewCameraFrame = false;
        if (cameraAvailable) {
            cv::Mat* frame =
                VideoFactory::getInstance()->getMainCamera().grabFrame();
            if (frame != nullptr) {
                drewCameraFrame = drawCameraFrame(*frame);
            }
        }
        if (!drewCameraFrame) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        SDL_GL_SwapBuffers();
        SDL_Delay(16);

        if (maximumRuntimeMs != 0U &&
            SDL_GetTicks() - startTime >= maximumRuntimeMs) {
            endController->end();
        }
    }

    Logger::getInstance()->out("Runtime smoke check completed successfully.");
    return 0;
}

Uint32 parseRuntimeDuration(const std::string& text) {
    std::size_t parsedCharacters = 0;
    const double seconds = std::stod(text, &parsedCharacters);
    if (parsedCharacters != text.size() || !std::isfinite(seconds) ||
        seconds < 0.001 ||
        seconds > 3600.0) {
        throw std::invalid_argument(
            "Runtime duration must be between 0.001 and 3600 seconds.");
    }
    return static_cast<Uint32>(seconds * 1000.0);
}

void destroyRuntimeSubsystems() {
    IMG_Quit();
    InputEventController::destroy();
    VideoFactory::destroy();
    TrackingMethodFactory::destroy();
    PhysicsController::destroy();
    World::destroy();
}

}  // namespace

int main(int argc, char* argv[]) {
    int result = 0;

    try {
        if (argc == 1) {
            result = runRuntimeSmokeCheck();
        } else if (argc == 2 && std::string(argv[1]) == "--runtime") {
            result = runRuntimeSmokeCheck();
        } else if (argc == 2 && std::string(argv[1]) == "--core") {
            result = runCoreSmokeCheck();
        } else if (argc == 3 && std::string(argv[1]) == "--runtime") {
            result = runRuntimeSmokeCheck(parseRuntimeDuration(argv[2]));
        } else if (argc == 2) {
            result = preprocessFile(argv[1]);
        } else {
            std::cerr
                << "Usage: minerva_smoke [--runtime [seconds] | --core | "
                   "source.mrv]\n";
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

    destroyRuntimeSubsystems();
    EndController::destroy();
    Logger::destroy();
    return result;
}
