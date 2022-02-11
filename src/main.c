#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COUNT(x) (sizeof(x) / sizeof((x)[0]))

#pragma region Entry

typedef struct Entry {
  int children_len;
  char name[512];  // Maybe allocate here?
  struct Entry *parent, *children[100];
} Entry;

Entry *
createEntry(Entry *entry, char *name) {
  strcpy(entry->name, name);
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

int parse(Entry *entry) {
  FILE *catFile;

  catFile = getfile("src/model/", "Categories", ".tsv", "r");

  if (catFile == NULL) {
    printf("Categories.tsv File Not Found: \n");
    return 0;
  }

  char line[512];

  while (fgets(line, sizeof line, catFile)) {
    char *token = strtok(line, "\t\n");
    // char *name = (char *)malloc(sizeof(token));
    // strcpy(name, token);

    createEntry(entry++, token);
  }

  fclose(catFile);
  return 1;
}

int main(void) {
  Entry entries[10] = {};

  if (!parse(entries)) {
    printf("Parsing Error\n");
    exit(1);
  }

  Entry *ep = entries;

  int i;

  for (i = 0; i < COUNT(entries); i++) {
    printf("Name: %s\n", ep->name);
    ep->parent = findEntry(entries, entries[0].name, COUNT(entries));
    printf("Parent: %s\n", ep->parent->name);
    ep++;
  }

  exit(0);
}
