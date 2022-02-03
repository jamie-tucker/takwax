#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COUNT(x) (sizeof(x) / sizeof((x)[0]))

#pragma region Category

typedef struct Category {
  char *name;
  struct Category *parent;
} Category;

typedef struct Collection {
  int count;
  Category items[1024];
} Collection;

Category *
createCategory(Category *cat, char *name) {
  cat->name = name;
  cat->parent = NULL;
  return cat;
}

Category *
findCategory(Collection *collection, char *name) {
  int i;

  for (i = 0; i < collection->count; ++i) {
    if (strcmp(name, collection->items[i].name) == 0) {
      return &collection->items[i];
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

int parse(Collection *collection) {
  FILE *category;

  category = getfile("src/model/", "Categories", ".tsv", "r");

  if (category == NULL) {
    printf("Categories.tsv File Not Found: \n");
    return 0;
  }

  char line[512];
  Category *cat;  // = ((Category *)collection->items[collection->count]);

  while (fgets(line, sizeof line, category)) {
    // printf("%s\n", line);
    // char *lp = strchr(line, '\t');

    // while (lp != NULL) {
    // int len = lp - line;
    // char word[512];

    char *word = (char *)malloc(sizeof(line));
    memset(word, '\0', 512);
    strncpy(word, line, 15);
    // printf("%s\n", word);
    // lp = strchr(lp + 1, '\t');
    // }
    // strcpy(word, line);
    // char *word;
    // memcpy(word, line, 512);

    cat = createCategory(&collection->items[collection->count++], word);
    // free(word);
  }
  fclose(category);
  return 1;
}

// static Category cats[10];
static Collection collection;
// static Collection collection;
int main(void) {
  if (!parse(&collection)) {
    printf("Parsing Error\n");
    exit(1);
  }

  int i;
  for (i = 0; i < collection.count; ++i) {
    printf("Name: %s\n", collection.items[i].name);
    collection.items[i].parent = findCategory(&collection, collection.items[0].name);
    printf("Parent: %s\n", collection.items[i].parent->name);
    // ((Category *)&collection.items[i])->parent = findCategory(&collection, ((Category *)&collection.items[0])->name);
    // printf("Parent: %s\n", ((Category *)&collection.items[i])->parent->name);
  }

  exit(0);
}
