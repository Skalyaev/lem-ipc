# <p align="center">lem-ipc</p>

> L'objectif est que les joueurs, regroupés en équipes, s'affrontent sur un plateau en 2D.
> 
> Pour qu'une équipe soit victorieuse, elle doit être la dernière à rester sur le plateau.
>
> Lorsqu'un joueur meurt, il disparaît du plateau.
> Pour qu'un joueur soit éliminé, il doit être touché par au moins 2 joueurs de la même équipe.
> Cela signifie qu'il doit y avoir un joueur sur une case adjacente à la sienne (les diagonales comptent).
>
> Quand un joueur réalise qu'il est entouré par au moins 2 joueurs d'une autre même autre équipe,
> il doit quitter le plateau et mettre fin à son exécution.
>
> Une case du plateau ne peut contenir qu'un seul joueur à la fois.
>
> - Chaque client est un processus et il ne doit y avoir qu'un seul exécutable.
>   Cela signifie que le premier joueur doit créer les ressources partagées (shm, msgq, sémaphores).
> - De la même manière, lorsqu'un joueur quitte le jeu,
>   il doit s'assurer qu'il est le dernier joueur sur le plateau.
>   Si c'est le cas, il doit nettoyer toutes les ressources IPC créées par le premier joueur
>   pour qu'elles ne restent pas en mémoire
> - Le plateau doit être stocké dans un segment de mémoire partagée (SHM).
>   Chaque joueur peut consulter le contenu du plateau,
>   mais ils doivent respecter les contraintes liées aux ressources partagées
>   et à l'accès compétitif (via des sémaphores).
> - Un joueur ne peut communiquer avec les autres joueurs qu'à travers une MSGQ.
> - Sur la carte, un joueur peut voir si une case est vide ou occupée par un autre joueur.
>   Si elle est occupée, le numéro de l'équipe du joueur y figurera.
>   Cependant, il est impossible de distinguer les joueurs d'une même équipe.

## Install

```bash
mkdir -p ~/.local/src
mkdir -p ~/.local/bin

apt update -y
apt install -y git
apt install -y make
apt install -y gcc
```

```bash
cd ~/.local/src
git clone https://github.com/Skalyaev/lem-ipc.git
cd lem-ipc && make

ln -s $PWD/lem-ipc ~/.local/bin/lem-ipc
export PATH=~/.local/bin:$PATH

lem-ipc
```
