<pre>
&#035;include "SDL.h"
&#035;include "ResourcePath.h"

&#035;include &ltOpenGLES/ES2/gl.h&gt
&#035;include &ltOpenGLES/ES2/glext.h&gt

&#035;include &ltmemory&gt
using namespace std;

<b>SDL_TouchFingerEvent myFinger;</b>

class Graphics {
private:
   SDL_Window*     m_window;
    
   GLuint          m_shaderProgram;
    
   ///
    // Create a shader object, load the shader source, and
    // compile the shader.
    //
   GLuint LoadShader ( GLenum type, const char *shaderSrc ) {
        printf("Graphics->LoadShader()\n");
        GLuint shader;
        GLint compiled;
        
        // Create the shader object
        shader = glCreateShader ( type );
        
        if ( shader == 0 )
            return 0;
        
        // Load the shader source
        glShaderSource ( shader, 1, &shaderSrc, NULL );
        
        // Compile the shader
        glCompileShader ( shader );
        
        // Check the compile status
        glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );
        
        if ( !compiled ) {
            GLint infoLen = 0;
       
        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
            
        if ( infoLen > 1 ) {
            char* infoLog = (char *)malloc (sizeof(char) * infoLen );
                
            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            printf ( "Error compiling shader:\n%s\n", infoLog );
                
            free ( infoLog );
       }
       glDeleteShader ( shader );
       return 0;
    }
    return shader;
}
    
public:
   Graphics(SDL_Window* window) {  
        printf("Graphics->Graphics() constructor\n");
        m_window = window;
   }
    
   bool Init() {
    printf("Graphics->Init()\n");
    GLbyte vShaderStr[] =
	   "attribute vec4 a_Position;                   \n"
	<b>   "uniform   mat4 u_MVP_Matrix;                 \n"</b>
	   "void main()                                  \n"
	   "{                                            \n"
	<b>   "   gl_Position = u_MVP_Matrix * vec4(a_Position.xyz, 1.0);  \n"</b>
	   "}                                            \n";
        
    GLbyte fShaderStr[] =
       "precision mediump float;\n"\
       "void main()                                  \n"
	   "{                                            \n"
<b>       "   gl_FragColor = vec4 ( 1.0, 0.3255, 0.298, 1.0 );\n"</b>
	   "}                                            \n";
        
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint  linked;
        
        // Load the vertex/fragment shaders
        vertexShader = LoadShader ( GL_VERTEX_SHADER, (const char *)vShaderStr );
        fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, (const char *)fShaderStr );
        
        // Create the program object
        programObject = glCreateProgram ( );
        
        if ( programObject == 0 )
            return 0;
        
        glAttachShader ( programObject, vertexShader );
        glAttachShader ( programObject, fragmentShader );
        
        // Bind a_Position to attribute 0   -->  what is a_Position ?
        glBindAttribLocation ( programObject, 0, "a_Position" );
        
        // Link the program
        glLinkProgram ( programObject );
        
        // Check the link status
        glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );
        
        if ( !linked ) {
            GLint infoLen = 0;
            
            glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
            
            if ( infoLen > 1 )
            {
                char* infoLog = (char *)malloc (sizeof(char) * infoLen );
                
                glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
                printf ( "Error linking program:\n%s\n", infoLog );
                
                free ( infoLog );
            }
            
            glDeleteProgram ( programObject );
            return false;
        }
        
        // Store the program object
        m_shaderProgram = programObject;
        
        // Set the viewport
        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);
        SDL_Log("w = %d, h = %d\n", w, h);
        glViewport ( 0, 0, w, h );
        
        // Set the background color
        glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
        
        return true;
    } /* bool Init() */

    void Draw() {
<b>    glClearColor(0.286, 0.6, 0.5294, 1);</b>
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
**
GLfloat vVertices[] = {30.0,  80.0, 0.0,
                       20.0,  60.0, 0.0,
                       60.0,  80.0, 0.0,
                       50.0,  60.0, 0.0};

//there must be a better way than this
float centerX = (30.0 + 20.0 + 60.0 + 50.0)/4.0;
float centerY = (80.0 + 60.0 + 80.0 + 60.0)/4.0;
float normalizedCX = centerX / 320.0;
float normalizedCY = centerY / 480.0;

SDL_Log("avg x = %f",normalizedCX);
SDL_Log("avg y = %f",normalizedCY);

SDL_Log("can I print myFinger pos? %f, %f", myFinger.x, myFinger.y);

 //  what could be vMVPMatrix ?
 GLfloat vMVPMatrix[16] = {
       0.00625,                         0.0,                             0.0,  0.0, //scale by 2/width
       0.0,                            -0.004166666,                     0.0,  0.0, //scale by -2/height
       0.0,                             0.0,                             1.0,  0.0,
      -1.0+(myFinger.x-normalizedCX)*2, 1.0-(myFinger.y-normalizedCY)*2, 0.0,  1.0
 };
**
        // Use the program object
        glUseProgram ( m_shaderProgram );
<b>
        //  what would these be ?
        GLint MVP_Location = glGetUniformLocation(m_shaderProgram, "u_MVP_Matrix");
        glUniformMatrix4fv(MVP_Location, 1, false, vMVPMatrix);
</b>        
        // Load the vertex data
        glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
        glEnableVertexAttribArray ( 0 );
        
        <b>glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 );</b>
        
        SDL_GL_SwapWindow(m_window);
    } 
    
}; 

void UpdateFrame(void* param) {
    Graphics* graphics = (Graphics*)param;
    graphics->Draw();
}

int EventFilter(void* userdata, SDL_Event* event) {
    SDL_Log("EventFilter()");
    <b>
    myFinger= (*event).tfinger;
    
    switch (event->type) {
        case SDL_FINGERMOTION: {
            //SDL_Log("Finger Moved. Current x = %f, current y = %f",myFinger.x, myFinger.y);
            //Graphics::x = myFinger.x;
            //Graphics::y = myFinger.y;
            return 0;}
            
        case SDL_FINGERDOWN: {
            SDL_Log("Finger Down at (%f,%f)",myFinger.x, myFinger.y );
            return 0;}
            
        case SDL_FINGERUP: {
            SDL_Log("Finger Up");
            return 0;}
    } 
    </b>
    return 1;
}

int main(int argc, char *argv[]) {
    /* initialize SDL, specifically its video subsystem, i.e. graphics: */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL. Exiting.\n");
        return 1;
    } else {
        SDL_Log("SDL is now initialized. Yay!\n");
    }
    
    /* obtain information from SDL about the current graphics display: */
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    
    /* tell SDL we're using OpenGL ES version 2 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    
    /* create window and graphics renderer/context */
    SDL_Window* window = SDL_CreateWindow(NULL, 0, 0, displayMode.w, displayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not initialize SDL Window. Exiting.\n");
        return 1;
    }
    auto gl = SDL_GL_CreateContext(window);
    unique_ptr<Graphics> graphics = unique_ptr<Graphics>(new Graphics(window));
    if (!graphics->Init()) {
        SDL_Log("Could not initialize OpenGL context. Exiting.\n");
        return 1;
    }
    
    /* set display callback to work with iOS animation, */
    /* using an iOS CADisplayLink (a timer object that allows your application to synchronize its drawing to the refresh rate of the display)  : */
    /*   1. the callback will act on which window?  the main SDL window we just created */
    /*   2. to be called after how many refresh frames?  every refresh frame of the display */
    /*   3. the callback is what function?  our UpdateFrame function */
    /*   4. the callback function receives what parameters? a pointer to our graphics object */
    SDL_iPhoneSetAnimationCallback(window, 1, UpdateFrame, graphics.get());
    
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
