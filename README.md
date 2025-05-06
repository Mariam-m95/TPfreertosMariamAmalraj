# TP FreeRTOS Mariam Amalraj

## 0. (Re)prise en main

### 0.1 Premiers pas

1. Le fichier main.c se trouve dans Core/Src/main.c
2. Ceux sont des balises qui permettend d'indiquer à l'utilisateur les endroits dans lesquels il faut écrire, si on écrit en dehors des balises, le code écrit s'efface lorsqu'on doit regénerer le code.
3. HAL_Delay(temps) et HAL_GPIO_TogglePin(GPIO_Port,GPIO_Pin,GPIO_State)
4. Ils sont définis dans le main.h
5. Voici le code pour faire clignoter la LED :
![clignoter LED](https://github.com/user-attachments/assets/e6e1ea58-7673-4426-8af5-43f22a902f49)

6. Voici le code pour faire allumer la LED lorsqu'on appuie dessus:
![allume led bouton](https://github.com/user-attachments/assets/c893b228-56c7-4cd4-8df1-8df5a0007fbb)


## 1. FreeRTOS, tâches et sémaphores

### 1.1 Tâche simple

1. On active FreeRTOS avec la version CMSIS V2.
TOTAL_HEAP_SIZE, qui est de 15360 bytes, définit la taille totale de la mémoire dynamique que FreeRTOS peut utiliser pour créer des tâches, des files d'attente, des sémaphores, etc.
Le management de la mémoire est heap_4

2. La macro portTICK_PERIOD_MS permet de convertir des millisecondes en "ticks" du système FreeRTOS. Voici le code pour faire clignoter la LED toutes les 100ms. 

![BlinkTask](https://github.com/user-attachments/assets/8fe595c6-f6e4-4d8d-b314-4f43a5499690)
![Xtaskcreate](https://github.com/user-attachments/assets/afbb2587-fa6c-4289-acd5-cf90f2618815)


### 1.2 Sémaphores pour la synchronisation

3. On crée 2 tâches, avec 2 priorités différentes.TaskGive donne un sémaphore toutes les 100ms.
![SemaphahoreCreate](https://github.com/user-attachments/assets/293aaca9-81b6-4feb-a123-5cb9d67c0b23)
![TaskGive/Take](https://github.com/user-attachments/assets/26cdac09-6905-406a-b0b5-0c6b0f79cb4c)
![Task Résultat](https://github.com/user-attachments/assets/72c83bdd-3c2f-4987-9e4d-c8a4267bd074)

Pour tester le mécanisme d'erreur, on lance le taskGive avec un délai au dessus de 1 sec, soit le temps après lequel on fait le reset software.

6.Quand taskGive faisait xSemaphoreGive(), FreeRTOS interrompait immédiatement taskGive pour faire tourner taskTake.
Maintenant : taskGive prioritaire
Quand taskGive donne le sémaphore, taskTake devient "prête à s'exécuter" (car elle attendait le sémaphore), mais ne s'exécute pas immédiatement. 

![Résultat changemt de priorité](https://github.com/user-attachments/assets/86504e3f-24be-4069-b6c3-66659c8216e3)

### 1.3 Notification

7. On utilise des task notifications au lieu de sémpahores :

![TaskGiveNotif](https://github.com/user-attachments/assets/25083b87-0a71-4827-86dd-97e0390a7218)
![Résultat Notif](https://github.com/user-attachments/assets/ac20d529-5f00-48d7-827d-37da5225891c)

### 1.4 Queues

![image](https://github.com/user-attachments/assets/4874108e-ac8a-4648-845c-803f1b921e3a)
![image](https://github.com/user-attachments/assets/6ed313f1-a47f-499e-83e2-44ba9726e105)

On récupère dans la queue, la valeur dans la tache take. La valeur est envoyée dans la queue dans la tache give.
### 1.5 Réentrance et exclusion mutuelle

10. Ici on observe ceci :

![image](https://github.com/user-attachments/assets/bda20e30-07d1-406b-bad2-6e0527bda6c9)

La fonction printf utilise une ressource partagée. Les sorties de printf peuvent être mélangées.
On utilise alors le mutex pour protéger l'accès à printf : 

![image](https://github.com/user-attachments/assets/12fd6d3b-cbe3-441e-8315-b4db1f66e14b)

Le mutex protège bien la fonction printf et le résultat est cohérent avec le code qu'on a copié.

## 2. On joue avec le Shell

1. On intégre les fichiers shell.c/.h et drv_uart1.c/.h pour pouvoir utiliser le shell.
Pour que la fonction qui exécute le shell fonctionne, on utilise une interruption avec un sémpahore. Une interruption se crée lorsque l'on entre du texte dans le shell. On crée le sémaphore binaire dans shell_init:

![shell_init](https://github.com/user-attachments/assets/a7097965-b688-45b6-8deb-88efd5e12383)

On a ensuite une fonction uart_read qui va générer une interruption lorsqu'il y aura une entrée dans le shell, qui va recevoir un sémaphore pour ensuite retourner un caractère. Le sémaphore est donné par la fonction shell_uart_receive_irq avec xSemaphoreGiveFromISR, et qui va ensuite faire un changement de contexte immédiat si une tâche plus prioritaire a été débloquée par ce sémaphore.

4. On doit créer 2 tâches, une pour le shell_run, une pour la led et on ajoute la fonction dans les fonctions shell avec task_shell.
   
![image](https://github.com/user-attachments/assets/ee6ced62-f1f5-4b46-ab11-e8642fc201fd)
![image](https://github.com/user-attachments/assets/82ef2d4e-d948-42d0-9cec-722a55cb0752)
![image](https://github.com/user-attachments/assets/02b8714e-b67e-49bb-be45-e9d387d2358c)

4. On crée la fonction spam pour la tache et le shell :

![image](https://github.com/user-attachments/assets/9cd51052-0d1d-4d22-a42b-075f8456dc15)
![image](https://github.com/user-attachments/assets/b6b1c313-359f-4ef6-be41-08a8e9dd9bcd)

## 3. Debug, gestion d'erreur et statistiques

### 3.1 Gestion du tas

1. C'est le HEAP
2. FreeRTOS
3. Voici la fonction Error_handler():

![image](https://github.com/user-attachments/assets/dbe6ced4-c7d0-4470-8c20-429faaa76e22)

4. Taille RAM initiale :

![image](https://github.com/user-attachments/assets/33e749ea-0851-480b-8881-4850ca153cae)

5. On crée une tâche bidon pour avoir des erreurs:

![image](https://github.com/user-attachments/assets/28849a55-e10a-46a1-ace4-b5172061228a)
![image](https://github.com/user-attachments/assets/02a792b2-c5a3-4c38-957f-08a44702f48d)

La taille de la mémoire ne change pas, mais le bss et data augmentent :

![image](https://github.com/user-attachments/assets/62c9156d-c0d1-406e-a2e1-0dfc486f5ef2)

7. on augmente la taille à 30720 bytes et voici la nouvelle utilisation mémoire :

![image](https://github.com/user-attachments/assets/6aee2597-aca8-4a17-a997-a2688c293613)

FreeRTOS alloue un tableau statique de la taille configurée dans la RAM, ce qui explique pourquoi on passe de 5 à 10%.

### 3.2 Gestion des piles

2. On configure CHECK_FOR_SYACK_OVERFLOW avec la méthode 2

3. On écrit la fonction vApplicationStackOVERFLOW :
![image](https://github.com/user-attachments/assets/2ebfc6af-ffbe-47ca-adeb-39b0df2410dc)

4.Voici ce qu'on obtient : 

![image](https://github.com/user-attachments/assets/9776e636-cf7f-48b7-8d77-7423393d8cda)

Ca affiche bien le message de vApplicationStackOverflow et celui de Error_Handler.

5. Il existe plusieurs hook :
- vApplicationIdleHook : appelé lorsque le système ne trouve plus de tâches à exécuter.
- vApplicationTickHook : appelé à chaque tick du système.


### 3.3 Statistiques dans l'IDE

6. Pour afficher le taux d'utilisation du CPU, on ajoute le TIMER 6 et on active l'interruption sur ce timer, puis on ajoute le code suivant:

![image](https://github.com/user-attachments/assets/299d397a-cbaf-4718-899d-de539eee8fb3)
![image](https://github.com/user-attachments/assets/86c1d677-98e8-45a2-959f-04176c4eef89)

On affiche le Run Time avec des scénarios différents : 

![image](https://github.com/user-attachments/assets/50fc17d1-d3df-4f16-b9cd-2d35b5154866)


### 3.4 Affichage des statistiques dans le shell

On affiche les stats dans le shell avec ce code :

![image](https://github.com/user-attachments/assets/8c5a7043-0169-477e-93fd-5f3e9c8ca776)

et voici ce qu'on obtient :

![image](https://github.com/user-attachments/assets/c6cf84e6-13a3-4fc7-a3c2-76268373c3f8)

