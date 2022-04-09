/*****************************************************************************\
 * TP CPE, 4ETI, TP synthese d'images
 * --------------
 *
 * Programme principal des appels OpenGL
 \*****************************************************************************/

/* TODO : 
    - Coder la possibilité de selectionner la façon de controler le stegosaure sur l'écran d'accueil 
    
    - gestion de la lumiere

*/
/*
    TOUCHES DU JEU
    fleches directionnelles pour tourner. 
    Il avance tout seul.
    espace pour utiliser le boost
    't' pour tirer des missiles.

    "REGLES" DU JEU
    le joueur a 10 vies
                5 boost 
                10 munitions
    s'il touche un obstacle il perd une vie 
    s'il a plus de vie c'est la fin du jeu

    Il evolue dans un monde en globle (s'il atteint un bord il reapparait de l'autre cote MONDE 500 X 500 X 100
    Des obstacles aleatoires tombes du ciel il doit les eviter
*/

#include "Joueur.h"
#include <time.h>
#include"simplexnoise.h"

//identifiant des shaders

GLuint shader_program_id;
GLuint gui_program_id;

camera cam;

const int nb_obj = 1; // rajouter +1 ici pour rajouter un objet
objet3d objmur[nb_obj];
Joueur joueur;
Obstacles nouvObstacle[200];
Missile missiles[100];
Ring ring[100];
std::default_random_engine gen;
std::uniform_real_distribution<float> distrib(0,1);

int nbObstacles = (sizeof(nouvObstacle) / sizeof(nouvObstacle[0]));
int nbMissiles = (sizeof(missiles) / sizeof(missiles[0]));
int nbring = (sizeof(ring) / sizeof(ring[0]));

bool debut_partie = true;
bool pause = false;
bool autorisation_pause = true;

text text_vie;
text text_boost;
text text_munition;
text text_points;
text message_fin;
text message_title;
text message_tap_to_play;
text message_contoller_choice[2];
text message_pause;

// tableau de boolean pour la gestion d'appuis simultanés de touches directionnelles
enum {LEFT,RIGHT,UP,DOWN, ROLLD, ROOLG};
bool tableau_mvmnt[] = {false,false,false,false, false, false };


// pos souris dans l'ecran
float pos_x;
float pos_y;
float min = 0.45;
float max = 0.55;


/// <summary>
///  Ecran Menu 
/// </summary>
/*****************************************************************************\
* Ecran d'accueil                                                             *
\*****************************************************************************/
static void init_menu()
{
    shader_program_id = glhelper::create_program_from_file("shaders/shader.vert", "shaders/shader.frag"); CHECK_GL_ERROR();

  
    gui_program_id = glhelper::create_program_from_file("shaders/gui.vert", "shaders/gui.frag"); CHECK_GL_ERROR();
    

    message_title.value = "PLANE SIMULATOR 3000";
    message_title.bottomLeft = vec2(-1.0, 0.3);
    message_title.topRight = vec2(-1.0 + 0.10, 0.6);
    init_text(&message_title);
   
    message_contoller_choice[0].value = "Press 'm' for mouse";
    message_contoller_choice[0].bottomLeft = vec2(-0.9, 0);
    message_contoller_choice[0].topRight = vec2(-0.9 + 0.1, 0.10);
    init_text(&message_contoller_choice[0]);
    message_contoller_choice[1].value = " 'j' for joystick";
    message_contoller_choice[1].bottomLeft = vec2(-0.8, -0.20);
    message_contoller_choice[1].topRight = vec2(-0.8 + 0.1, -0.1);
    init_text(&message_contoller_choice[1]);

    message_tap_to_play.value = "Press enter to play";
    message_tap_to_play.bottomLeft = vec2(-0.70, -0.60);
    message_tap_to_play.topRight = vec2(-0.70 + 0.080, -0.50);
    init_text(&message_tap_to_play);
   
}

static void display_callback_menu()
{
    glClearColor(0.5f, 0.6f, 0.9f, 1.0f); CHECK_GL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();
    draw_text(&message_title);
    draw_text(&message_contoller_choice[0]);
    draw_text(&message_contoller_choice[1]);

    draw_text(&message_tap_to_play);
    glutSwapBuffers();
}

static void timer_callback_menu(int)
{
    if (debut_partie)
        glutTimerFunc(5, timer_callback_menu, 0);
    else
    {
        glutDisplayFunc(display_callback);
        glutTimerFunc(5, timer_callback, 0);
        init();
    }
    
    glutPostRedisplay();
}


/*****************************************************************************\
* Initialisation générale                                                     *
\*****************************************************************************/
static void init()
{

    shader_program_id = glhelper::create_program_from_file("shaders/shader.vert", "shaders/shader.frag"); CHECK_GL_ERROR();

    cam.projection = matrice_projection(60.0f * M_PI / 180.0f, 1.0f, 0.01f, 100.0f);
    cam.tr.translation = vec3(-2.0, 2.0, 10.0);
    
    // initialisation des objets utilisés
    init_model_joueur(); // joueur avion
    init_model_sol(); // Sol en herbe
    init_model_obstacle(); // lune
    init_model_ring(); // ring
    init_model_missile();// missiles
        
    
    // duplication des obstacles
    for (int j = 1; j <= nbObstacles - 1; j++)
        nouvObstacle[j] = nouvObstacle[0];

    // duplication des missiles
    for (int j = 1; j <= nbMissiles - 1; j++)
        missiles[j] = missiles[0];

    // duplication des rings
    for (int j = 1; j <= nbring - 1; j++)
    {
        ring[j] = ring[0];
        ring[j].tr.translation.z += 2 * j + 1;
        ring[j].setPosition(ring[j].tr.translation);
    }
    
    gui_program_id = glhelper::create_program_from_file("shaders/gui.vert", "shaders/gui.frag"); CHECK_GL_ERROR();


    // initialisation des textes
    text_vie.value = "Vie: " + std::to_string(joueur.getVie());
    text_vie.bottomLeft = vec2(0.60, 0.90);
    text_vie.topRight = vec2(0.60 + 0.05, 0.98);
    init_text(&text_vie);
  
    text_points.value = "Points: " + std::to_string(joueur.getPoints());
    text_points.bottomLeft = vec2(-1., 0.90);
    text_points.topRight = vec2(-1. + 0.05, 0.98);
    init_text(&text_points);

    text_munition.value = "Munitions: " + std::to_string(joueur.getReserveMunition());
    text_munition.bottomLeft = vec2(0.40, -0.98);
    text_munition.topRight = vec2(0.40 + 0.045, -0.9);
    init_text(&text_munition);
    
    text_boost.value = "Boost: " + std::to_string(joueur.getBoost());
    text_boost.bottomLeft = vec2(-1., -0.98);
    text_boost.topRight = vec2(-1. + 0.05, -0.9);
    init_text(&text_boost);

    message_fin.value = "FIN DE PARTIE";
    message_fin.bottomLeft = vec2(-0.65, -0.25);
    message_fin.topRight = vec2(-0.65 + 0.115, 0.25);
    init_text(&message_fin);
    message_fin.visible = false;

    message_pause.value = "PAUSE";
    message_pause.bottomLeft = vec2(-0.25, 0.0);
    message_pause.topRight = vec2(-0.25 + 0.115, 0.3);
    init_text(&message_pause);
    message_pause.visible = false;


}

/*****************************************************************************\
* display_callback                                                           *
\*****************************************************************************/
 static void display_callback()
{

    glClearColor(0.5f, 0.6f, 0.9f, 1.0f); CHECK_GL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();
 
    // Rotation vers la gauche
    if (tableau_mvmnt[LEFT] == true) {
       
        joueur.tournerGauche(abs(pos_y-min) );
    }

    // Rotation vers la droite
    if (tableau_mvmnt[RIGHT] == true) {
     
        joueur.tournerDroite(abs(pos_y - max));
    }

    // Déplacements vers le haut
    if (tableau_mvmnt[UP] == true) {
        
        joueur.haut(abs(pos_x - min));
    }

    // Déplacements vers le bas
    if (tableau_mvmnt[DOWN] == true) {
        
        joueur.bas(abs(pos_x - max));
    }
    
    /* NON FONCTIONNEL !!
    // roll droite
    if (tableau_mvmnt[ROLLD] == true) {
        
        joueur.rollDroite();
    }
    // roll gauche
    if (tableau_mvmnt[ROOLG] == true) {
       
        joueur.rollGauche();
    }
    NON FONCTIONNEL !!*/

    for (int i = 0; i < nbMissiles; i++)
    {
        if (missiles[i].visible)
        {
            missiles[i].lancement(missiles[i].getPosition(), missiles[i].getAngle());
        }
    }

    // Actualisation de la position et orientation camera
    mat4 rotation_x = matrice_rotation(joueur.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(joueur.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(joueur.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    mat4 rot = rotation_z * rotation_y * rotation_x;
    vec3 dir = rot * vec3(0,0,10.);
    vec3 up = rot * vec3(0,4.,0);

    cam.tr.translation = joueur.tr.translation - dir + up;
    cam.tr.rotation_center = cam.tr.translation;
    vec3 v = joueur.tr.rotation_euler;
    cam.tr.rotation_euler = vec3(v.x, -1*v.y, -1*v.z) + vec3(M_PI/12.,M_PI, 0);

    for (int i = 0; i < nb_obj; i++)
    {
            draw_obj3d(&objmur[i], cam);

    }

    // affichage du joueur
    draw_obj3d(&joueur, cam);

    // affichage des obstacles
    for (int j = 0; j < nbObstacles; j++)
        draw_obj3d(nouvObstacle+j, cam);

    // affichage du missiles
    for (int i = 0; i < nbMissiles; i++)
        draw_obj3d(missiles + i, cam);

    // affichage des anneaux
    for (int i = 0; i < nbring; i++)
        draw_obj3d(ring + i, cam);


    // ecrture du texte si ils sont en visible
    draw_text(&text_boost);
    draw_text(&text_vie);
    draw_text(&text_points);
    draw_text(&text_munition);
    draw_text(&message_fin);
    draw_text(&message_pause);

    glutSwapBuffers();
}
 

/*****************************************************************************\
* Gestion boutons souris                                                      *
\*****************************************************************************/
void mouse_callback(int button, int state, int x, int y)
{
    switch (button)
    {
        case GLUT_LEFT_BUTTON:
        {
            if (state == GLUT_DOWN)
            {
                joueur.tirerMissile(&missiles[joueur.getReserveMunition() - 1]);
                text_munition.value = "Munitions: " + std::to_string(joueur.getReserveMunition());
            }
            break;
        }
        case GLUT_RIGHT_BUTTON:
        {
            if (state == GLUT_DOWN)
            {
                joueur.utilisationBoost();
                text_boost.value = "Boost: " + std::to_string(joueur.getBoost());
            }
            break;
        }
    }
}

/*****************************************************************************\
* Gestion position souris                                                     *
\*****************************************************************************/
void passive_mouse_callback(int x, int y)
{
    pos_x = (float)x / 600;
    pos_y = (float)y / 600;
 
    if (pos_x > max)
    {
        tableau_mvmnt[RIGHT] = true;
    }
    else if (pos_x < min)
    {
        tableau_mvmnt[LEFT] = true;
    }
    else
    {
        tableau_mvmnt[RIGHT] = false;
        tableau_mvmnt[LEFT] = false;
    }

    if (pos_y > max)
    {
        tableau_mvmnt[UP] = true;
    }
    else if (pos_y < min)
    {
        tableau_mvmnt[DOWN] = true;
    }
    else
    {
        tableau_mvmnt[UP] = false;
        tableau_mvmnt[DOWN] = false;
    }
}

/*****************************************************************************\
* Gestion tir, boost et pause                                                 *
\*****************************************************************************/
void gestiontire(int)
{
    joueur.tire_autorisation = true;
}
void gestionboost(int)
{
    joueur.boost_autorisation = true;
}
void gestionpause(int)
{
    autorisation_pause = true;
}

/*****************************************************************************\
* Gestion du joystick                                                         *
\*****************************************************************************/
void joystick_callback(unsigned int buttons, int xaxis, int yaxis, int zaxis)
{
    float min = -150;
    float max = 150;

    switch (buttons)
    {
        case 0: // joystick gauche 
        {
            if (xaxis > 150)
            {
                joueur.tournerDroite(abs(xaxis - min) / (1000*2));
            }
            else if (xaxis < -150)
                joueur.tournerGauche(abs(xaxis - min) / (1000 * 2));

            if (yaxis > 150)
            {
                joueur.haut(abs(yaxis - min) / (1000 * 2));
            }
            else if (yaxis < -150)
                joueur.bas(abs(yaxis - min)/ (1000 * 2));

            /* NON FONCTIONNEL 
            if (zaxis > 150)
            {
                joueur.rollGauche();
            }
            else if (zaxis < -150)
                joueur.rollDroite();
            NON FONCTIONNEL */
            break;
        }
        case 1: // a xbox ou x ps
        {
            if (joueur.tire_autorisation)
            {
                joueur.tire_autorisation = false;
                joueur.tirerMissile(&missiles[joueur.getReserveMunition() - 1]);
                text_munition.value = "Munitions: " + std::to_string(joueur.getReserveMunition());
                glutTimerFunc(700, gestiontire, 0);
            }
            break;
        }
        case 8: // y xbox ou triangle ps
        {
            if (joueur.boost_autorisation)
            {
                joueur.boost_autorisation = false;
                joueur.utilisationBoost();
                text_boost.value = "Boost: " + std::to_string(joueur.getBoost());
                glutTimerFunc(100, gestionboost, 0);
            }
            break;
        }
        case 128 : // boutton exit
        {
            exit(0);
            break;
        }
        case 64: // pause 
            if (autorisation_pause)
            {
                autorisation_pause = false;
                pause = !pause;
                message_pause.visible = !message_pause.visible;
                glutTimerFunc(700, gestionpause, 0);
            }
            break;
        default:
            break;
    }
}

/*****************************************************************************\
* Gestion des appels clavier                                                  *
\*****************************************************************************/
void keyboard_callback(unsigned char key, int, int)
{
    switch (key)
    {
        // touche pour les captures d'écran
        case 'p':
            glhelper::print_screen();
            break;

        // touches generales d'arret du jeu
        case 'Q':
        case 27:
            exit(0);
            break;

        // sélection de la souris
        case 'm':
            if (debut_partie)
            {
                debut_partie = false;
                init();
                glutMouseFunc(mouse_callback);
                glutPassiveMotionFunc(passive_mouse_callback);
            }
            break;
        
        // selection du joystick
        case 'j':
            if (debut_partie)
            {
                debut_partie = false;
                init();
                glutTimerFunc(25, timer_callback, 0);
                glutJoystickFunc(joystick_callback, 25);
            }
            break;

        // mise en pause du jeu
        case 'b':
            pause = !pause;
            message_pause.visible = !message_pause.visible;
            break;

        // touches pour le joueur
        case 't':
            if (joueur.getVie() >0)
            {
                joueur.tirerMissile(&missiles[joueur.getReserveMunition() - 1]);
                text_munition.value = "Munitions: " + std::to_string(joueur.getReserveMunition());
            }
            break;

        /* NON FONCTIONNEL
        case 'q':
            tableau_mvmnt[ROOLG] = true;
            break;
        case 'd':
            tableau_mvmnt[ROLLD] = true;
            break;
        NON FONCTIONNEL */

        // barre espace pour le boost
        case 32:
            joueur.utilisationBoost();
            text_boost.value = "Boost: " + std::to_string(joueur.getBoost());
            break;
        case 13: // touche entrée /restart
            debut_partie = false;
            init();
            break;
    }
}

// NON FONCTIONNEL
// void keyboard_up_callback(unsigned char key, int, int)
// {
//     switch (key)
//     {
//         case 'q':
//             tableau_mvmnt[ROOLG] = false;
//             break;
//         case 'd':
//             tableau_mvmnt[ROLLD] = false;
//             break;
//         default:
//             break;
//     }
// }

/*****************************************************************************\
* special_callback                                                            *
\*****************************************************************************/
static void special_callback(int key, int,int)
{
    switch (key)
    {
        case GLUT_KEY_UP:
        tableau_mvmnt[UP] = true; //Autorisation de la rotation avec la touche du haut
        break;
        case GLUT_KEY_DOWN:
        tableau_mvmnt[DOWN] = true; //Autorisation de la rotation avec la touche du bas
        break;
        case GLUT_KEY_LEFT:
        tableau_mvmnt[LEFT] = true; //Autorisation de la rotation avec la touche de gauche
        break;
        case GLUT_KEY_RIGHT:
        tableau_mvmnt[RIGHT] = true; //Autorisation de la rotation avec la touche de droite
        break;
    
    }
}

static void special_up_callback(int key, int,int)
{
    switch (key)
    {
        case GLUT_KEY_UP:
            tableau_mvmnt[UP] = false; //Fin d'autorisation de la rotation avec la touche du haut
            break;
        case GLUT_KEY_DOWN:
            tableau_mvmnt[DOWN] = false; //Fin d'autorisation de la rotation avec la touche du bas
            break;
        case GLUT_KEY_LEFT:
            tableau_mvmnt[LEFT] = false; //Fin d'autorisation de la rotation avec la touche de gauche
            break;
        case GLUT_KEY_RIGHT:
            tableau_mvmnt[RIGHT] = false; //Fin d'autorisation de la rotation avec la touche de droite
            break;
    }
}

/*****************************************************************************\
* finPartie                                                                 *
\*****************************************************************************/
void finPartie()
{
    for (int j = 0; j < nbObstacles; j++)
        nouvObstacle[j].visible = false;
    for (int j = 0; j < 10; j++)
        missiles[j].visible = false;
    for (int j = 0; j < nb_obj; j++)
        objmur[j].visible = false;
    for (int j = 0; j < nbring; j++)
        ring[j].visible = false;
    joueur.visible = false;

    text_boost.visible = false;
    text_munition.visible = false;
    text_vie.visible = false;
    message_fin.visible = true;
}

/*****************************************************************************\
* timer_callback                                                              *
\*****************************************************************************/
static void timer_callback(int)
{
    glutTimerFunc(25, timer_callback, 0);
    if (pause)
    {
        for (int i = 0; i < sizeof(tableau_mvmnt); i++)
        {
            tableau_mvmnt[i] = false;
        }
    }
    else
    {
        // Déplacements vers l'avant
        joueur.avancer();

        for (int j = 0; j < nbObstacles; j++)
        {
            nouvObstacle[j].gestionApparitionObstacles();
            if (nouvObstacle[j].visible)
            {
                nouvObstacle[j].tomber();
                
                /*
                * Gestion des collisions
                */

                // gestion collisions missiles meteorites
                for (int i = 0; i < 100; i++)
                {
                    if (missiles[i].visible)
                    {
                        missiles[i].collisionObj(&nouvObstacle[j]);
                        missiles[i].miseAjour();
                        nouvObstacle[j].miseAjour();
                    }
                }

                // gestion collision entre le joueur et les meteorites
                joueur.collisionObj(&nouvObstacle[j]);
                joueur.miseAjour();
                nouvObstacle[j].miseAjour();
            }
        }

        for (int i = 0; i < nbring; i++)
        {
            ring[i].gestionApparitionObstacles();

            if (ring[i].visible)
            {
                vec3 posJoueur = joueur.getPosition();
                vec3 posRing = ring[i].getPosition();

                if (abs(posJoueur.x - posRing.x) <= 2. && abs(posJoueur.y - posRing.y) <= 2. && abs(posJoueur.z - posRing.z) <= 1.) // version precise 
                //if (joueur.distance(joueur.getPosition(), ring[i].getPosition()) < 0.7 ) // version plus generaliste
                {

                    joueur.setPoints(joueur.getPoints() + 1);
                    ring[i].visible = false;
                    text_points.value = "Points: " + std::to_string(joueur.getPoints());

                }
            }
        }
        text_vie.value = "Vie: " + std::to_string(joueur.getVie());
        if (joueur.getVie() == 0)
        {
            finPartie();
        }
    }
    glutPostRedisplay();
}


/*****************************************************************************\
* main                                                                         *
\*****************************************************************************/
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | MACOSX_COMPATIBILITY);
    glutInitWindowSize(600, 600);
    glutCreateWindow("OpenGL");

    
    glutDisplayFunc(display_callback_menu);

    // Fonction détectant la l'appuie d'une touche
    glutKeyboardFunc(keyboard_callback);
    glutSpecialFunc(special_callback);
    // Fonction détectant la relache d'une touche
    // glutKeyboardUpFunc(keyboard_up_callback);
     glutSpecialUpFunc(special_up_callback);
   
    glutTimerFunc(25, timer_callback_menu, 0);
    glewExperimental = true;
    glewInit();

    std::cout << "OpenGL: " << (GLchar *)(glGetString(GL_VERSION)) << std::endl;
   
    init_menu();
    glutMainLoop();

    return 0;
}

/*****************************************************************************\
* draw_text                                                                   *
\*****************************************************************************/
void draw_text(const text * const t)
{
    if(!t->visible){return;}
  
    glDisable(GL_DEPTH_TEST);
    glUseProgram(t->prog);

    vec2 size = (t->topRight - t->bottomLeft); 
    
    GLint loc_size = glGetUniformLocation(gui_program_id, "size"); CHECK_GL_ERROR();
    if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
    glUniform2f(loc_size,size.x, size.y);     CHECK_GL_ERROR();

    glBindVertexArray(t->vao);                CHECK_GL_ERROR();
  
    for(unsigned i = 0; i < t->value.size(); ++i)
    {
        GLint loc_start = glGetUniformLocation(gui_program_id, "start"); CHECK_GL_ERROR();
        if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
        glUniform2f(loc_start,t->bottomLeft.x+i*size.x, t->bottomLeft.y);    CHECK_GL_ERROR();

        GLint loc_char = glGetUniformLocation(gui_program_id, "c"); CHECK_GL_ERROR();
        if (loc_char == -1) std::cerr << "Pas de variable uniforme : c" << std::endl;
        glUniform1i(loc_char, (int)t->value[i]);    CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, t->texture_id);                            CHECK_GL_ERROR();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);                    CHECK_GL_ERROR();
    }
}

/*****************************************************************************\
* draw_obj3d                                                                  *
\*****************************************************************************/
void draw_obj3d(const objet3d* const obj, camera cam)
{
  if(!obj->visible){return;}

  glEnable(GL_DEPTH_TEST);
  glUseProgram(obj->prog);
  
  {
    GLint loc_projection = glGetUniformLocation(shader_program_id, "projection"); CHECK_GL_ERROR();
    if (loc_projection == -1) std::cerr << "Pas de variable uniforme : projection" << std::endl;
    glUniformMatrix4fv(loc_projection,1,false,pointeur(cam.projection));    CHECK_GL_ERROR();

    GLint loc_rotation_view = glGetUniformLocation(shader_program_id, "rotation_view"); CHECK_GL_ERROR();
    if (loc_rotation_view == -1) std::cerr << "Pas de variable uniforme : rotation_view" << std::endl;
    mat4 rotation_x = matrice_rotation(cam.tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(cam.tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(cam.tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(loc_rotation_view,1,false,pointeur(rotation_x*rotation_y*rotation_z));    CHECK_GL_ERROR();

    vec3 cv = cam.tr.rotation_center;
    GLint loc_rotation_center_view = glGetUniformLocation(shader_program_id, "rotation_center_view"); CHECK_GL_ERROR();
    if (loc_rotation_center_view == -1) std::cerr << "Pas de variable uniforme : rotation_center_view" << std::endl;
    glUniform4f(loc_rotation_center_view , cv.x,cv.y,cv.z , 0.0f); CHECK_GL_ERROR();

    vec3 tv = cam.tr.translation;
    GLint loc_translation_view = glGetUniformLocation(shader_program_id, "translation_view"); CHECK_GL_ERROR();
    if (loc_translation_view == -1) std::cerr << "Pas de variable uniforme : translation_view" << std::endl;
    glUniform4f(loc_translation_view , tv.x,tv.y,tv.z , 0.0f); CHECK_GL_ERROR();
  }
  {
    GLint loc_rotation_model = glGetUniformLocation(obj->prog, "rotation_model"); CHECK_GL_ERROR();
    if (loc_rotation_model == -1) std::cerr << "Pas de variable uniforme : rotation_model" << std::endl;
    
     mat4 rotation_x = matrice_rotation(obj->tr.rotation_euler.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(obj->tr.rotation_euler.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(obj->tr.rotation_euler.z, 0.0f, 0.0f, 1.0f);

   
    glUniformMatrix4fv(loc_rotation_model,1,false,pointeur(rotation_z*rotation_y*rotation_x));    CHECK_GL_ERROR();

    vec3 c = obj->tr.rotation_center;
    GLint loc_rotation_center_model = glGetUniformLocation(obj->prog, "rotation_center_model");   CHECK_GL_ERROR();
    if (loc_rotation_center_model == -1) std::cerr << "Pas de variable uniforme : rotation_center_model" << std::endl;
    glUniform4f(loc_rotation_center_model , c.x,c.y,c.z , 0.0f);                                  CHECK_GL_ERROR();

    vec3 t = obj->tr.translation;
    GLint loc_translation_model = glGetUniformLocation(obj->prog, "translation_model"); CHECK_GL_ERROR();
    if (loc_translation_model == -1) std::cerr << "Pas de variable uniforme : translation_model" << std::endl;
    glUniform4f(loc_translation_model , t.x,t.y,t.z , 0.0f);                                     CHECK_GL_ERROR();
  }
  glBindVertexArray(obj->vao);                                              CHECK_GL_ERROR();

  glBindTexture(GL_TEXTURE_2D, obj->texture_id);                            CHECK_GL_ERROR();
  glDrawElements(GL_TRIANGLES, 3*obj->nb_triangle, GL_UNSIGNED_INT, 0);     CHECK_GL_ERROR();
}

/*****************************************************************************\
* init_text                                                                   *
\*****************************************************************************/
void init_text(text *t){
  vec3 p0=vec3( 0.0f, 0.0f, 0.0f);
  vec3 p1=vec3( 0.0f, 1.0f, 0.0f);
  vec3 p2=vec3( 1.0f, 1.0f, 0.0f);
  vec3 p3=vec3( 1.0f, 0.0f, 0.0f);

  vec3 geometrie[4] = {p0, p1, p2, p3}; 
  triangle_index index[2] = { triangle_index(0, 1, 2), triangle_index(0, 2, 3)};

  glGenVertexArrays(1, &(t->vao));                                            CHECK_GL_ERROR();
  glBindVertexArray(t->vao);                                                  CHECK_GL_ERROR();

  GLuint vbo;
  glGenBuffers(1, &vbo);                                                      CHECK_GL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER,vbo);                                          CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,sizeof(geometrie),geometrie,GL_STATIC_DRAW);   CHECK_GL_ERROR();

  glEnableVertexAttribArray(0); CHECK_GL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); CHECK_GL_ERROR();

  GLuint vboi;
  glGenBuffers(1,&vboi);                                                      CHECK_GL_ERROR();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboi);                                 CHECK_GL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(index),index,GL_STATIC_DRAW);   CHECK_GL_ERROR();

  t->texture_id = glhelper::load_texture("data/fontB.tga");

  t->visible = true;
  t->prog = gui_program_id;
}

GLuint upload_mesh_to_gpu(const mesh& m)
{
  GLuint vao, vbo, vboi;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1,&vbo);                                                       CHECK_GL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER,vbo); CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,m.vertex.size()*sizeof(vertex_opengl),&m.vertex[0],GL_STATIC_DRAW); CHECK_GL_ERROR();

  glEnableVertexAttribArray(0); CHECK_GL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0);  CHECK_GL_ERROR();

  glEnableVertexAttribArray(1); CHECK_GL_ERROR();
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

  glEnableVertexAttribArray(2); CHECK_GL_ERROR();
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2*sizeof(vec3))); CHECK_GL_ERROR();

  glEnableVertexAttribArray(3); CHECK_GL_ERROR();
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3*sizeof(vec3))); CHECK_GL_ERROR();

  glGenBuffers(1,&vboi); CHECK_GL_ERROR();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboi); CHECK_GL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,m.connectivity.size()*sizeof(triangle_index),&m.connectivity[0],GL_STATIC_DRAW); CHECK_GL_ERROR();

  return vao;
}

void init_model_joueur()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/f15.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = .5f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.0f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,transform);

  // Centre la rotation du modele 1 autour de son centre de gravite approximatif
  joueur.tr.rotation_center = vec3(0.0f,0.0f,0.0f);
 
  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  joueur.vao = upload_mesh_to_gpu(m);

  joueur.nb_triangle = m.connectivity.size();
  joueur.texture_id = glhelper::load_texture("data/f15.tga");
  joueur.visible = true;
  joueur.prog = shader_program_id;

  joueur.tr.translation = vec3(-2.0, 5.0, -10.0);
  joueur.setPosition(joueur.tr.translation);
  joueur.setAngle(joueur.tr.rotation_euler);
}

// float relief(int coord_x, int coord_y, float m_seed)
// {
//     float x = coord_x + 1.0f  / 10 - 0.5f,
//         y = coord_y + 1.0f  / 10 - 0.5f;
//     return 2.f * std::pow(scaled_octave_noise_3d(4.f, 0.15f, 1.f / 6.f, 4.f, 0.f, 1.f, x, y, m_seed), 3.f);
// }

void init_model_sol()
{
    mesh m;

    vec3 p0 = vec3(-400.0f, 0.0, -400.0f);
    vec3 p1 = vec3(400.0f, 0.0, -400.0f);
    vec3 p2 = vec3(400.0f, 0.0, 400.0f);
    vec3 p3 = vec3(-400.0f, 0.0, 400.0f);

    //normales pour chaque sommet
    vec3 n0 = vec3(0.0f, 1.0f, 0.0f);
    vec3 n1 = n0;
    vec3 n2 = n0;
    vec3 n3 = n0;

    //couleur pour chaque sommet
    vec3 c0 = vec3(1.0f, 1.0f, 1.0f);
    vec3 c1 = c0;
    vec3 c2 = c0;
    vec3 c3 = c0;

    //texture du sommet
    vec2 t0 = vec2(0.0f, 0.0f);
    vec2 t1 = vec2(1.0f, 0.0f);
    vec2 t2 = vec2(1.0f, 1.0f);
    vec2 t3 = vec2(0.0f, 1.0f);

    vertex_opengl v0 = vertex_opengl(p0, n0, c0, t0);
    vertex_opengl v1 = vertex_opengl(p1, n1, c1, t1);
    vertex_opengl v2 = vertex_opengl(p2, n2, c2, t2);
    vertex_opengl v3 = vertex_opengl(p3, n3, c3, t3);

    m.vertex = { v0, v1, v2, v3 };

    //indice des triangles
    triangle_index tri0 = triangle_index(0, 1, 2);
    triangle_index tri1 = triangle_index(0, 2, 3);
    m.connectivity = { tri0, tri1 };

    objmur[0].nb_triangle = 2;
    objmur[0].vao = upload_mesh_to_gpu(m);
            
    objmur[0].texture_id = glhelper::load_texture("data/grass.tga");
        
    objmur[0].visible = true;
    objmur[0].prog = shader_program_id;
}
 
void init_model_obstacle()
{
  // Chargement d'un maillage a partir d'un fichier
  mesh m = load_obj_file("data/lune.obj");

  // Affecte une transformation sur les sommets du maillage
  float s = 0.9f;
  mat4 transform = mat4(   s, 0.0f, 0.0f, 0.0f,
      0.0f,    s, 0.0f, 0.50f,
      0.0f, 0.0f,   s , 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
  apply_deformation(&m,matrice_rotation(M_PI/2.0f,1.0f,0.0f,0.0f));
  apply_deformation(&m,matrice_rotation(M_PI,0.0f,1.0f,0.0f));
  apply_deformation(&m,transform);

  update_normals(&m);
  fill_color(&m,vec3(1.0f,1.0f,1.0f));

  nouvObstacle[0].vao = upload_mesh_to_gpu(m);
  
  nouvObstacle[0].nb_triangle = m.connectivity.size();
  nouvObstacle[0].texture_id = glhelper::load_texture("data/luneBump.tga");

  nouvObstacle[0].visible = false;
  nouvObstacle[0].prog = shader_program_id;

  nouvObstacle[0].tr.translation = vec3(2.0, 10.0, -10.0);
  nouvObstacle[0].setPosition(nouvObstacle[0].tr.translation);
}

void init_model_missile()
{
    mesh m = load_obj_file("data/missile2.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.3f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, 0.50f,
        0.0f, 0.0f, s, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, matrice_rotation(M_PI / 2.0f, 1.0f, 0.0f, 0.0f));
    apply_deformation(&m, matrice_rotation(M_PI, 0.0f, 1.0f, 0.0f));
    apply_deformation(&m, transform);

    update_normals(&m);
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));
    missiles[0].vao = upload_mesh_to_gpu(m);
    missiles[0].nb_triangle = m.connectivity.size();
    missiles[0].texture_id = glhelper::load_texture("data/missile.tga");

    missiles[0].visible = false;
    missiles[0].prog = shader_program_id;
}

/*****************************************************************************\
* Ring                                                                        *
\*****************************************************************************/
void init_model_ring()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/Ring.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 5.f;
    mat4 transform = mat4(  s, 0.0f, 0.0f, 0.0f,
                            0.0f, s, 0.0f, 0.0f,
                            0.0f, 0.0f, s, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f
                         );
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    joueur.tr.rotation_center = vec3(0.0f, 0.0f, 0.0f);

    update_normals(&m);
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    ring[0].vao = upload_mesh_to_gpu(m);

    ring[0].nb_triangle = m.connectivity.size();
    ring[0].texture_id = glhelper::load_texture("data/ring.tga");
    ring[0].visible = false;
    ring[0].prog = shader_program_id;

    ring[0].tr.translation = vec3(-2.0, 1.0, -8.0);
    ring[0].setPosition(ring[0].tr.translation);
}