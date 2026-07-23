#include <MPY/MPYWrapper.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

TEST(PythonIntegrationTest, CompilesScriptsAndExposesEngineObjects) {
    MPYWrapper::getInstance()->initPython();

    const std::filesystem::path scriptPath =
        std::filesystem::current_path() / "bundle-l-script.py";
    {
        std::ofstream script(scriptPath);
        ASSERT_TRUE(script.is_open());
        script << "bundle_l_value = 42\n";
    }

    MAORenderable3DLine& parent =
        MAOFactory::getInstance()->addMAORenderable3DLine(
            "python-parent", 1.0F, 10, 20, 30);
    MLBControllerScript controller(
        "script", parent, scriptPath.generic_string());
    ASSERT_NE(controller.getCompiledObj(), nullptr);
    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLERSCRIPT);

    MGEModule module;
    module.setCurrentController(&controller);
    EXPECT_FALSE(module.mPyGetCurrentController().is_none());
    EXPECT_FALSE(module.mPyGetMAO(parent.getName()).is_none());

    PyObject* mainModule = PyImport_AddModule("__main__");
    ASSERT_NE(mainModule, nullptr);
    PyObject* globals = PyModule_GetDict(mainModule);
    ASSERT_NE(globals, nullptr);
    PyObject* value =
        PyRun_String("MGE_PLAY", Py_eval_input, globals, globals);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(PyLong_AsLong(value), 0);
    Py_DECREF(value);

    std::filesystem::remove(scriptPath);
}
