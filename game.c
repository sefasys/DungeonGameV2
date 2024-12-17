#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include "game.h"
#include "room.h"
#define MAX_NAME_LENGTH 256

void clear_input_buffer() {
    while (getchar() != '\n');
}

void game_loop(Player *player, Room *allRooms[], int room_count) {
    char command[MAX_DESC_LENGTH];
    while (1) {
        printf("\n> ");
        if (!fgets(command, sizeof(command), stdin)) {
            fprintf(stderr, "Error reading command\n");
            break;
        }
        command[strcspn(command, "\n")] = 0;  // Remove newline
        if (strcmp(command, "exit") == 0) {
            printf("Exiting the game...\n");
            break;
        }
         execute_command(player, command, allRooms, room_count);
    }
}

void execute_command(Player *player, char *command, Room *allRooms[], int room_count){
    char *token = strtok(command, " ");
    if (!token) return;

    if (strcmp(token, "move") == 0) {
        char *direction = strtok(NULL, " ");
        if (direction) move_player(player, direction);
        else printf("Move where? Specify a direction (up, down, left, right).\n");
    } else if (strcmp(token, "look") == 0) {
        look_around(player);
    } else if (strcmp(token, "inventory") == 0) {
        show_inventory(player);
    } else if(strcmp(token, "commands") == 0){
        show_commands();
    } else if (strcmp(token, "pickup") == 0) {
        pickup_item(player);
    } else if(strcmp(token, "delete") == 0){
        deleteItemFromInventory(player);
    } else if (strcmp(token, "attack") == 0) {
        attack_creature(player);
    } else if (strcmp(token, "interact") == 0) { // New interaction command
        interact_creature(player);
    } else if(strcmp(token, "equip") == 0){
        equip_sword(player);
    } else if(strcmp(token, "read") == 0){
         read_book(player);
     }else if (strcmp(token, "save") == 0) {
            char *filename = strtok(NULL, " ");
            if (filename) {
                save_game(player, allRooms, room_count, filename);
            } else {
                printf("Please specify a filename to save.\n");
            }
    } else if (strcmp(token, "load") == 0) {
            char *filename = strtok(NULL, " ");
            if (filename) {
            load_game(player, allRooms, room_count, filename);
            } else {
                printf("Please specify a filename to load.\n");
            }
    }else {
        printf("Unknown command: %s\n", token);
    }
}

void show_commands(){
    printf("move <direction: up, down, left, right>, look, inventory, commands, pickup <itemname>, read, delete, equip, interact, list, save <file>, load <file>, exit \n");
}

void move_player(Player *player, const char *direction) {
    Room *next_room = NULL;
    if (strcmp(direction, "up") == 0) next_room = player->current_room->up;
    else if (strcmp(direction, "down") == 0) next_room = player->current_room->down;
    else if (strcmp(direction, "left") == 0) next_room = player->current_room->left;
    else if (strcmp(direction, "right") == 0) next_room = player->current_room->right;

    if (next_room) {
        player->current_room = next_room;
        printf("You move %s.\n", direction);
        look_around(player);
    } else {
        printf("You can't go %s from here.\n", direction);
    }
}

void look_around(Player *player) {
    printf("\n%s\n", player->current_room->description);
    if (player->current_room->item) printf("You see a %s here.\n", player->current_room->item->name);
    if (player->current_room->creature) printf("A %s is here!\n", player->current_room->creature->name);
}

void show_inventory(Player *player) {
    if (player->inventory_count == 0) {
        printf("Your inventory is empty.\n");
    } else {
        printf("You have: \n");
        for (int i = 0; i < player->inventory_count; i++) {
            printf("- %s\n", player->inventory[i]->name);
        }
    }
}

void pickup_item(Player *player) {
    if (!player->current_room->item) {
        printf("There's nothing to pick up here.\n");
        return;
    }
    if (player->inventory_count >= MAX_INVENTORY) {
        printf("Your inventory is full!\n");
        return;
    }
    Item *item = player->current_room->item;
    player->inventory[player->inventory_count++] = item;
    player->current_room->item = NULL;
    printf("You picked up a %s.\n", item->name);
}

void attack_creature(Player *player) {
    if (!player->current_room->creature) {
        printf("There's nothing to attack here.\n");
        return;
    }

    Creature *creature = player->current_room->creature;
    int die_roll = (rand() % 6) + 1;  // Simulate a die roll (1 to 6)
    int predicted_roll;
    
    printf("Predict the die roll (1-6): ");
    scanf("%d", &predicted_roll);
    clear_input_buffer();

    printf("The die rolled a %d!\n", die_roll);

    if (predicted_roll == die_roll) {
        printf("Critical hit! You deal double damage!\n");
        creature->health -= player->strength * 2;
    } else if (predicted_roll == die_roll - 1 || predicted_roll == die_roll + 1) {
        printf("Normal hit! You deal normal damage.\n");
        creature->health -= player->strength;
    } else {
        printf("You missed! No damage dealt.\n");
    }
}



void interact_creature(Player *player) {
    if (!player->current_room->creature) {
        printf("There's no creature to interact with.\n");
        return;
    }

    Creature *creature = player->current_room->creature;

    while (player->health > 0 && creature->health > 0) {
        printf("\n%s's health: %d\nYour health: %d\n", creature->name, creature->health, player->health);
        printf("Choose an action:\n");
        printf("1. Attack\n");
        printf("2. Defuse\n");
        printf("3. Use Item\n");

        int choice;
        scanf("%d", &choice);

        if (choice == 1) {  // Attack
            attack_creature(player);
        } else if (choice == 2) {  // Defuse
            if (creature->strength == 0) {
                printf("%s is defused!\n", creature->name);
                printf("%s\n", creature->story);
                free(creature->name);
                free(creature);
                player->current_room->creature = NULL;
                return;
            } else {
                printf("Defuse failed. The creature wants to fight.\n");
            }
        } else if (choice == 3) {  // Use Item
            use_item(player);
        } else {
            printf("Invalid choice. Try again.\n");
        }

        // Creature attacks back if still alive
        if (creature->health > 0) {
    // %50 ihtimalle yaratık saldırısı başarısız olabilir
    if (rand() % 2 == 0) {  // 0 veya 1 üretir; 0 başarılı saldırı anlamına gelir
        printf("The %s attacked you %d damage!\n", creature->name, creature->strength);
        player->health -= creature->strength;
    } else {
        printf("%s missed its chance!\n", creature->name);
    }
}


        if (player->health <= 0) {
            printf("You have been defeated by the %s!\n\n", creature->name);
            printf("You have died. Game over.\n");
            exit(0);
        }
    }

    printf("You defeated the %s!\n", creature->name);
    if (creature->story) {
        printf("%s\n", creature->story);
    }

    free(creature->name);
    free(creature);
    player->current_room->creature = NULL;
}

void save_game(Player *player, Room *allRooms[], int room_count, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not save the game to %s.\n", filename);
        return;
    }

    // Oyuncu bilgilerini kaydet
    fprintf(file, "PlayerHealth: %d\n", player->health);
    fprintf(file, "PlayerStrength: %d\n", player->strength);
    fprintf(file, "InventoryCount: %d\n", player->inventory_count);

    // Oyuncunun envanterindeki eşyaları kaydet
    for (int i = 0; i < player->inventory_count; i++) {
        fprintf(file, "Item: %s %d %d %d %d\n",
                player->inventory[i]->name,
                player->inventory[i]->healing_value,
                player->inventory[i]->damage_boost,
                player->inventory[i]->is_usable,
                player->inventory[i]->is_weapon);
    }

    // Oyuncunun bulunduğu odanın ID'sini kaydet
    fprintf(file, "CurrentRoomID: %d\n", player->current_room->id);

    // Odaları kaydet
    fprintf(file, "RoomCount: %d\n", room_count);
    for (int i = 0; i < room_count; i++) {
        Room *room = allRooms[i];
        fprintf(file, "RoomID: %d\n", room->id);
        fprintf(file, "Description: %s\n", room->description);

        // Oda itemini kaydet
        if (room->item) {
            fprintf(file, "Item: %s %d %d %d %d\n",
                    room->item->name,
                    room->item->healing_value,
                    room->item->damage_boost,
                    room->item->is_usable,
                    room->item->is_weapon);
        } else {
            fprintf(file, "Item: NULL\n");
        }

        // Oda yaratığını kaydet
        if (room->creature) {
            fprintf(file, "Creature: %s %d %d\n",
                    room->creature->name,
                    room->creature->health,
                    room->creature->strength);
        } else {
            fprintf(file, "Creature: NULL\n");
        }
    }

    fclose(file);
    printf("Game saved to %s.\n", filename);
}

void load_game(Player *player, Room *allRooms[], int room_count, const char *filename) {
    char buffer[MAX_NAME_LENGTH];
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not load the game from %s.\n", filename);
        return;
    }

    printf("Loading game from %s...\n", filename);

    // Oyuncu bilgilerini yükle
    if (fscanf(file, "PlayerHealth: %d\n", &player->health) != 1) {
        printf("Error reading PlayerHealth.\n");
        fclose(file);
        return;
    }
    printf("Loaded Health: %d\n", player->health);

    if (fscanf(file, "PlayerStrength: %d\n", &player->strength) != 1) {
        printf("Error reading PlayerStrength.\n");
        fclose(file);
        return;
    }
    printf("Loaded Strength: %d\n", player->strength);

    if (fscanf(file, "InventoryCount: %d\n", &player->inventory_count) != 1) {
        printf("Error reading InventoryCount.\n");
        fclose(file);
        return;
    }
    printf("Loaded Inventory Count: %d\n", player->inventory_count);

    // Oyuncunun envanterini yükle
    for (int i = 0; i < player->inventory_count; i++) {
        player->inventory[i] = malloc(sizeof(Item));
        if (fscanf(file, "Item: %s %d %d %d %d\n",
                   buffer,
                   &player->inventory[i]->healing_value,
                   &player->inventory[i]->damage_boost,
                   &player->inventory[i]->is_usable,
                   &player->inventory[i]->is_weapon) == 5) {
            player->inventory[i]->name = strdup(buffer);
            printf("Loaded item: %s\n", player->inventory[i]->name);
        } else {
            printf("Error reading item %d.\n", i);
            fclose(file);
            return;
        }
    }

    // Oyuncunun bulunduğu odanın ID'sini yükle
    int room_id;
    if (fscanf(file, "CurrentRoomID: %d\n", &room_id) != 1) {
        printf("Error reading CurrentRoomID.\n");
        fclose(file);
        return;
    }
    if (room_id >= 0 && room_id < room_count) {
        player->current_room = allRooms[room_id];
        printf("Loaded Current Room ID: %d\n", room_id);
    } else {
        printf("Invalid room ID in save file: %d\n", room_id);
        fclose(file);
        return;
    }

    fclose(file);
    printf("Game loaded successfully from %s.\n", filename);
}










