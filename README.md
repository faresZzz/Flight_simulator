# Flight Simulator School Project

Ce projet est un projet que nous avons réalisé pour l'écoles par binôme pendant un durée de 1 semaine.
Ce projet avait pour objectif de nous faire découvrir la programmation la synthèse d'image en passant par le **GPU** en utilisant **OpenGl** ainsi **C++**

Nous avons donc décidé de de réaliser un simulateur de vol. 
Dans ce projet nous avons découvert les moteurs de rendu graphique, mais nous avons également du prendre en compte et mettre en places les contraintes sur les lois de la physique, le rendu d'ombre, la generation de terrain.


## Pre-requis
ATTENTION SI IL MANQUE LES LIBRAIRIES **GLUT** ET **GLEW**, COPIER COLLER LE DOSSIER EXTERN DES TUTORIAUX DANS LE DOSSIER PRINCIPAL !

### Compilation

Pour compiler et executer à partir du CMakeLists.txt en ligne de commande (Linux et macOs)

```sh
mkdir build
cd build
cmake ..
make
cd ..
./build/projet
```

ou 

```sh
mkdir build
cmake . -B build
make -C ./build && ./build/projet
```

**Testé sur windows avec Visual Studio Community 2019 (MSVC2019)**

## Fabriqué avec

Programme réalisé en utilisant [Visual Studio Community 2022](https://visualstudio.microsoft.com/fr/vs/community/) sous Python 3.9.7


## Versions

**Dernière version stable :** 1.0
**Dernière version :** 1.0

## Auteurs

* **Alexandre Burlot** _alias_ [@bubuoreo](https://github.com/bubuoreo)
* **Farès Zaghouane** _alias_ [@faresZzz](https://github.com/faresZzz)


## Licence
All code are released under GPLv3+ licence. The associated documentation and other content are released under [CC-BY-SA](http://creativecommons.org/licenses/by-sa/4.0/).
