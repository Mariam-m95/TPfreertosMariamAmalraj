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
