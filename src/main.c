#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODEL_DIR "src/model/"
#define CONTENT_DIR "src/content/"
#define TEMPLATE_DIR "src/template/"

#define ENTRY_SIZE 32

#define COUNT(x) (sizeof(x) / sizeof((x)[0]))

#pragma region Entry

typedef struct Entry {
  int children_len;
  char name[ENTRY_SIZE], template[ENTRY_SIZE];
  struct Entry *parent, *children[64];
} Entry;

Entry *
createEntry(Entry *entry, char *name) {
  strcpy(entry->name, name);
  entry->children_len = 0;
  return entry;
}

Entry *
findEntry(Entry *entry, char *name, int size) {
  int i;

  for (i = 0; i < size; ++i) {
    if (strcmp(name, entry[i].name) == 0) {
      return &entry[i];
    }
  }

  printf("Cannot find entry %s\n", name);

  return NULL;
}

#pragma endregion

FILE *
getfile(char *dir, char *filename, char *ext, char *op) {
  char fullpath[1024] = {"\0"};
  strcat(fullpath, dir);
  strcat(fullpath, filename);
  strcat(fullpath, ext);
  printf("Full Path = \"%s\"\n", fullpath);
  return fopen(fullpath, op);
}

int parse_entries(Entry *entry, int *entry_len) {
  FILE *entryFile = getfile(MODEL_DIR, "Entries", ".tsv", "r");

  if (entryFile == NULL) {
    printf("Entries.tsv File Not Found: \n");
    return 0;
  }

  char line[3 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, entryFile));  // skip header

  Entry *entryPtr = entry;
  while (fgets(line, sizeof line, entryFile)) {
    char *token = strtok(line, "\t\n");
    createEntry(entryPtr, token);
    (*entry_len)++;

    token = strtok(NULL, "\t\n");
    entryPtr->parent = findEntry(entry, token, *entry_len);
    entryPtr->parent->children[entryPtr->parent->children_len++] = entryPtr;

    token = strtok(NULL, "\t\n");
    strcpy(entryPtr->template, token);

    entryPtr++;
  }

  fclose(entryFile);
  return 1;
}

int parse(Entry *entry, int *entry_len) {
  if (!parse_entries(entry, entry_len)) {
    printf("Error Parsing Entries\n");
  }

  return 1;
}

int main(void) {
  Entry entries[10];
  int entry_len = 0;

  if (!parse(entries, &entry_len)) {
    printf("Parsing Error\n");
    exit(1);
  }

  // Print Debugs
  int i;

  for (i = 0; i < entry_len; i++) {
    printf("Name: %s\n", entries[i].name);
    printf("Parent: %s\n", entries[i].parent->name);
    printf("Children: ");
    int c;
    for (c = 0; c < entries[i].children_len; c++) {
      printf("%s ", entries[i].children[c]->name);
    }
    printf("\n");
    printf("Template: %s\n", entries[i].template);
  }

  exit(0);
}
