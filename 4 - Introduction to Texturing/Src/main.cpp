#include "SDL.h"
#include "ResourcePath.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "rapidxml/rapidxml.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "MaterialManager.h"
#include "Camera.h"

#include <memory>
#include <vector>
#include <fstream>

using namespace std;
using namespace rapidxml;

class Graphics;

unique_ptr<Graphics> g_graphics;
SDL_Surface* g_screen;

xml_document<> g_shaderDoc;
string g_shaderPath = "./Shaders/";
string g_shaderName = "shader_basic_NoTex.xml";
string g_scenePath = "./";
string g_sceneName = "Jazz.dae";

struct vertexDataTmp{
	glm::vec3 position;
};

enum eInputMode
{
    E_MODE_NONE = 0,
    E_MODE_STRAFE,
    E_MODE_FREELOOK,
    NUM_MODES
};

enum eDisplayOrientation
{
    E_ORIENTATION_PORTRAIT = 0,
    E_ORIENTATION_LANDSCAPE,
    NUM_ORIENTATIONS
};

eInputMode              g_mode = E_MODE_NONE;
eDisplayOrientation     g_displayOrientation = E_ORIENTATION_LANDSCAPE;

class Graphics {
private:
    SDL_Window*         m_window;
    int                 m_w, m_h;
    
    glm::mat4x4         m_MVPMatrix;
    
    SceneManager*       m_pSceneManager;
    MaterialManager*    m_pMaterialManager;
    float               lastTime;

public:
    
    Camera*         m_pCamera;
    
public:
    Graphics(SDL_Window* window) {       
        printf("Graphics->Graphics() constructor\n");
        m_window = window;
    }
    
    bool Init() {
        printf("Graphics->Init()\n");
        int successfullInit = GL_TRUE;
        
        // Set the viewport and camera properties:
        glm::mat4x4     orientationTransform;
        float           aspectRatio;
        if (g_displayOrientation == E_ORIENTATION_PORTRAIT)
        {
            SDL_GetWindowSize(m_window, &m_w, &m_h);
            aspectRatio = 9.0f/16.0f;
            orientationTransform = glm::mat4x4(1.0f);
        }
        if (g_displayOrientation == E_ORIENTATION_LANDSCAPE)
        {
            SDL_GetWindowSize(m_window, &m_h, &m_w);
            aspectRatio = 16.0f/9.0f;
            orientationTransform = glm::rotate(glm::mat4x4(1.0f), -90.0f, glm::vec3(0.0f,0.0f,1.0f));
        }
        
        glViewport ( 0, 0, m_w, m_h );
        
        m_pCamera = new Camera();
        m_pCamera->translateCamera(7.5f, 7.0f, 18.0f);
        m_pCamera->addYawRotation(30.0f);
        m_pCamera->addPitchRotation(-15.0f);
        m_pCamera->setOrientationTransform(orientationTransform);
        m_pCamera->setAspectRatio(aspectRatio);
        
        // Load All Shaders
        m_pMaterialManager = MaterialManager::GetInstance();
        successfullInit &= m_pMaterialManager->LoadShaders(g_shaderPath);
        
        // Load the Scene
        string dataPath = g_scenePath + g_sceneName;
        m_pSceneManager = new SceneManager(dataPath.c_str());
        
        // Set the background color
        glClearColor ( 0.25f, 0.25f, 0.25f, 0.0f );
        
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        
        glEnable(GL_BLEND);
        
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        
        return successfullInit;
    } 

    void Draw() {
        // uncomment for some wild colorful flashes:
        //glClearColor(rand() % 255 / 255.0f, rand() % 255 / 255.0f, rand() % 255 / 255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Compute a delta time:
        float secondsSinceStart = SDL_GetTicks() / 1000.0f;
        float dt = secondsSinceStart-lastTime;
        lastTime = secondsSinceStart;
        
        m_pSceneManager->draw(m_pCamera, dt);
        
        SDL_GL_SwapWindow(m_window);
    } 
    
};

void UpdateFrame(void* param) {
    //SDL_Log("UpdateFrame()");
    Graphics* graphics = (Graphics*)param;
    graphics->Draw();
}

int EventFilter(void* userdata, SDL_Event* event) {
    glm::vec4 touchVec(event->tfinger.x, event->tfinger.y, 0.0, 0.0f);
    glm::vec4 dTouchVec(event->tfinger.dx, event->tfinger.dy, 0.0, 0.0f);
    if (g_displayOrientation == E_ORIENTATION_LANDSCAPE)
    {
        glm::mat4 ViewTransform = glm::rotate(glm::mat4x4(1.0f), -90.0f, glm::vec3(0.0f,0.0f,1.0f));
        touchVec = ViewTransform * touchVec;
        dTouchVec = ViewTransform * dTouchVec;
    }
    
    switch (event->type) {
        case SDL_FINGERMOTION:
            if (g_mode == E_MODE_STRAFE) {
                g_graphics->m_pCamera->translateCamera(10.0f*dTouchVec.x, 0.0f, 10.0f*dTouchVec.y);
            }
            
            else if (g_mode == E_MODE_FREELOOK) {
                g_graphics->m_pCamera->addPitchRotation(-100.0f*dTouchVec.y);
                g_graphics->m_pCamera->addYawRotation(-100.0f*dTouchVec.x);
            }
            return 0;
            
        case SDL_FINGERDOWN:
            //SDL_Log("Finger Down");
            if (touchVec.x >= 0.5) {
                g_mode = E_MODE_FREELOOK;
            }
            else {
                g_mode = E_MODE_STRAFE;
            }
            
            return 0;
            
        case SDL_FINGERUP:
            //SDL_Log("Finger Up");
            g_mode = E_MODE_NONE;
            return 0;
    }
    return 1;
}


/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    /* initialize SDL, specifically its video subsystem, i.e. graphics: */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL. Exiting.\n");
        return 1;
    } else {
        SDL_Log("SDL is now initialized. Yay!\n");
    }
    
    if (g_displayOrientation == E_ORIENTATION_PORTRAIT)
        SDL_SetHint( "SDL_IOS_ORIENTATIONS", "Portrait" );
    if (g_displayOrientation == E_ORIENTATION_LANDSCAPE)
        SDL_SetHint( "SDL_IOS_ORIENTATIONS", "LandscapeRight" );
    
    /* obtain information from SDL about the current graphics display: */
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    
    /* tell SDL we're using OpenGL ES version 2 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    // Unfortunately AA through SDL is not working in OSX:
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    
    /* create window and graphics renderer/context */
    SDL_Window* window = SDL_CreateWindow(NULL, 0, 0, displayMode.w, displayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not initialize SDL Window. Exiting.\n");
        return 1;
    }
    
    auto gl = SDL_GL_CreateContext(window);
    g_screen = SDL_GetWindowSurface( window );
    
    g_graphics = unique_ptr<Graphics>(new Graphics(window));
    if (!g_graphics->Init()) {
        SDL_Log("Could not initialize OpenGL context. Exiting.\n");
        return 1;
    }
    
    /* set display callback to work with iOS animation, */
    /* using an iOS CADisplayLink (a timer object that allows your application to synchronize its drawing to the refresh rate of the display)  : */
    /*   1. the callback will act on which window?  the main SDL window we just created */
    /*   2. to be called after how many refresh frames?  every refresh frame of the display */
    /*   3. the callback is what function?  our UpdateFrame function */
    /*   4. the callback function receives what parameters? a pointer to our graphics object */
    SDL_iPhoneSetAnimationCallback(window, 1, UpdateFrame, g_graphics.get());
    SDL_iPhoneSetEventPump(SDL_TRUE);
    
    /* add a callback to be triggered when an event is added to the event queue: */
    SDL_AddEventWatch(EventFilter, NULL);
    
    SDL_Log("My resource path is %s", getResourcePath().c_str());
    
    // SDL Game Loop
    SDL_Event event;
    auto done = false;
    while (!done)
    {
        SDL_PumpEvents();
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;
                    
                case SDL_APP_DIDENTERFOREGROUND:
                    SDL_Log("SDL_APP_DIDENTERFOREGROUND");
                    break;
                    
                case SDL_APP_DIDENTERBACKGROUND:
                    SDL_Log("SDL_APP_DIDENTERBACKGROUND");
                    break;
                    
                case SDL_APP_LOWMEMORY:
                    SDL_Log("SDL_APP_LOWMEMORY");
                    break;
                    
                case SDL_APP_TERMINATING:
                    SDL_Log("SDL_APP_TERMINATING");
                    break;
                    
                case SDL_APP_WILLENTERBACKGROUND:
                    SDL_Log("SDL_APP_WILLENTERBACKGROUND");
                    break;
                    
                case SDL_APP_WILLENTERFOREGROUND:
                    SDL_Log("SDL_APP_WILLENTERFOREGROUND");
                    break;
                    
                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        {
                            SDL_Log("Window %d resized to %dx%d", event.window.windowID, event.window.data1, event.window.data2);
                            
                            break;
                        }
                    }
                }
            }
        }
    }
    
    SDL_GL_DeleteContext(gl);
    
    // Done! Close the window, clean-up and exit the program.
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
} 
