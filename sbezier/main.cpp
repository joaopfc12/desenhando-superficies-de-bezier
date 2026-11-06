#include <GL/glut.h>
#include <stdlib.h>

typedef struct {
    float x, y, z;
} Ponto3D;

Ponto3D ctrlPoints[4][4] = {
    { { 0.0f,  0.0f,  0.0f}, {25.0f,  0.0f,  0.0f}, {50.0f,  0.0f,  0.0f}, {75.0f,  0.0f,  0.0f} },
    { { 0.0f,  0.0f, 25.0f}, {25.0f, 50.0f, 25.0f}, {50.0f, 50.0f, 25.0f}, {75.0f,  0.0f, 25.0f} },
    { { 0.0f,  0.0f, 50.0f}, {25.0f, 50.0f, 50.0f}, {50.0f, 50.0f, 50.0f}, {75.0f,  0.0f, 50.0f} },
    { { 0.0f,  0.0f, 75.0f}, {25.0f,  0.0f, 75.0f}, {50.0f,  0.0f, 75.0f}, {75.0f,  0.0f, 75.0f} }
};

#define RESOLUCAO 20

Ponto3D superficie[RESOLUCAO + 1][RESOLUCAO + 1];


float bernstein(int i, float t) {
    float s = 1.0f - t;
    switch (i) {
    case 0: return s * s * s;
    case 1: return 3.0f * t * s * s;
    case 2: return 3.0f * t * t * s;
    case 3: return t * t * t;
    }
    return 0.0f;
}

Ponto3D bezierSuperficie(float u, float v) {
    Ponto3D p = { 0.0f, 0.0f, 0.0f };
    for (int i = 0; i < 4; i++) {
        float bu = bernstein(i, u);
        for (int j = 0; j < 4; j++) {
            float b = bu * bernstein(j, v);
            p.x += b * ctrlPoints[i][j].x;
            p.y += b * ctrlPoints[i][j].y;
            p.z += b * ctrlPoints[i][j].z;
        }
    }
    return p;
}

void calculaSuperficie(void) {
    for (int a = 0; a <= RESOLUCAO; a++) {
        float u = (float)a / RESOLUCAO;
        for (int b = 0; b <= RESOLUCAO; b++) {
            float v = (float)b / RESOLUCAO;
            superficie[a][b] = bezierSuperficie(u, v);
        }
    }
}

int mouseX, mouseY;
int mouseBotao = -1;
float rotX = 25.0f;
float rotY = -45.0f;
float distancia = 180.0f;


void init(void) {
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    glPointSize(8.0f);
    glLineWidth(1.5f);

    calculaSuperficie(); 
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

   
    gluLookAt(0.0, 0.0, distancia,   
        0.0, 0.0, 0.0,         
        0.0, 1.0, 0.0);       

   
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

  
    glTranslatef(-37.5f, -15.0f, -37.5f);

    
    glColor3f(1.0f, 1.0f, 1.0f); 
    for (int a = 0; a <= RESOLUCAO; a++) {      
        glBegin(GL_LINE_STRIP);
        for (int b = 0; b <= RESOLUCAO; b++)
            glVertex3f(superficie[a][b].x, superficie[a][b].y, superficie[a][b].z);
        glEnd();
    }
    for (int b = 0; b <= RESOLUCAO; b++) {     
        glBegin(GL_LINE_STRIP);
        for (int a = 0; a <= RESOLUCAO; a++)
            glVertex3f(superficie[a][b].x, superficie[a][b].y, superficie[a][b].z);
        glEnd();
    }

   
    glColor3f(0.7f, 0.7f, 0.7f); 
    for (int i = 0; i < 4; i++) {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < 4; j++) {
            glVertex3f(ctrlPoints[i][j].x, ctrlPoints[i][j].y, ctrlPoints[i][j].z);
        }
        glEnd();
    }
    for (int j = 0; j < 4; j++) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < 4; i++) {
            glVertex3f(ctrlPoints[i][j].x, ctrlPoints[i][j].y, ctrlPoints[i][j].z);
        }
        glEnd();
    }

   
    glColor3f(1.0f, 0.2f, 0.2f); 
    glBegin(GL_POINTS);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            glVertex3f(ctrlPoints[i][j].x, ctrlPoints[i][j].y, ctrlPoints[i][j].z);
        }
    }
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

   
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 500.0);
}

void mouse(int botao, int estado, int x, int y) {
    if (estado == GLUT_DOWN) {
        mouseBotao = botao;
        mouseX = x;
        mouseY = y;
    }
    else {
        mouseBotao = -1;
    }
}

void motion(int x, int y) {
    if (mouseBotao == GLUT_LEFT_BUTTON) {
        rotY += (x - mouseX) * 0.5f;
        rotX += (y - mouseY) * 0.5f;
        mouseX = x;
        mouseY = y;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') {
        distancia -= 5.0f;
        if (distancia < 20.0f) distancia = 20.0f;
        glutPostRedisplay();
    }
    else if (key == '-' || key == '_') {
        distancia += 5.0f;
        glutPostRedisplay();
    }
    else if (key == 27) { 
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CG - Superficie de Bezier");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}