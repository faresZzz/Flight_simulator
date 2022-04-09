#pragma once
// Fichier d'en t�te classe Joueur


#include "declaration.h"
#include <random>

struct Super : public objet3d
{
    float vitesse;
    vec3 position;
    vec3 angle;
    bool collision;
    
    vec3 getPosition();
    void setPosition(vec3 position);

    vec3 getAngle();
    void setAngle(vec3 angle);
    float distance(vec3 position1, vec3 position2);
    void collisionObj(Super *obj);
    void miseAjour();
};


// MISSILE
struct Missile : public Super
{
    float vitesse = 2.;
    /*vec3 angle;
    vec3 position;

    vec3 getPosition();
    void setPosition(vec3 position);

    vec3 getAngle();
    void setAngle(vec3 angle);*/

    void lancement(vec3 positiondepart, vec3 angledepart);
    
};

// JOUEUR
struct Joueur : public Super
{
    int vie = 10;
    int boost = 5;
    int points = 0;
    float timerBoost;
    float vitesseNormale = 0.5; 
    float vitesseBoost = 1.;
    float vitesse = vitesseNormale;
    
   
    float dL = 0.5f;
    float d_angle = 0.1f;
    bool tire_autorisation = true;
    bool boost_autorisation = true;
    int reserveMunition = 100;
   /* vec3 position ;
    vec3 angle;*/

    int getVie();
    bool setVie(int nvie);
    int getPoints();
    void setPoints(int point);
    int getBoost();
    bool setBoost(int nboost);

    //vec3 getPosition();
    void setPosition(vec3 position);

    //vec3 getAngle();
    //void setAngle(vec3 angle);

    int getReserveMunition();
    float getVitesse();
    void setVitesse(float nvitesse);
    void utilisationBoost();
    void tournerDroite(float coef);
    void tournerGauche(float coef);
    void haut(float coef);
    void bas(float coef);
    void rollDroite();
    void rollGauche();
   
    void avancer();
    void tirerMissile(Missile *minition);
    void miseAjour();
};


// OBSTACLES
struct Obstacles : public Super
{
    float vitesse = 0.1;
    /*vec3 position;
    vec3 angle;*/

    //vec3 getPosition();
    void setPosition(vec3 position);

    //vec3 getAngle();
    //void setAngle(vec3 angle);

    void tomber();
    void gestionApparitionObstacles();

};

struct Ring : public Obstacles
{
    void gestionApparitionObstacles();
};



