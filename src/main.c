#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODEL_DIR "src/model/"
#define CONTENT_DIR "src/content/"
#define TEMPLATE_DIR "src/template/"

#define ENTRY_SIZE 32

#define COUNT(x) (sizeof(x) / sizeof((x)[0]))

#pragma region Model

typedef enum {
  header = 1,
  footer = 2,
  content = 3
} TemplateType;

typedef struct Template {
  long body_len;
  char name[ENTRY_SIZE];
  char *body;
  TemplateType type;
} Template;

typedef struct Templates {
  int length;
  struct Template values[100];
} Templates;

typedef struct Entry {
  int children_len, incoming_len;
  long content_len;
  char name[ENTRY_SIZE];
  char *content;
  struct Template *template;
  struct Entry *parent, *children[64], *incoming[64];
} Entry;

typedef struct Entries {
  int length;
  struct Entry values[100];
} Entries;

Entry *
create_entry(Entries *entries, char *name) {
  Entry *entry = &entries->values[entries->length++];
  strcpy(entry->name, name);
  entry->children_len = 0;
  return entry;
}

Entry *
find_entry(Entries *entries, char *name) {
  int i;

  for (i = 0; i < entries->length; ++i) {
    if (strcmp(name, entries->values[i].name) == 0) {
      return &entries->values[i];
    }
  }

  printf("Cannot find entry %s\n", name);

  return NULL;
}

int get_type(char *type) {
  if (strcmp(type, "header") == 0)
    return 1;
  else if (strcmp(type, "footer") == 0)
    return 2;
  else if (strcmp(type, "content") == 0)
    return 3;
  else
    return 0;
}

Template *
create_template(Templates *templates, char *name, char *type) {
  Template *template = &templates->values[templates->length++];
  strcpy(template->name, name);
  if ((template->type = get_type(type)) == 0) {
    printf("Incorrect Template Type %s", type);
    return NULL;
  }

  return template;
}

Template *
find_template(Templates *templates, char *name) {
  int i;

  for (i = 0; i < templates->length; ++i) {
    if (strcmp(name, templates->values[i].name) == 0) {
      return &templates->values[i];
    }
  }

  printf("Cannot find Template %s\n", name);

  return NULL;
}

#pragma endregion

FILE *
get_file(char *dir, char *filename, char *ext, char *op) {
  char fullpath[1024] = {"\0"};
  strcat(fullpath, dir);
  strcat(fullpath, filename);
  strcat(fullpath, ext);
  printf("Opening File: \"%s\"\n", fullpath);
  return fopen(fullpath, op);
}

int parse_entries(Entries *entries, Templates *templates) {
  FILE *entryFile = get_file(MODEL_DIR, "entries", ".tsv", "r");

  if (entryFile == NULL) {
    printf("entries.tsv File Not Found: \n");
    return 0;
  }

  char line[3 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, entryFile));  // skip header

  // Entry *entryPtr = entries->values;
  while (fgets(line, sizeof line, entryFile)) {
    char *name = strtok(line, "\t\n");
    char *parent = strtok(NULL, "\t\n");
    char *template = strtok(NULL, "\t\n");

    Entry *entryPtr = create_entry(entries, name);

    FILE *contentFile = get_file(CONTENT_DIR, name, ".md", "r");

    if (contentFile == NULL) {
      printf("Content File Not Found %s.md\n", name);
      return 0;
    }

    fseek(contentFile, 0, SEEK_END);
    entryPtr->content_len = ftell(contentFile);
    fseek(contentFile, 0, SEEK_SET);
    entryPtr->content = (char *)malloc(entryPtr->content_len);
    if (entryPtr->content) {
      fread(entryPtr->content, 1, entryPtr->content_len, contentFile);
    } else {
      printf("Not enough memory");
      return 0;
    }

    fclose(contentFile);

    entryPtr->parent = find_entry(entries, parent);
    entryPtr->parent->children[entryPtr->parent->children_len++] = entryPtr;

    entryPtr->template = find_template(templates, template);
  }

  fclose(entryFile);
  return 1;
}

int parse_templates(Templates *templates) {
  FILE *templateModel = get_file(MODEL_DIR, "templates", ".tsv", "r");

  if (templateModel == NULL) {
    printf("File Not Found: templates.tsv\n");
    return 0;
  }

  char line[2 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, templateModel));  // skip header

  while (fgets(line, sizeof line, templateModel)) {
    char *name = strtok(line, "\t\n");
    char *type = strtok(NULL, "\t\n");

    Template *templatePtr = create_template(templates, name, type);

    if (templatePtr == NULL)
      return 0;

    FILE *templateFile = get_file(TEMPLATE_DIR, name, ".html", "r");

    if (templateFile == NULL) {
      printf("File Not Found: %s.html\n", name);
      return 0;
    }

    fseek(templateFile, 0, SEEK_END);
    templatePtr->body_len = ftell(templateFile);
    fseek(templateFile, 0, SEEK_SET);
    templatePtr->body = (char *)malloc(templatePtr->body_len);
    if (templatePtr->body) {
      fread(templatePtr->body, 1, templatePtr->body_len, templateFile);
    } else {
      printf("Not enough memory");
      return 0;
    }

    fclose(templateFile);
  }

  fclose(templateModel);
  return 1;
}

int parse(Entries *entries, Templates *templates) {
  if (!parse_templates(templates)) {
    printf("Error Parsing Templates\n");
    return 0;
  }

  if (!parse_entries(entries, templates)) {
    printf("Error Parsing Entries\n");
    return 0;
  }

  return 1;
}

static Entries entries;
static Templates templates;

int main(void) {
  if (!parse(&entries, &templates)) {
    printf("Parsing Error\n");
    exit(1);
  }

  // Print Debugs
  int i;
  Entry *head = NULL;

  for (i = 0; i < entries.length; i++) {
    if (head == NULL && (&entries.values[i] == entries.values[i].parent)) {
      head = &entries.values[i];
      printf("Head is: %s\n", head->name);
    }
    printf("Name: %s\n", entries.values[i].name);
    printf("Size: %lu\n", sizeof(entries.values[i]));
    printf("Parent: %s\n", entries.values[i].parent->name);
    printf("Children: ");
    int c;
    for (c = 0; c < entries.values[i].children_len; c++) {
      printf("%s ", entries.values[i].children[c]->name);
    }
    printf("\n");
    printf("Template: %s\n", entries.values[i].template->name);
  }

  exit(0);
}
