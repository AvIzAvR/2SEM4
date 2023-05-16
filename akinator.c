#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "akinator.h"

Node* createNode(const char* question) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->question = strdup(question);
    node->yes = NULL;
    node->no = NULL;
    return node;
}

void insert(Node* node, const char* question, const char* answer) {
    if (strcmp(answer, "yes") == 0) {
        if (node->yes == NULL) {
            node->yes = createNode(question);
        } else {
            insert(node->yes, question, answer);
        }
    } else if (strcmp(answer, "no") == 0) {
        if (node->no == NULL) {
            node->no = createNode(question);
        } else {
            insert(node->no, question, answer);
        }
    }
}

void play(Node* node, Node*** answers, int* answersSize) {
    setlocale(LC_ALL, "Rus");
    if(node->yes == NULL) {
        printf("Это %s?\n", node->question);
    }
    else {
        printf("%s\n", node->question);
    }
    char response[10];
    while(strcmp(response,"+") != 0 && strcmp(response,"-") != 0) {
        scanf("%9s", response);
    }
    if (strcmp(response, "+") == 0) {
        *answers = realloc(*answers, (*answersSize + 1)*sizeof(Node*));
        (*answers)[*answersSize] = node;
        (*answersSize)++;
        if(node->yes != NULL) {
            play(node->yes, answers, answersSize);
        }
        else {
            printf("Я угадал!\n");
        }
    } else if (strcmp(response, "-") == 0) {
        if(node->no != NULL) {
            play(node->no, answers, answersSize);
        }
        else {
            printf("Я проиграл. Что это было?\n");
            char newObject[100];
            char newQuestion[100];
            scanf("%s", newObject);
            printf("Напишите вопрос, который можно задать, чтобы отличить %s от %s:\n", newObject, node->question);
            scanf(" %[^\n]s", newQuestion);
            Node* newQuestionNode = createNode(newQuestion);
            Node* newObjectNode = createNode(newObject);
            newQuestionNode->yes = newObjectNode;
            node->no = newQuestionNode;
        }
    }
}

void saveToFile(FILE* fp, Node* node) {
    if (node == NULL) {
        return;
    }
    fprintf(fp, "{");
    fprintf(fp, "\"question\":\"%s\"", node->question);
    if (node->yes != NULL) {
        fprintf(fp, ",\"yes\":");
        saveToFile(fp, node->yes);
    }
    if (node->no != NULL) {
        fprintf(fp, ",\"no\":");
        saveToFile(fp, node->no);
    }
    fprintf(fp, "}");
}

void save(Node* root, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if(fp != NULL) {
        saveToFile(fp, root);
        fclose(fp);
    }
}

int getEnd(const char* line) {
    int c = 0;
    int end = 0;
    for(int i = 0; i < (int)strlen(line); i++) {
        if(line[i] == '{')
            c++;
        if(line[i] == '}') {
            c--;
            if(c == 0) {
                end = i;
                break;
            }
        }
    }
    return end;
}

Node* parseNode(const char* line);

void parse(const char* yesStart, const char* toCheck, const char* line, Node** node) {
    if (yesStart != NULL && strcmp(toCheck,"yes") == 0) {
        int end = getEnd(yesStart);

        char* yesEnd = calloc(end-5, sizeof(char));
        strncpy(yesEnd, yesStart+6, end - 5);
        if (yesEnd != NULL) {
            (*node)->yes = parseNode(yesEnd);
            free(yesEnd);
        }
        char* afterYes = calloc(strlen(line)-end-2, sizeof(char));
        strncpy(afterYes, yesStart+end+2, strlen(line)-end-2);
        const char* noStart = strstr(afterYes, "\"no\":{");
        if (noStart != NULL) {
            end = getEnd(noStart);
            char* noEnd = calloc(end-5, sizeof(char));
            strncpy(noEnd, noStart+5, end - 4);
            if (noEnd != NULL) {
                (*node)->no = parseNode(noEnd);
                free(noEnd);
            }
        }
    }
    else {
        const char* noStart = strstr(line, "\"no\":{");
        if (noStart != NULL) {
            int end = getEnd(noStart);
            char* noEnd = calloc(end-5, sizeof(char));
            strncpy(noEnd, noStart+5, end - 5);
            if (noEnd != NULL) {
                (*node)->no = parseNode(noEnd);
                free(noEnd);
            }
        }
    }
}

Node* parseNode(const char* line) {
    const char* questionStart = strstr(line, "\":\"");
    if (questionStart == NULL) {
        return NULL;
    }
    questionStart += 3;
    const char* questionEnd = strchr(questionStart, '\"');
    if (questionEnd == NULL) {
        return NULL;
    }
    size_t questionLen = questionEnd - questionStart;
    char* question = malloc(questionLen + 1);
    if (question == NULL) {
        return NULL;
    }
    memcpy(question, questionStart, questionLen);
    question[questionLen] = '\0';
    Node* node = createNode(question);
    const char* yesStart = strstr(line, "\"yes\":{");
    char* tocheck = malloc(4);
    if (tocheck == NULL) {
        free(question);
        free(node);
        return NULL;
    }
    memcpy(tocheck, questionEnd + 3, 3);
    tocheck[3] = '\0';
    parse(yesStart, tocheck, line, &node);
    free(tocheck);
    free(question);
    return node;
}

Node* loadFromFile(FILE* fp) {
    char* line = NULL;
    size_t len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        char* new_line = realloc(line, len + 2);
        if (new_line == NULL) {
            free(line);
            return NULL;
        }
        line = new_line;
        line[len++] = c;
    }
    line[len] = '\0';

    return parseNode(line);
}


Node* load(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("\033[31mError! File not found\033[0m\n");
        return NULL;
    }
    Node* root = loadFromFile(fp);
    fclose(fp);
    return root;
}

void printTree(struct Node* root, int level, Node** answers, int answersLen) {
    setlocale(LC_ALL, "Rus");
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    int founded = 0;
    for(int i = 0; i < answersLen; i++) {
        if(answers[i] == root) {
            founded = 1;
            break;
        }
    }
    if(founded)
        printf("\033[32m= %s\033[0m\n", root->question);
    else
        printf("\033[31m- %s\033[0m\n", root->question);

    printTree(root->yes, level+1, answers, answersLen);
    printTree(root->no, level, answers, answersLen);
}

void clearMemory(Node* node) {
    if(node->question != NULL)
        free(node->question);
    if(node->yes != NULL) {
        clearMemory(node->yes);
    }
    if(node->no != NULL) {
        clearMemory(node->no);
    }
    free(node);
}