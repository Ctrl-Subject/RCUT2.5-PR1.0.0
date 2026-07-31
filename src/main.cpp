#define GLUT_DISABLE_ATEXIT_HACK

#include <GL/freeglut.h>
#include <cmath>

#define PI 3.1415926535f

//------------------------------------------

float PlayerX = 2 * 64 + 32;
float PlayerY = 1 * 64 + 32;
float PlayerAngle = 0;
float rotSpeed = 0.05f;

bool keys[256] = {0};
bool skeys[256] = {0};
bool ignoreWarp = false;

const int TILE = 64;

//------------------------------------------

struct window{
    int width;
    int height;
    window(int w,int h){width=w;height=h;}
};

window win(1024,512);

void drawPointerBox()
{
    int CenterX = win.width / 2;
    int CenterY = win.height / 2;

    int size = 5;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1, 1, 1, 0.5f);

    glBegin(GL_QUADS);
        glVertex2i(CenterX - size, CenterY - size);
        glVertex2i(CenterX + size, CenterY - size);
        glVertex2i(CenterX + size, CenterY + size);
        glVertex2i(CenterX - size, CenterY + size);
    glEnd();

    glDisable(GL_BLEND);
}



//------------------ MAP -------------------

int mapx=21, mapyY=27;

// 0 = empty, 1 = wall, 2 = teleport wall
int map[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,
    1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,
    1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,
    1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,
    0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,
    0,0,0,0,1,0,1,0,1,1,0,1,1,0,1,0,1,0,0,0,1,
    1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,1,
    2,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,2,
    1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,
    0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,
    0,0,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,0,1,
    1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,
    1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,
    1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,
    1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,
    1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

int WraPlayerX(int x)
{
    return (x % mapx + mapx) % mapx;
}

//------------------------------------------

void drawRays(int screenH, int screenW, float FOV)
{
    float ra = PlayerAngle - FOV/2;
    float da = FOV / screenW;

    for(int r = 0; r < screenW; r++)
    {
        float rayDirX = cos(ra);
        float rayDirY = sin(ra);

        int mapx = (int)(PlayerX / TILE);
        int mapyY = (int)(PlayerY / TILE);

        float deltaDistX = (fabs(rayDirX) < 0.0001f) ? 1e30f : fabs(1.0f / rayDirX);
        float deltaDistY = (fabs(rayDirY) < 0.0001f) ? 1e30f : fabs(1.0f / rayDirY);

        int stepx, stepy;
        float sideDistX, sideDistY;

        if(rayDirX < 0){
            stepx = -1;
            sideDistX = (PlayerX/TILE - mapx) * deltaDistX;
        } else {
            stepx = 1;
            sideDistX = (mapx + 1.0f - PlayerX/TILE) * deltaDistX;
        }

        if(rayDirY < 0){
            stepy = -1;
            sideDistY = (PlayerY/TILE - mapyY) * deltaDistY;
        } else {
            stepy = 1;
            sideDistY = (mapyY + 1.0f - PlayerY/TILE) * deltaDistY;
        }

        int hit = 0, side, hitType = 0;

        while(!hit){
            if(sideDistX < sideDistY){
                sideDistX += deltaDistX;
                mapx += stepx;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapyY += stepy;
                side = 1;
            }

            // stop at map bounds
            if(mapx < 0 || mapx >= mapx || mapyY < 0 || mapyY >= mapyY){
                hit = 1;
                hitType = 0;
                break;
            }

            int tile = map[mapyY * mapx + mapx];
            if(tile != 0){
                hit = 1;
                hitType = tile;
            }
        }

        float dist;
        if(side == 0)
            dist = (sideDistX - deltaDistX);
        else
            dist = (sideDistY - deltaDistY);

        dist *= TILE;

        float ca = PlayerAngle - ra;
        if(ca < 0) ca += 2*PI;
        if(ca > 2*PI) ca -= 2*PI;
        dist *= cos(ca);

        float lineH = (TILE * screenH) / (dist + 0.0001f);
        if(lineH > screenH) lineH = screenH;

        float lineO = (screenH/2) - lineH/2;

        // 🎨 COLOR LOGIC
        if(hitType == 2)         glColor3f(0,0,1);     // blue teleport
        else if(side == 0)       glColor3f(1,0,0);
        else                     glColor3f(0.7,0,0);

        glBegin(GL_LINES);
        glVertex2i(r, lineO);
        glVertex2i(r, lineO + lineH);
        glEnd();

        ra += da;
    }
}

//------------------------------------------

void mouseMove(int x, int y)
{
    if(ignoreWarp)
    {
        ignoreWarp = false;
        return;
    }

    int centerX = win.width / 2;
    int dx = x - centerX;

    float sensitivity = 0.003f;
    PlayerAngle += dx * sensitivity;

    if(PlayerAngle < 0) PlayerAngle += 2*PI;
    if(PlayerAngle > 2*PI) PlayerAngle -= 2*PI;

    ignoreWarp = true;
    glutWarpPointer(centerX, win.height / 2);
}

void update()
{
    float moveSpeed = 3.0f;

    // ----------------------------
    // INPUT DIRECTION
    // ----------------------------
    float dx = 0, dy = 0;
    
    // ----------------------------
	// ROTATION (ARROW KEYS)
	// ----------------------------
	if(skeys[GLUT_KEY_LEFT])
	    PlayerAngle -= rotSpeed;
	
	if(skeys[GLUT_KEY_RIGHT])
	    PlayerAngle += rotSpeed;
	
	if(PlayerAngle < 0) PlayerAngle += 2 * PI;
	if(PlayerAngle > 2 * PI) PlayerAngle -= 2 * PI;

    if(keys['w'] || keys['W']) { dx += cos(PlayerAngle); dy += sin(PlayerAngle); }
    if(keys['s'] || keys['S']) { dx -= cos(PlayerAngle); dy -= sin(PlayerAngle); }
    if(keys['d'] || keys['D']) { dx -= sin(PlayerAngle); dy += cos(PlayerAngle); }
    if(keys['a'] || keys['A']) { dx += sin(PlayerAngle); dy -= cos(PlayerAngle); }

    float len = sqrt(dx*dx + dy*dy);
    if(len != 0)
    {
        dx /= len;
        dy /= len;
    }

    float nx = PlayerX + dx * moveSpeed;
    float ny = PlayerY + dy * moveSpeed;

    // ----------------------------
    // X MOVE + CHECK TILE
    // ----------------------------
    int mx = WraPlayerX((int)(nx / TILE));
    int my = (int)(PlayerY / TILE);

    if(my >= 0 && my < mapyY)
    {
        int tile = map[my * mapx + mx];

        if(tile == 0)
        {
            PlayerX = nx;
        }
        else if(tile == 2)
        {
            // TELEPORT (X SIDE)
            if(mx == 0)
                PlayerX = (mapx - 2) * TILE;
            else
                PlayerX = TILE;
        }
    }

    // ----------------------------
    // Y MOVE + CHECK TILE
    // ----------------------------
    mx = WraPlayerX((int)(PlayerX / TILE));
    my = (int)(ny / TILE);

    if(my >= 0 && my < mapyY)
    {
        int tile = map[my * mapx + mx];

        if(tile == 0)
        {
            PlayerY = ny;
        }
        else if(tile == 2)
        {
            // TELEPORT (Y SIDE)
            if(my == 0)
                PlayerY = (mapyY - 2) * TILE;
            else
                PlayerY = TILE;
        }
    }

    glutPostRedisplay();
}

//------------------------------------------

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawRays(win.height, win.width, PI/3);
    drawPointerBox();
    glutSwapBuffers();
    glutPostRedisplay();
}

//------------------------------------------

void timer(int v){ 
	update(); 
	glutPostRedisplay();
	glutTimerFunc(16,timer,0); 
}

void keyDown(unsigned char k,int x,int y){ keys[k]=1; }
void keyUp(unsigned char k,int x,int y){ keys[k]=0; }
void skeyDown(int k,int x,int y){ skeys[k]=1; }
void skeyUp(int k,int x,int y){ skeys[k]=0; }

//------------------------------------------

void init()
{
    glClearColor(0.3,0.3,0.3,0);
    gluOrtho2D(0,win.width,win.height,0);
}

//------------------------------------------

int main(int argc,char** argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(win.width, win.height);
    glutCreateWindow("PlayerAnglecman");
    glutSetCursor(GLUT_CURSOR_NONE);

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(skeyDown);
	glutSpecialUpFunc(skeyUp);
	glutPlayerAnglessiveMotionFunc(mouseMove);

    glutTimerFunc(0,timer,0);
    glutMainLoop();
}