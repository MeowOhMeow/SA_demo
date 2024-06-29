// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <thread>
#include <chrono>
#include <iostream>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "api.h"
#include "utility.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char **)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    bool show_results_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float targetFrameRate = 60.0f;
    auto targetDuration = std::chrono::milliseconds(int(1e3 / targetFrameRate));
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    API::Parameters parameters;
    std::thread task_thread;

    ImVector<char *> filenames = getFilenamesInDirC("./testcases");

    int my_image_width = 0;
    int my_image_height = 0;
    GLuint my_image_texture = 0;
    bool ret = false;

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        lastFrameTime = std::chrono::high_resolution_clock::now();

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Show the control panel
        {
            ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("Control Panel"); // Create a window called "Control Panel" and append into it.

            ImGui::Text("Application average (%.1f FPS)", io.Framerate);

            ImGui::Checkbox("Show Library Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Show Results Window", &show_results_window);

            // ImGui::ProgressBar(0.42f, ImVec2(0.0f, 0.0f));
            // ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            // ImGui::Text("Progress Bar");

            // ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Processing...");
            // ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            // ImGui::Text("Thread Status");

            // ImGui::ProgressBar(-1.0f, ImVec2(0.0f, 0.0f), "Idling...");
            // ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            // ImGui::Text("Test Bar");

            static int item_current = 0;
            if (ImGui::BeginCombo("Input File", parameters.inputFile))
            {
                for (int n = 0; n < filenames.Size; n++)
                {
                    bool is_selected = (n == item_current);
                    if (ImGui::Selectable(filenames[n], is_selected))
                    {
                        // clear the old path
                        memset(parameters.inputFile, 0, IM_ARRAYSIZE(parameters.inputFile));
                        // copy the dir + new path
                        strcpy(parameters.inputFile, "./testcases/");
                        strcat(parameters.inputFile, filenames[n]);
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::InputText("Output File", parameters.outputFile, IM_ARRAYSIZE(parameters.outputFile));
            ImGui::InputDouble("Temperature", &parameters.temperature);
            ImGui::InputDouble("Cooling Rate", &parameters.coolingRate);
            ImGui::InputDouble("Absolute Temperature", &parameters.absoluteTemperature);
            ImGui::InputInt("Target Iterations", &parameters.targetIterations);
            ImGui::SameLine();
            HelpMarker("Set to 0 to run until the absolute temperature is reached.");

            static int status = 0;
            static bool completed = false;

            if (ImGui::Button("Run"))
            {
                if (!API::task_running)
                {
                    if (task_thread.joinable())
                    {
                        task_thread.join();
                    }
                    task_thread = std::thread(API::run, parameters);
                    status = 1;
                    completed = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                API::task_cancel = true;
                status = 0;
            }

            if (API::task_running)
            {
                if (API::task_done)
                {
                    API::task_running = false;
                    completed = true;
                    char buffer[256];
                    // copy from the parameters
                    strcpy(buffer, parameters.outputFile);
                    // replace the extension with png
                    char *dot = strrchr(buffer, '.');
                    if (dot)
                    {
                        strcpy(dot, ".png");
                    }
                    else
                    {
                        strcat(buffer, ".png");
                    }
                    ret = LoadTextureFromFile(buffer, &my_image_texture, &my_image_width, &my_image_height);
                    IM_ASSERT(ret);
                    if (task_thread.joinable())
                    {
                        task_thread.join();
                    }
                }
                else
                {
                    status = 1;
                }
            }
            else
            {
                status = 0;
            }

            ImGui::ProgressBar(API::task_progress.load(), ImVec2(0.0f, 0.0f));
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("Progress Bar");
            switch (status)
            {
            case 0:
                ImGui::ProgressBar(-1.0f, ImVec2(0.0f, 0.0f), "Idling...");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Thread Status");
                break;
            case 1:
                ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Running...");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Thread Status");
                break;

            default:
                ImGui::ProgressBar(-1.0f, ImVec2(0.0f, 0.0f), "Error!");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Thread Status");
                break;
            }

            if (API::error_message)
            {
                ImGui::Text("Error: %s", API::error_message.load());
            }
            else if (completed)
            {
                ImGui::Text("Completed!");
            }

            ImGui::End();
        }

        if (show_results_window)
        {
            ImGui::SetNextWindowSize(ImVec2(1040, 790), ImGuiCond_FirstUseEver);

            if (ImGui::Begin("Results"))
            {
                if (ret)
                {
                    ImGui::Image((void *)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
                }
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        // Sleep to maintain target frame rate
        auto frameDuration = std::chrono::high_resolution_clock::now() - lastFrameTime;
        auto sleepDuration = targetDuration - frameDuration;
        if (sleepDuration > std::chrono::milliseconds(0))
        {
            std::this_thread::sleep_for(sleepDuration);
        }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
