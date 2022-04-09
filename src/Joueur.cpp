// Implementation classe Joueur

#include "Joueur.h"
#include <iostream>
#include <thread>
#include <chrono>

// generateur de nb aleatoires en float entre 0 - 1
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);


// SUPER
vec3 Super::getPosition()
{
    return this->position;
}

void Super::setPosition(vec3 position)
{
    this->position = position; 
    this->tr.translation = this->position;
}

vec3 Super::getAngle()
{
    return this->angle;
}

void Super::setAngle(vec3 angle)
{
    this->angle = angle;
    this->tr.rotation_euler = this->angle;
}

float Super::distance(vec3 position1, vec3 position2)
{
    return sqrtf(pow((position1.x - position2.x), 2) + pow((position1.y - position2.y), 2) + pow((position1.z - position2.z), 2));
}

void Super::collisionObj(Super *obj)
{
    if (this->distance(this->position, obj->getPosition()) < 2.0)
    {
        this->collision = true;
        obj->collision = true;
    }
}

void Super::miseAjour()
{
    if (this->collision)
    {
        this->visible = false;
        this->collision = false;
    }
}


// Joueur
// getter setter
int Joueur :: getVie()
{
    return this->vie;
}
bool Joueur :: setVie(int nvie)
{
    this->vie = nvie;
    return true;
}

int Joueur::getPoints()
{
    return this->points;
}

void Joueur::setPoints(int point)
{
    if (point > 0)
        this->points = point;
}



int Joueur :: getBoost()
{
    return this->boost;
}
bool Joueur :: setBoost(int nboost)
{
    this->boost = nboost;
    return true;
}

float Joueur :: getVitesse()
{
    return this->vitesse;
}
void Joueur :: setVitesse(float nvitesse)
{
    
    this->vitesse = nvitesse;
}

void Joueur::setPosition(vec3 position)
{
    if (abs(position.x) >= 250.)
    {   
        position.x = -position.x;
    }
    if (position.y <= 0.)
    {
        this->setVie(0);
    }
    if (abs(position.z) >= 250.)
    {
        position.z = -position.z;
    }

    this->position = position;
    this->tr.translation = this->position;
}

int Joueur::getReserveMunition()
{
    return this->reserveMunition;
}

// actions
void Joueur :: utilisationBoost()
{
    if (this->boost)
    {
        this->setBoost(this->boost-1);
        setVitesse(this->vitesseBoost);
        std::thread m_thread;
        m_thread = std::thread([&]() {
            auto delta = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
            std::this_thread::sleep_until(delta);
            setVitesse(this->vitesseNormale);
            });
        m_thread.detach(); 
    }
}

void Joueur :: avancer()
{
    angle = this->getAngle();
   
     /*mat4 rotation_x = matrice_rotation(angle.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(angle.y, 0.0f, 1.0f, 0.0f);*/
    //mat4 rotation_z = matrice_rotation(angle.z, cam_z.x, cam_z.y, cam_z.z);
    mat4 rotation_z = matrice_rotation(angle.z, 0.0f, 0.0f, 1.0f);
    vec3 y_obj = rotation_z * vec3(0, 1, 0);
    mat4 rotation_y = matrice_rotation(angle.y, y_obj.x, y_obj.y, y_obj.z);

    vec3 x_obj = rotation_y * vec3(1,0,0);
    mat4 rotation_x = matrice_rotation(angle.x, x_obj.x, x_obj.y, x_obj.z);
    mat4 rot = rotation_x * rotation_y * rotation_z;
    vec3 dir = rot * vec3(0, 0, 1);

    this->setPosition(this->getPosition() += this->vitesse * dir);

    
}
void Joueur :: tournerDroite(float coef)
{
   
    this->tr.rotation_euler.y -= this->d_angle * coef;
    this->angle = this->tr.rotation_euler;
}
void Joueur :: tournerGauche(float coef)
{
    this->tr.rotation_euler.y += this->d_angle * coef;
    this->angle = this->tr.rotation_euler;

}
void Joueur :: haut(float coef)
{
    this->tr.rotation_euler.x -= this->d_angle * coef;
    this->angle = this->tr.rotation_euler;

}
void Joueur :: bas(float coef)
{
    this->tr.rotation_euler.x += this->d_angle * coef;
    this->angle = this->tr.rotation_euler;

}

void Joueur::rollDroite()
{
    this->tr.rotation_euler.z = std :: fmod(this->tr.rotation_euler.z + this->d_angle , 2*M_PI);
    this->angle = this->tr.rotation_euler;
}

void Joueur::rollGauche()
{
    this->tr.rotation_euler.z = std::fmod(this->tr.rotation_euler.z - this->d_angle, 2 * M_PI);
    this->angle = this->tr.rotation_euler;
}

void Joueur :: tirerMissile(Missile *minition)
{
    if (this->reserveMunition > 0)
    {
        this->reserveMunition--;
       minition->lancement(this->getPosition(), this->getAngle());
       minition->setAngle(this->getAngle());
    }

}

void Joueur::miseAjour()
{
    if (this->collision)
    {
        this->collision = false;
        this->vie -= 1;
        if (this->vie == 0)
        {
            this->visible = false;
        }
        
    }
}

/*****************************************************************************\
* Obstacles                                                                   *
\*****************************************************************************/
void Obstacles::setPosition(vec3 position)
{
    if (position.y < 1)
        this->visible = false;
    else
    {
        this->position = position;
        this->tr.translation = this->position;
    }
}

void Obstacles ::tomber()
{
    /*this->tr.rotation_euler.x += ((double)rand() / (RAND_MAX))/10;
    this->tr.rotation_euler.y += ((double)rand() / (RAND_MAX))/10;
    this->tr.rotation_euler.z += ((double)rand() / (RAND_MAX))/10;*/

    this->setAngle(this->angle + vec3(distribution(generator) / 10, distribution(generator) / 10, distribution(generator) / 10));

    
    mat4 rotation_x = matrice_rotation(this->angle.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(this->angle.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(this->angle.z, 0.0f, 0.0f, 1.0f);
   
   
    this->setPosition(this->getPosition() - this->vitesse * vec3(0, 1, 0));
    this->tr.translation -= this->vitesse* vec3(0, 1, 0);
    if (this->tr.translation.y < 0)
    {
        this->visible = false;
    }
}

void Obstacles :: gestionApparitionObstacles()
{
    if (!this->visible) // si l'obstacle n'est pas visible on tire au sort le fait de le faire appartaitre, en lui donnant une position
    {
        float de = distribution(generator);  // generates number in the range 1..6 

        if (de > 0.7)
        {
            this->visible = true;
            float x = (distribution(generator) * 500.0) - 250.0;
            float z = (distribution(generator) * 500.0) - 250.0;
            float y = (distribution(generator) * 50) + 100;
            this->tr.rotation_euler = 500*vec3(M_PI/x,M_PI/y,M_PI/z);
            setPosition(vec3(x, y, z));

        }
    }
}

/*****************************************************************************\
* Missiles                                                                    *
\*****************************************************************************/
void Missile::lancement(vec3 positiondepart, vec3 angledepart)
{
    this->visible = true;
    
    this->setPosition( positiondepart);
    this->setAngle(angledepart);
   // mat4 rotation_x = matrice_rotation(angledepart.x, 1.0f, 0.0f, 0.0f);
    mat4 rotation_y = matrice_rotation(angledepart.y, 0.0f, 1.0f, 0.0f);
    mat4 rotation_z = matrice_rotation(angledepart.z, 0.0f, 0.0f, 1.0f);
    vec3 x_obj = rotation_y * vec3(1, 0, 0);
    mat4 rotation_x = matrice_rotation(angledepart.x, x_obj.x, x_obj.y, x_obj.z);
    mat4 rot = rotation_x * rotation_y * rotation_z;
    vec3 dir = rot * vec3(0, 0, 1);
    this->setPosition(this->getPosition() + this->vitesse * dir);

}

/*****************************************************************************\
* Ring                                                                        *
\*****************************************************************************/
void Ring::gestionApparitionObstacles()
{
    bool lastvisible = this->visible;
    this->Obstacles::gestionApparitionObstacles();
    if (visible && !lastvisible)
    {
        float y = (distribution(generator) * 20.0)+ 2. ;
        this->setPosition(vec3(this->getPosition().x, y, this->getPosition().z));
    }
        
}
