#include "SDL.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>

using namespace std;

class Graphics;
unique_ptr<Graphics> g_graphics;

SDL_Surface* g_screen;

struct vertexData {
    glm::vec3 position;
};

enum eEditMode {
    E_MODE_INSERT = 0,
    E_MODE_EDIT,
    E_MODE_TRANSFORM,
    NUM_MODES
};

eEditMode g_mode = E_MODE_INSERT;

class Graphics {

private:
    SDL_Window*     m_window;
    int             m_w, m_h;
    
    GLuint          m_shaderProgram;
    
    glm::mat4x4     m_MVPMatrix;
    
public:
    std::vector<vertexData>     m_vertices;
    std::vector<vertexData *>   m_pVertices;

    glm::vec3                   m_fingerDragOffset;
    
    std::vector<vertexData>     m_touchZoneVertices;
    float                       m_touchZoneHeight;
    
private:
    /* ------------------------------ */
    /* Create a shader object, load the shader source, and compile the shader. */
    /* ------------------------------ */
    GLuint LoadShader ( GLenum type, const char *shaderSrc ) {
        SDL_Log("Graphics->LoadShader()\n");

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
                SDL_Log ( "Error compiling shader:\n%s\n", infoLog );
                
                free ( infoLog );
            }
            
            glDeleteShader ( shader );
            return 0;
        }
        
        return shader;
    } /* LoadShader() */
    
public:
    Graphics(SDL_Window* window) {
        SDL_Log("Graphics->Graphics() constructor\n");
        m_window = window;
    }
    
    /* initialize the Graphics class */
    bool Init() {
        SDL_Log("Graphics->Init()\n");
        
        GLbyte vShaderStr[] =
        "attribute vec4 a_Position;                   \n"
        "uniform   mat4 u_MVP_Matrix;                 \n"
        "void main()                                  \n"
        "{                                            \n"
        "                                           \n"
        "   gl_Position = u_MVP_Matrix * vec4(a_Position.xyz, 1.0);  \n"
        "   gl_PointSize = 10.0;\n"
        "}                                            \n";

        GLbyte fShaderStr[] =
        "precision mediump float;                       \n"
        "uniform vec4 u_fragmentColor;                  \n"
        "void main()                                    \n"
        "{                                              \n"
        "   gl_FragColor = u_fragmentColor;              \n"
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
                SDL_Log ( "Error linking program:\n%s\n", infoLog );
                
                free ( infoLog );
            }
            
            glDeleteProgram ( programObject );
            return false;
        }
        
        // Store the program object
        m_shaderProgram = programObject;
        
        // Set the viewport
        
        SDL_GetWindowSize(m_window, &m_w, &m_h);
        glViewport ( 0, 0, m_w, m_h );
        
        // Set the background color
        glClearColor ( 0.25f, 0.25f, 0.25f, 0.0f );
        
        // Init Vertex List
        m_vertices.clear();
        vertexData tmp;
        tmp.position = glm::vec3(0.5f*m_w,0.5f*m_h,0.0f);
        m_vertices.push_back(tmp);
        
        // Init Touch Zone
        m_touchZoneHeight = 0.1;
        
        m_fingerDragOffset = glm::vec3(0.0f);
        tmp.position = glm::vec3(0.0f,m_h*(1.0f- m_touchZoneHeight),0.0f);
        m_touchZoneVertices.push_back(tmp);
        tmp.position = glm::vec3(m_w,m_h*(1.0f - m_touchZoneHeight),0.0f);
        m_touchZoneVertices.push_back(tmp);
        tmp.position = glm::vec3(m_w,m_h,0.0f);
        m_touchZoneVertices.push_back(tmp);
        tmp.position = glm::vec3(0.0f,m_h,0.0f);
        m_touchZoneVertices.push_back(tmp);
        
        return true;
    } 
    
    void Draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        vertexData lCentroid = getCentroid();
        
        m_MVPMatrix = glm::mat4x4(1.0f); // Load Identity
        glm::mat4x4 lProjMatrix = glm::ortho(0.0f, (float)m_w, 0.0f, (float)m_h, -1.0f, 1.0f);

        if(g_mode == E_MODE_TRANSFORM) {
            float phi = 0;
            //if(glm::abs(m_fingerDragOffset.z) > glm::abs(yDistance)) {
            //    phi = glm::acos(yDistance / m_fingerDragOffset.z);
            //}
            float dr = glm::distance(m_fingerDragOffset.x, m_fingerDragOffset.y);
            float R = 10.0f;
            //phi = asin(dr / glm::sqrt(R*R+dr*dr));
            //phi = acos(R  / glm::sqrt(R*R+dr*dr));
            phi = dr/R;
            glm::mat4x4 TransMatrix = glm::mat4x4(1.0,                  0.0,                 0.0,0.0,
                                      0.0,                  1.0,                 0.0,0.0,
                                      0.0,                  0.0,                 1.0,0.0,
                                      lCentroid.position.x, lCentroid.position.y,0.0,1.0);
            glm::mat4x4 InvTransMatrix = glm::mat4x4(1.0,                   0.0,                 0.0,0.0,
                                                     0.0,                   1.0,                 0.0,0.0,
                                                     0.0,                   0.0,                 1.0,0.0,
                                                     -lCentroid.position.x, -lCentroid.position.y,0.0,1.0);
            glm::mat4x4 RotMatrix = glm::mat4x4(glm::cos(phi), -glm::sin(phi), 0.0,0.0,
                                                glm::sin(phi), glm::cos(phi), 0.0,0.0,
                                                0.0,           0.0,           1.0,0.0,
                                                0.0,           0.0,           0.0,1.0);
            m_MVPMatrix = lProjMatrix * TransMatrix * RotMatrix * InvTransMatrix; //* RotMatrix * TransMatrix;
        }
        else {
            m_MVPMatrix = lProjMatrix;
        }

        // Use the program object
        glUseProgram ( m_shaderProgram );

        GLint MVP_Location = glGetUniformLocation(m_shaderProgram, "u_MVP_Matrix");
        glUniformMatrix4fv(MVP_Location, 1, false, glm::value_ptr(m_MVPMatrix));
        
        GLint fragmentColor = glGetUniformLocation(m_shaderProgram, "u_fragmentColor");
        glUniform4f(fragmentColor,1.0,0.0,0.0,1.0);
        
        // Load the vertex data
        glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, &m_vertices[0] );
        glEnableVertexAttribArray ( 0 );
        
        glDrawArrays ( GL_POINTS, 0, m_vertices.size() );
        glDrawArrays ( GL_LINE_LOOP, 0, m_vertices.size() );
        
        if(m_vertices.size() > 2) {
            glUniform4f(fragmentColor, 1.0,1.0,0.0,1.0);
            glVertexAttribPointer (0 , 3, GL_FLOAT, GL_FALSE, 0, &lCentroid);
            glDrawArrays (GL_POINTS, 0, 1);
        }
            
        glUniform4f(fragmentColor, 1.0,1.0,1.0,1.0);
        
        /*    copy the selected vertices */
        /*    from the pointed locations in m_pVertices */
        /*    to a temporary vector in sequence */
        std::vector<vertexData> tmpVertices;
        std::vector<vertexData *>::iterator it;
        tmpVertices.clear();
        for (it=m_pVertices.begin(); it!=m_pVertices.end(); it++) {
            tmpVertices.push_back(**it);
        }

        /*   draw the selected vertices,  */
        /*    and the highlighted centroid if necessary. */       
        glVertexAttribPointer (0 , 3, GL_FLOAT, GL_FALSE, 0, &tmpVertices[0]);
        glDrawArrays( GL_POINTS, 0, tmpVertices.size());
        glDrawArrays (GL_LINE_LOOP, 0, tmpVertices.size());
        
        // Draw Mode Toggle Touch Zone
        m_MVPMatrix = lProjMatrix;
        glUniformMatrix4fv(MVP_Location, 1, false, glm::value_ptr(m_MVPMatrix));
        /* Use different colors */
        /*     for the three distinct interaction modes. */
        if(g_mode == 0) {
            glUniform4f(fragmentColor,1.0,0.0,0.0,1.0);
        }
        else if(g_mode == 1) {
            glUniform4f(fragmentColor,0.0,1.0,0.0,1.0);
        }
        else if(g_mode == 2) {
            glUniform4f(fragmentColor,0.0,0.0,1.0,1.0);
        }
        glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, &m_touchZoneVertices[0]);
        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );

        
        SDL_GL_SwapWindow(m_window);
    } 
    
    void generateProximityList(glm::vec3 curPos) {
        std::vector<vertexData>::iterator it;
        float pointProximityRadius = 20.0f;
        float edgeProximityDistance = 10.0f;
        
        // Check for close points:
        m_pVertices.clear();
        for (it = m_vertices.begin(); it != m_vertices.end(); it++) {
            
            /* Check for proximity to curPos */
            /* and store a pointer to the closest candidate in m_pVertices */
            if (glm::distance(curPos, it->position) < pointProximityRadius) {
                SDL_Log("too close to a point! ");
                m_pVertices.push_back(&(*it));
            }
        }
        
        // Check for close edges:
        if (m_pVertices.size()==0 && m_vertices.size()>1) {
            float minDistance = edgeProximityDistance;
            SDL_Log("looping through %d vertices", m_vertices.size());
            for(int i=0; i<m_vertices.size(); i++) {
                int prevIndex = (i-1) - m_vertices.size() * glm::floor((i-1.0)/m_vertices.size());
                
                glm::vec3 edge = m_vertices[i].position - m_vertices[prevIndex].position;
                float distance = glm::distance(m_vertices[i].position, m_vertices[prevIndex].position);
                
                //SDL_Log("the two points are this far apart = %f", distance);
                glm::vec3 norm = glm::normalize(glm::vec3(-(m_vertices[i].position.y - m_vertices[prevIndex].position.y), (m_vertices[i].position.x - m_vertices[prevIndex].position.x), 0.0f));
                glm::vec3 v = glm::normalize(edge);
                
                glm::vec3 x0minusx1 = curPos - m_vertices[i].position;
                
                //SDL_Log("NORM(%f,%f,%f) * [x0-x1](%f,%f,%f)", norm.x,norm.y,norm.z,x0minusx1.x,x0minusx1.y,x0minusx1.z);
                float height = glm::dot(norm, x0minusx1);
                SDL_Log("current height(distance from edge) = %f", height);
                float length = glm::dot(v, x0minusx1);
                SDL_Log("currentlength to vertex #%d = %f", i, length);
                SDL_Log("distance = %f", distance);
                
                /* Check for edges proximity, */
                /*    and store pointers to the vertices of the closest candidate in m_pVertices */
                if(glm::abs(height) < minDistance && length < 0 && length > -distance) {
                    SDL_Log("too close to an edge!");
                    m_pVertices.push_back(&m_vertices[i]);
                    m_pVertices.push_back(&m_vertices[prevIndex]);
                }
            }
        }
        
        // Check for proxmity to the geometric center:
        if ((m_pVertices.size()==0) && (m_vertices.size()>2)) {
            vertexData lCentroid = getCentroid();


            /* Check for curPos proximity to the centroid, */
            /*     and store pointers to all m_vertices in m_pVertices */
            if (glm::distance(curPos, lCentroid.position) < pointProximityRadius) {
                int i = 0;
                for(i = 0; i < m_vertices.size(); i++) {
                    m_pVertices.push_back(&m_vertices[i]);
                }
            }
        }
        
    } 
    
    std::vector<vertexData *> & getProximityList() {
        return m_pVertices;
    }
    
    void clearProximityList() {
        m_pVertices.clear();
    }
    
    vertexData getCentroid() {
        std::vector<vertexData>::iterator it;
        glm::vec3 tmpVec(0.0f,0.0f,0.0f);
        vertexData tmpData;

        /* Compute and */
        /* return the average position from m_vertices */
        float totalX = 0;
        float totalY = 0;
        int i = 0;
        for(i = 0; i < m_vertices.size(); i++) {
            totalX += m_vertices[i].position.x;
            totalY += m_vertices[i].position.y;
        }
        
        tmpData.position.x = totalX/i;
        tmpData.position.y = totalY/i;

        return tmpData;
    }
    
    void getDimensions(float &w, float &h) {
        w = m_w;
        h = m_h;
    }
    
    void applyModelTransformation() {
        std::vector<vertexData>::iterator it;
        vertexData lCentroid = getCentroid();

        /* Permanently apply all     */
        /* temporary transformations to the vertices */
        float phi = 0;
        float dr = glm::distance(m_fingerDragOffset.x, m_fingerDragOffset.y);
        SDL_Log("applying model transformation (%f,%f)",m_fingerDragOffset.x,m_fingerDragOffset.y);
        float R = 10.0f;
        phi = dr/R;
        glm::mat4x4 TransMatrix = glm::mat4x4(1.0,                  0.0,                 0.0,0.0,
                                              0.0,                  1.0,                 0.0,0.0,
                                              0.0,                  0.0,                 1.0,0.0,
                                              lCentroid.position.x, lCentroid.position.y,0.0,1.0);
        glm::mat4x4 InvTransMatrix = glm::mat4x4(1.0,                   0.0,                 0.0,0.0,
                                                 0.0,                   1.0,                 0.0,0.0,
                                                 0.0,                   0.0,                 1.0,0.0,
                                                 -lCentroid.position.x, -lCentroid.position.y,0.0,1.0);
        glm::mat4x4 RotMatrix = glm::mat4x4(glm::cos(phi), -glm::sin(phi), 0.0,0.0,
                                            glm::sin(phi), glm::cos(phi), 0.0,0.0,
                                            0.0,           0.0,           1.0,0.0,
                                            0.0,           0.0,           0.0,1.0);
        glm::mat4x4 FPRMat = TransMatrix * RotMatrix * InvTransMatrix;
        
        for(it = m_vertices.begin(); it != m_vertices.end(); it++) {
            glm::vec4 tempPosition = glm::vec4(it->position.x, it->position.y, it->position.z, 1.0);
            tempPosition = TransMatrix * RotMatrix * InvTransMatrix * tempPosition;
            it->position.x = tempPosition.x;
            it->position.y = tempPosition.y;
            it->position.z = tempPosition.z;
        }
        
    }
}; 

void UpdateFrame(void* param) {
    //SDL_Log("UpdateFrame()");
    Graphics* graphics = (Graphics*)param;
    graphics->Draw();
} 

int EventFilter(void* userdata, SDL_Event* event) {
    SDL_Log("EventFilter()");
    SDL_Log("Mode: %d", g_mode);
    vertexData tmp;
    glm::vec3 hitPoint;
    glm::vec3 translationVector;
    glm::mat4x4 ModelMatrix;
    std::vector<vertexData *> & proximityList = g_graphics->getProximityList();
    std::vector<vertexData *>::iterator it;
    float w, h;
    
    g_graphics->getDimensions(w,h);
    
    switch (event->type) {
        case SDL_FINGERMOTION:
            SDL_Log("Finger Motion");
            translationVector = glm::vec3(w*event->tfinger.dx, -h*event->tfinger.dy, 0.0);
            if (g_mode == E_MODE_EDIT) {
                for(it=proximityList.begin(); it!=proximityList.end(); it++) {
                    /* Use the input vector --a translation-- */
                    /* to update the positions of the selected vertices */
                    (*it)->position = (*it)->position + translationVector;
                }
            }
            if (g_mode == E_MODE_TRANSFORM) {
                /* Use the input vector */
                /*            to compute a model transformation in the Draw() method. */
                //float totalXChange = g_graphics->m_fingerDragOffset.z + w*event->tfinger.dx;
                //SDL_Log("totalXChange = %f", totalXChange);
                g_graphics->m_fingerDragOffset += translationVector; //glm::vec3(w*event->tfinger.x, w*event->tfinger.y, totalXChange);
            }
            return 0;
            
        case SDL_FINGERDOWN:
            SDL_Log("Finger Down");
            {
                g_graphics->m_fingerDragOffset = glm::vec3(0.0f);
                hitPoint = glm::vec3(w * event->tfinger.x, h * (1.0 - event->tfinger.y), 0.0f);
                if (hitPoint.y > 0.9f * h) {
                    // toggle interaction mode: 
                    g_mode = (eEditMode)((g_mode + 1) % NUM_MODES);
                    //SDL_Log("Switching interaction mode to ...");
                    return 0;
                }
                
                if (g_mode == E_MODE_INSERT) {
                    /* Complete implementing generateProximityList() */
                    g_graphics->generateProximityList(hitPoint);
                    proximityList = g_graphics->getProximityList();
                    if (proximityList.size() == 0) {
                        /* Insert a vertex */
                        /*  with position equal to hitPoint in m_vertices. */
                        SDL_Log("inserting a new vertex!");
                        vertexData insertedVertex;
                        insertedVertex.position = glm::vec3(hitPoint.x,hitPoint.y,0.0f);
                        g_graphics->m_vertices.push_back( insertedVertex );
                    }
                }
                if (g_mode == E_MODE_EDIT) {
                    g_graphics->generateProximityList(hitPoint);
                    proximityList = g_graphics->getProximityList();
                }
            }
            return 0;
            
        case SDL_FINGERUP:
            SDL_Log("Finger Up");
            if (g_mode == E_MODE_TRANSFORM) {
                /* Apply */
                /* the temporary transformations upon releasing */
                g_graphics->applyModelTransformation();
            }
            g_graphics->m_fingerDragOffset = glm::vec3(0.0f);
            g_graphics->clearProximityList();
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
    
    /* add a callback to be triggered when an event is added to the event queue: */
    SDL_AddEventWatch(EventFilter, NULL);
        
    if (SDL_RecordGesture(-1) == 0)
    {
        return 1;
    };
    
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
