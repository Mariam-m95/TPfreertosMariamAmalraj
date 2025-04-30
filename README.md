# TP FreeRTOS Mariam Amalraj

## 0. (Re)prise en main

### 0.1 Premiers pas

1. Le fichier main.c se trouve dans Core/Src/main.c
2. Ceux sont des balises qui permettend d'indiquer à l'utilisateur les endroits dans lesquels il faut écrire, si on écrit en dehors des balises, le code écrit s'efface lorsqu'on doit regénerer le code.
3. HAL_Delay(temps) et HAL_GPIO_TogglePin(GPIO_Port,GPIO_Pin,GPIO_State)
4. Ils sont définis dans le main.h
5. f
6. f
![allume led bouton](https://github.com/user-attachments/assets/c893b228-56c7-4cd4-8df1-8df5a0007fbb)


## 1. FreeRTOS, tâches et sémaphores

### 1.1 Tâche simple

1. On active FreeRTOS avec la version CMSIS V2.
TOTAL_HEAP_SIZE, qui est de 15360 bytes, définit la taille totale de la mémoire dynamique que FreeRTOS peut utiliser pour créer des tâches, des files d'attente, des sémaphores, etc.
On peut avoir au maximum 56 priorités. La taille de la Queue Register est de 8.

2.![image](https://github.com/user-attachments/assets/8fe595c6-f6e4-4d8d-b314-4f43a5499690)
![image](https://github.com/user-attachments/assets/afbb2587-fa6c-4289-acd5-cf90f2618815)
La macro portTICK_PERIOD_MS permet de convertir des millisecondes en "ticks" du système FreeRTOS.

### 1.2 Sémaphores pour la synchronisation

3. ![image](https://github.com/user-attachments/assets/293aaca9-81b6-4feb-a123-5cb9d67c0b23)
4. Pour tester le mécanisme d'erreur, on lance le taskGive avec un délai au dessus de 1 sec, soit le temps après lequel on fait le reset software.
![image](https://github.com/user-attachments/assets/26cdac09-6905-406a-b0b5-0c6b0f79cb4c)
5.![sortie](https://github.com/user-attachments/assets/72c83bdd-3c2f-4987-9e4d-c8a4267bd074)
6.Quand taskGive faisait xSemaphoreGive(), FreeRTOS interrompait immédiatement taskGive pour faire tourner taskTake.
Maintenant : taskGive prioritaire
Quand taskGive donne le sémaphore, taskTake devient "prête à s'exécuter" (car elle attendait le sémaphore), mais ne s'exécute pas immédiatement. 
![image](https://github.com/user-attachments/assets/86504e3f-24be-4069-b6c3-66659c8216e3)

### 1.3 Notification

![image](https://github.com/user-attachments/assets/25083b87-0a71-4827-86dd-97e0390a7218)
![image](https://github.com/user-attachments/assets/ac20d529-5f00-48d7-827d-37da5225891c)

### 1.4 Queues

![image](https://github.com/user-attachments/assets/4874108e-ac8a-4648-845c-803f1b921e3a)
![image](https://github.com/user-attachments/assets/656c0c22-5e09-4fdb-9f44-17c6580d0bfc)
![image](https://github.com/user-attachments/assets/6ed313f1-a47f-499e-83e2-44ba9726e105)


### 1.5 Réentrance et exclusion mutuelle

10. Ici on observe ceci :
![image](https://github.com/user-attachments/assets/bda20e30-07d1-406b-bad2-6e0527bda6c9)
La fonction printf utilise une ressource partagée (la sortie série, en général), ce qui signifie que plusieurs tâches peuvent vouloir y accéder simultanément pour afficher des informations à l'écran ou dans la console. Les sorties de printf peuvent être mélangées, c'est-à-dire que la sortie de l'une des tâches pourrait être partiellement écrite avant que l'autre tâche commence à afficher sa propre sortie.

On utilise le mutex pour protéger l'accès à printf
![image](https://github.com/user-attachments/assets/12fd6d3b-cbe3-441e-8315-b4db1f66e14b)

## 2. On joue avec le Shell

1. On intégre les fichiers shell.c/.h et drv_uart1.c/.h pour pouvoir utiliser le shell.
1.1. sdsd
1.2. fzef
1.3.fd
1.4sdsds
1.5sdsds
2.
3. On doit créer 2 tâches, une pour le shell_run, une pour la led.
![image](https://github.com/user-attachments/assets/ee6ced62-f1f5-4b46-ab11-e8642fc201fd)
![image](https://github.com/user-attachments/assets/82ef2d4e-d948-42d0-9cec-722a55cb0752)
![image](https://github.com/user-attachments/assets/02b8714e-b67e-49bb-be45-e9d387d2358c)

4.
![image](https://github.com/user-attachments/assets/e7486ac9-aea9-447f-8769-f3b09f7a4e2d)
ca bloque
![image](https://github.com/user-attachments/assets/3ea7819d-836c-42a7-8200-5c6059432e84)


![alt text](image.png)
///////////////////////IMAGE/////////////////////

## 3. Debug, gestion d'erreur et statistiques

### 3.1 Gestion du tas

1. HEAP
2. FreeRTOS
