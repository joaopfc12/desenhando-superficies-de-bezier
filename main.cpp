#include <GL/glut.h>
#include <stdlib.h>

typedef struct {
    float x, y, z;
} Ponto3D;

// Matriz 4x4 com os 16 pontos de controle inicializados diretamente
Ponto3D ctrlPoints[4][4] = {
    { { 0.0f,  0.0f,  0.0f}, {25.0f,  0.0f,  0.0f}, {50.0f,  0.0f,  0.0f}, {75.0f,  0.0f,  0.0f} },
    { { 0.0f,  0.0f, 25.0f}, {25.0f, 50.0f, 25.0f}, {50.0f, 50.0f, 25.0f}, {75.0f,  0.0f, 25.0f} },
    { { 0.0f,  0.0f, 50.0f}, {25.0f, 50.0f, 50.0f}, {50.0f, 50.0f, 50.0f}, {75.0f,  0.0f, 50.0f} },
    { { 0.0f,  0.0f, 75.0f}, {25.0f,  0.0f, 75.0f}, {50.0f,  0.0f, 75.0f}, {75.0f,  0.0f, 75.0f} }
};

// Resolução da malha da superfície (número de subdivisões em u e em v)
#define RESOLUCAO 20

// Pontos da superfície já calculados: S(u,v) para u,v em [0,1]
Ponto3D superficie[RESOLUCAO + 1][RESOLUCAO + 1];

// Polinômios de Bernstein de grau 3:
// B0(t) = (1-t)^3, B1(t) = 3t(1-t)^2, B2(t) = 3t^2(1-t), B3(t) = t^3
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

// Avalia a superfície de Bezier bicúbica no ponto (u,v):
// S(u,v) = soma_i soma_j  Bi(u) * Bj(v) * P[i][j]
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

// Calcula todos os pontos da malha uma única vez
void calculaSuperficie(void) {
    for (int a = 0; a <= RESOLUCAO; a++) {
        float u = (float)a / RESOLUCAO;
        for (int b = 0; b <= RESOLUCAO; b++) {
            float v = (float)b / RESOLUCAO;
            superficie[a][b] = bezierSuperficie(u, v);
        }
    }
}

// Cor da superfície de acordo com a altura (y): azul embaixo, amarelo em cima
void corPorAltura(float y) {
    float t = y / 50.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    glColor3f(0.15f + 0.85f * t, 0.35f + 0.5f * t, 0.9f - 0.7f * t);
}

// Variáveis para rotação e zoom interativos
int mouseX, mouseY;
int mouseBotao = -1;
float rotX = 25.0f;
float rotY = -45.0f;
float distancia = 180.0f;


void init(void) {
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f); // Fundo cinza-escuro
    glEnable(GL_DEPTH_TEST);
    glPointSize(8.0f);
    glLineWidth(1.5f);

    calculaSuperficie(); // Pré-calcula os pontos da superfície de Bezier
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Posiciona a câmera em perspectiva
    gluLookAt(0.0, 0.0, distancia,   // Posição do observador
        0.0, 0.0, 0.0,         // Ponto de mira
        0.0, 1.0, 0.0);        // Vetor UP

    // Rotações controladas pelo mouse
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    // Centraliza a grade na origem (dimensões da malha: 75 em X e Z, 50 em Y)
    glTranslatef(-37.5f, -15.0f, -37.5f);

    // 0. Superfície de Bezier
    // 0a. Faces preenchidas (tiras de quadriláteros), coloridas pela altura
    glEnable(GL_POLYGON_OFFSET_FILL);   // Empurra as faces um pouco para trás
    glPolygonOffset(1.0f, 1.0f);        // para a malha de linhas aparecer por cima
    for (int a = 0; a < RESOLUCAO; a++) {
        glBegin(GL_QUAD_STRIP);
        for (int b = 0; b <= RESOLUCAO; b++) {
            Ponto3D p1 = superficie[a][b];
            Ponto3D p2 = superficie[a + 1][b];
            corPorAltura(p1.y); glVertex3f(p1.x, p1.y, p1.z);
            corPorAltura(p2.y); glVertex3f(p2.x, p2.y, p2.z);
        }
        glEnd();
    }
    glDisable(GL_POLYGON_OFFSET_FILL);

    // 0b. Malha de linhas (curvas isoparamétricas em u e em v)
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int a = 0; a <= RESOLUCAO; a++) {      // curvas com u constante
        glBegin(GL_LINE_STRIP);
        for (int b = 0; b <= RESOLUCAO; b++)
            glVertex3f(superficie[a][b].x, superficie[a][b].y, superficie[a][b].z);
        glEnd();
    }
    for (int b = 0; b <= RESOLUCAO; b++) {      // curvas com v constante
        glBegin(GL_LINE_STRIP);
        for (int a = 0; a <= RESOLUCAO; a++)
            glVertex3f(superficie[a][b].x, superficie[a][b].y, superficie[a][b].z);
        glEnd();
    }

    // 1. Linhas do polígono de controle
    glColor3f(0.7f, 0.7f, 0.7f); // Linhas cinza-claras
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

    // 2. Vértices de controle destacados
    glColor3f(1.0f, 0.2f, 0.2f); // Pontos em vermelho
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

    // Projeção em Perspectiva (fovy, aspect, zNear, zFar)
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
    else if (key == 27) { // ESC
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