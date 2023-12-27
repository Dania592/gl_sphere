#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#endif
int window_height = 1000;
int window_width = 1000;
int nbPara = 7;
int nbMeri = 7;
float cameraAngle = 0.0f;

typedef struct{
    float x;
    float y;
    float z;
}Point;

typedef struct{
    int nbSom;
    int nbFacette;
    int **Facettes;
    Point *Sommets;
}Surface;
Surface surf;


// discrétisation de l'espace paramétrique
void parametrisation(float a, float b, int n, float *tab){
    float pas = (b-a)/(float)(n-1);
    tab[0] = a;
    for(int i = 1; i <= n; i++){
        tab[i] = tab[i-1]+pas;
    }
}

void espaceParametrique(float a, float b, float c,float d,float *U, float *V){
    parametrisation(a, b, nbPara, U);
    parametrisation(c, d, nbMeri, V);
}

Point surface(float teta, float phi, float r){
    Point point;
    point.x = r*cosf(teta)*sinf(phi);
    point.y = r*sinf(teta)*sinf(phi);
    point.z = r*cosf(phi);
    return point;
}

void generateSommets(float *U, float *V, Point *sommets, float r){
    int nbS = 0;

    // sommet pole sud
    float teta = U[0];
    float phi = V[0];
    sommets[nbS] = surface(teta, phi, r);
    nbS++;

    // sommets intermediaires
    for(int p = 1; p <= nbPara-2; p++){
        phi = V[p];
        for(int m = 0; m <= nbMeri-2; m++){
            teta = U[m];
            sommets[nbS] = surface(teta, phi, r);
            nbS++;
        }
    }
    // sommet pole nord
    phi = V[nbMeri-1];
    teta = U[0];
    sommets[nbS] = surface(teta, phi, r);
    nbS++;
}

void generFacettes(int **facettes){
    int nbF = 0;
    // génération des facettes du pole sud (triangulaire)
    for(int m = 1; m <= nbMeri-2; m++){
        facettes[nbF][0] = 3;
        facettes[nbF][1] = 0;
        facettes[nbF][2] = m+1;
        facettes[nbF][3] = m;
        nbF++;
    }

    // dernière facette du pole sud (triangulaire)
    facettes[nbF][0] = 3;
    facettes[nbF][1] = 0;
    facettes[nbF][2] = 1;
    facettes[nbF][3] = nbMeri-1;
    nbF++;

    // génération des facettes intermédiaires (rectangulaire)
    int p;
    for(p = 1; p <= nbPara-3; p++){
        for(int m = 1; m <= nbMeri-2; m++){
            facettes[nbF][0] = 4;
            facettes[nbF][1] = m + (p-1)*(nbMeri-1);
            facettes[nbF][2] =  m + (p-1)*(nbMeri-1) + 1;
            facettes[nbF][3] =  m + p*(nbMeri-1) + 1;
            facettes[nbF][4] =  m + p*(nbMeri-1);
            nbF++;
        }
        facettes[nbF][0] = 4;
        facettes[nbF][1] = p*(nbMeri-1);
        facettes[nbF][2] = (p-1)*(nbMeri-1) + 1;
        facettes[nbF][3] = p*(nbMeri-1) + 1;
        facettes[nbF][4] = (p+1)*(nbMeri-1);
        nbF++;
    }

    // génération des facettes du pole nord (triangulaire)
    for(int m = 1; m <= nbMeri -2; m++){
        facettes[nbF][0] = 3;
        facettes[nbF][1] = m + (nbPara-3)*(nbMeri-1);
        facettes[nbF][2] = m + (nbPara-3)*(nbMeri-1) + 1;
        facettes[nbF][3] = 1 + (nbPara-2)*(nbMeri-1);
        nbF++;
    }
    // dernière facette pole nord (triangulaire)
    facettes[nbF][0] = 3;
    facettes[nbF][1] = (nbPara-2)*(nbMeri-1);
    facettes[nbF][2] = (nbPara-3)*(nbMeri-1) + 1;
    facettes[nbF][3] = (nbPara-2)*(nbMeri-1) + 1;
    nbF++;
}

Point *allocMemTabPoints(int nbPoint){
    return (Point*) malloc(nbPoint*sizeof (Point));
}

int **allocMemMatEnt(int nbL, int nbC){
    int **facettes = (int**)malloc(nbL* sizeof(int*));
    for(int i = 0; i < nbL; i++){
        facettes[i] = (int*)malloc(nbC*sizeof(int));
    }
    return facettes;
}

Surface allocMemSurf(){
    surf.nbSom = (nbMeri-1)*(nbPara-2)+2;
    surf.nbFacette = (nbMeri-1)*(nbPara-1);
    surf.Sommets = allocMemTabPoints(surf.nbSom);
    surf.Facettes = allocMemMatEnt(surf.nbFacette, 5);
    return surf;
}

void libMemoireSommets(Point *sommets){
    free(sommets);
    sommets = NULL;
}

void libMemoireSurf(Surface *surf){
    libMemoireSommets(&(*surf->Sommets));
    for(int i = 0; i < surf->nbFacette; i++){
        free(surf->Facettes[i]);
    }
    free(surf->Facettes);
    free(surf);
}

void allocMeMemEspaceParam(int m, int n, float **U, float **V){
    *U = (float*) malloc((n+1)* sizeof(float));
    *V = (float*) malloc((m+1)* sizeof(float));
    if (*U == NULL || *V == NULL) {
        printf("Allocation mémoire échouée\n");
        exit(1);
    }
}

void surfaceParam(float r){
    float *U, *V;
    allocMeMemEspaceParam(nbPara, nbMeri, &U, &V);
    espaceParametrique(0, 2*M_PI, 0, M_PI, U, V);
    surf = allocMemSurf();
    generateSommets(U, V,surf.Sommets, r);
    generFacettes(surf.Facettes);
    //libMemoireSurf(&surf);
}

void render_scene(){
    for(int i = 0; i < nbMeri-1; i++){
        int p = surf.Facettes[i][1];
        Point A = surf.Sommets[p];
        p = surf.Facettes[i][2];
        Point B = surf.Sommets[p];
        p = surf.Facettes[i][3];
        Point C = surf.Sommets[p];
        glColor3f(0.996f, 0.937f, 0.867f);
        glBegin(GL_TRIANGLES);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(C.x, C.y, C.z);
            glVertex3f(A.x, A.y, A.z);
        glEnd();
    }

    for(int i = nbMeri-1; i < surf.nbFacette - nbMeri+1; i++){
        int p = surf.Facettes[i][1];
        Point A = surf.Sommets[p];
        p = surf.Facettes[i][2];
        Point B = surf.Sommets[p];
        p = surf.Facettes[i][3];
        Point C = surf.Sommets[p];
        p = surf.Facettes[i][4];
        Point D = surf.Sommets[p];
        glColor3f(0.996f, 0.937f, 0.867f);
        glPointSize(8);
        glBegin(GL_POINTS);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(C.x, C.y, C.z);
            glVertex3f(D.x, D.y, D.z);
            glVertex3f(A.x, A.y, A.z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3f(A.x, A.y, A.z);
        glVertex3f(B.x, B.y, B.z);
        glVertex3f(C.x, C.y, C.z);
        glVertex3f(D.x, D.y, D.z);
        glVertex3f(A.x, A.y, A.z);
        glEnd();
    }

    for(int i = surf.nbFacette - nbMeri+1; i < surf.nbFacette; i++){
        int p = surf.Facettes[i][1];
        Point A = surf.Sommets[p];
        p = surf.Facettes[i][2];
        Point B = surf.Sommets[p];
        p = surf.Facettes[i][3];
        Point C = surf.Sommets[p];
        glColor3f(0.996f, 0.937f, 0.867f);
        glBegin(GL_TRIANGLES);
            glVertex3f(A.x, A.y, A.z);
            glVertex3f(B.x, B.y, B.z);
            glVertex3f(C.x, C.y, C.z);
            glVertex3f(A.x, A.y, A.z);
        glEnd();
    }
}

void window_reshape(int width, int height) {
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void window_display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float cameraX = 5.0f * sinf(cameraAngle);
    float cameraZ = 5.0f * cosf(cameraAngle);

    gluLookAt(cameraX, 2.0, cameraZ,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    glColor3f(1.0f, 1.0f, 1.0f);

    render_scene();

    glutSwapBuffers();
}

void updateCamera() {
    cameraAngle += 0.02f;
}

void timer(int value) {
    updateCamera();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // 60 trames par seconde
}

int main(int argc, char** argv) {
    nbPara = 15;
    nbMeri = 15;
    surfaceParam(1);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("surface parametrique");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(window_display);
    glutReshapeFunc(window_reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}


//
// Created by dania on 24/12/2023.
//
