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
  int children_len, incoming_len;
  long content_len;
  char name[ENTRY_SIZE], template[ENTRY_SIZE];
  char *content;
  struct Entry *parent, *children[64], *incoming[64];
} Entry;

typedef struct Entries {
  int entries_len;
  struct Entry entries[100];
} Entries;

Entry *
createEntry(Entry *entry, char *name) {
  strcpy(entry->name, name);
  entry->children_len = 0;
  return entry;
}

Entry *
findEntry(Entries *entries, char *name) {
  int i;

  for (i = 0; i < entries->entries_len; ++i) {
    if (strcmp(name, entries->entries[i].name) == 0) {
      return &entries->entries[i];
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
  printf("Opening File: \"%s\"\n", fullpath);
  return fopen(fullpath, op);
}

int parse_entries(Entries *entries) {
  FILE *entryFile = getfile(MODEL_DIR, "entries", ".tsv", "r");

  if (entryFile == NULL) {
    printf("Entries.tsv File Not Found: \n");
    return 0;
  }

  char line[3 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, entryFile));  // skip header

  Entry *entryPtr = entries->entries;
  while (fgets(line, sizeof line, entryFile)) {
    char *token = strtok(line, "\t\n");
    createEntry(entryPtr, token);

    FILE *contentFile = getfile(CONTENT_DIR, token, ".md", "r");

    if (contentFile) {
      fseek(contentFile, 0, SEEK_END);
      entryPtr->content_len = ftell(contentFile);
      fseek(contentFile, 0, SEEK_SET);
      entryPtr->content = malloc(entryPtr->content_len);
      if (entryPtr->content) {
        fread(entryPtr->content, 1, entryPtr->content_len, contentFile);
      } else {
        printf("Not enough memory");
        return 0;
      }

      fclose(contentFile);
    } else {
      printf("Error Parsing Content: %s\n", token);
      return 0;
    }

    entries->entries_len++;

    token = strtok(NULL, "\t\n");
    entryPtr->parent = findEntry(entries, token);
    entryPtr->parent->children[entryPtr->parent->children_len++] = entryPtr;

    token = strtok(NULL, "\t\n");
    strcpy(entryPtr->template, token);

    entryPtr++;
  }

  fclose(entryFile);
  return 1;
}

int parse(Entries *entries) {
  if (!parse_entries(entries)) {
    printf("Error Parsing Entries\n");
  }

  return 1;
}

static Entries entries;

int main(void) {
  if (!parse(&entries)) {
    printf("Parsing Error\n");
    exit(1);
  }

  // Print Debugs
  int i;
  Entry *head = NULL;

  for (i = 0; i < entries.entries_len; i++) {
    if (head == NULL && (&entries.entries[i] == entries.entries[i].parent)) {
      head = &entries.entries[i];
      printf("Head is: %s\n", head->name);
    }
    printf("Name: %s\n", entries.entries[i].name);
    printf("Size: %lu\n", sizeof(entries.entries[i]));
    printf("Parent: %s\n", entries.entries[i].parent->name);
    printf("Children: ");
    int c;
    for (c = 0; c < entries.entries[i].children_len; c++) {
      printf("%s ", entries.entries[i].children[c]->name);
    }
    printf("\n");
    printf("Template: %s\n", entries.entries[i].template);
  }

  exit(0);
}
