#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODEL_DIR "src/model/"
#define CONTENT_DIR "src/content/"
#define TEMPLATE_DIR "src/template/"
#define OUTPUT_DIR "output/site/"
#define TEMP_DIR "output/temp/"

#define ENTRY_SIZE 32
#define ENTRY_COUNT 100

// Markdown Settings
#define TAB_SIZE 2

#define COUNT(x) (sizeof(x) / sizeof((x)[0]))

#pragma region Model

typedef struct Media {
  char filename[ENTRY_SIZE];
  char *alt_text, caption;
  int show_caption;
} Media;

typedef struct Template {
  long body_len;
  char id[ENTRY_SIZE];
  char *body;
} Template;

typedef struct Templates {
  int length;
  struct Template values[ENTRY_COUNT];
} Templates;

typedef struct Entry {
  int children_len, incoming_len;
  long content_len;
  char id[ENTRY_SIZE];
  char name[ENTRY_SIZE];
  char content[1024 * 1024];
  struct Template *header, *footer, *template;
  struct Entry *parent, *children[ENTRY_COUNT], *incoming[ENTRY_COUNT];
} Entry;

typedef struct Entries {
  int length;
  struct Entry values[ENTRY_COUNT];
} Entries;

Entry *
create_entry(Entries *entries, char *id, char *name) {
  Entry *entry = &entries->values[entries->length++];
  strcpy(entry->id, id);
  strcpy(entry->name, name);
  entry->children_len = 0;
  entry->content_len = 0;
  entry->incoming_len = 0;
  return entry;
}

Entry *
find_entry(Entry *entry_start, int length, char *id) {
  int i;

  for (i = 0; i < length; ++i) {
    Entry *entry_ptr = entry_start + i;
    if (strcmp(id, entry_ptr->id) == 0) {
      return entry_ptr;
    }
  }

  return NULL;
}

Entry *
find_entry_in_entries(Entries *entries, char *id) {
  return find_entry(&entries->values[0], entries->length, id);
}

Template *
create_template(Templates *templates, char *id) {
  Template *template = &templates->values[templates->length++];
  strcpy(template->id, id);

  return template;
}

Template *
find_template(Templates *templates, char *id) {
  int i;

  for (i = 0; i < templates->length; ++i) {
    if (strcmp(id, templates->values[i].id) == 0) {
      return &templates->values[i];
    }
  }

  printf("Template Not Found: %s\n", id);

  return NULL;
}

#pragma endregion

static FILE *
get_file(char *dir, char *filename, char *ext, char *op) {
  char fullpath[1024] = {"\0"};
  strcat(fullpath, dir);
  strcat(fullpath, filename);
  strcat(fullpath, ext);
  printf("Opening File: \"%s\"\n", fullpath);
  return fopen(fullpath, op);
}

static int
delete_file(char *dir, char *filename, char *ext) {
  char fullpath[1024] = {"\0"};
  strcat(fullpath, dir);
  strcat(fullpath, filename);
  strcat(fullpath, ext);
  printf("Removing File: \"%s\"\n", fullpath);
  return remove(fullpath);
}

#pragma region Markdown

static void md_line(FILE *output, char *curLine, Entry *entry, Entries *entries);

static int
md_img(FILE *file, char *curLine) {
  char *start_ptr = strchr(curLine, '!');
  if (strncmp(start_ptr, "![", 2) == 0) {
    char *end_ptr = strchr(start_ptr + 1, ']');
  } else {
    return 0;
  }

  // fprintf(file, "<a href=\"%s\">%s</a>", linkURL, lineName);

  return 1;
}

static int
md_link(FILE *file, char *curLine, Entry *entry, Entries *entries) {
  char linkName[1024] = {'\0'};
  char linkURL[1024] = {'\0'};
  int externalLink = 0;

  char *start_ptr = strchr(curLine, '['), *link_end_ptr;
  if (start_ptr) {
    char *end_ptr = strchr(start_ptr + 1, ']');
    char *link_ptr;

    if (strncmp(end_ptr, "]({", 3) == 0) {
      link_ptr = end_ptr + 3;
      link_end_ptr = strchr(link_ptr, '}');
      *link_end_ptr = '\0';

      Entry *linkedEntry = find_entry_in_entries(entries, link_ptr);
      if (linkedEntry) {
        Entry *incEntry = find_entry(linkedEntry->incoming[0], linkedEntry->incoming_len, entry->id);
        if (incEntry == NULL && linkedEntry != entry) {
          linkedEntry->incoming[linkedEntry->incoming_len++] = entry;
        }

        sprintf(linkURL, "./%s.html", linkedEntry->id);
      }

      *link_end_ptr = '}';
      link_end_ptr++;
    } else if (strncmp(end_ptr, "](#", 3) == 0) {
      link_ptr = end_ptr + 2;  // include the #
      link_end_ptr = strchr(link_ptr, ')');
      *link_end_ptr = '\0';
      sprintf(linkURL, "%s", link_ptr);
      *link_end_ptr = ')';
    } else if (strncmp(end_ptr, "](", 2) == 0) {
      link_ptr = end_ptr + 2;
      link_end_ptr = strchr(link_ptr, ')');

      *link_end_ptr = '\0';
      sprintf(linkURL, "%s", link_ptr);
      *link_end_ptr = ')';
      externalLink = 1;
    } else {
      return 0;
    }

    *start_ptr = '\0';
    fprintf(file, "%s", curLine);
    size_t nameSize = end_ptr - start_ptr - 1;

    if (nameSize > 0) {
      strncpy(linkName, start_ptr + 1, nameSize);
    } else {
      strcpy(linkName, linkURL);
    }

    *start_ptr = '[';
  } else {
    return 0;
  }

  if (externalLink)
    fprintf(file, "<a href=\"%s\" target=\"_blank\">%s</a>", linkURL, linkName);
  else
    fprintf(file, "<a href=\"%s\">%s</a>", linkURL, linkName);

  if (link_end_ptr)
    md_line(file, link_end_ptr + 1, entry, entries);

  return 1;
}

static void
md_line(FILE *output, char *curLine, Entry *entry, Entries *entries) {
  if (md_link(output, curLine, entry, entries)) {
    // char *start_ptr = strchr(curLine, '{');
    // if (start_ptr) {
    //   char *end_ptr = strchr(start_ptr + 1, '}');

    //   *start_ptr = '\0';
    //   fprintf(output, "%s", curLine);
    //   size_t size = end_ptr - start_ptr - 1;

    //   if (strncmp("title", start_ptr + 1, size) == 0) {
    //     ftitle(output, entry);
    //   } else if (strncmp("nav", start_ptr + 1, size) == 0) {
    //     fnav(output, &entries->values[0]);
    //   } else if (strncmp("...", start_ptr + 1, size) == 0) {
    //     fbreadcrumbs(output, entry);
    //   } else if (strncmp("content", start_ptr + 1, size) == 0) {
    //     output_md(output, entry, entries);
    //   }

    //   *start_ptr = '{';
    // output_html_line(output, end_ptr + 1, entry, entries);
  } else {
    fprintf(output, "%s", curLine);
  }
}

static int
reset_list(FILE *output, int listLevel) {
  while (listLevel > 0) {
    fputs("\n</ul>", output);
    listLevel--;
  }

  return listLevel;
}

static int
is_html(char *curLine) {
  if (strncmp(curLine, "<", 1) == 0) {
    char *ptr = curLine;
    while (*ptr != '\n') {
      if (strncmp(ptr, ">", 1) == 0) {
        return 1;
      }

      ptr++;
    }
  }

  return 0;
}

static int
is_header(char *curLine) {
  if (strncmp(curLine, "# ", 2) == 0) {
    return 1;
  } else if (strncmp(curLine, "## ", 3) == 0) {
    return 2;
  } else if (strncmp(curLine, "### ", 4) == 0) {
    return 3;
  } else if (strncmp(curLine, "#### ", 5) == 0) {
    return 4;
  } else if (strncmp(curLine, "##### ", 6) == 0) {
    return 5;
  } else if (strncmp(curLine, "###### ", 7) == 0) {
    return 6;
  }

  return 0;
}

static void
output_markdown(FILE *output, char *buffer, Entry *entry, Entries *entries) {
  char *curLine = buffer;
  int prevIndent = 0, listLevel = 0;
  while (curLine) {
    char *nextLine = strchr(curLine, '\n');
    if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

    char openTag[100] = {'\0'};
    char closeTag[100] = {'\0'};
    int lineLength = curLine - nextLine;
    int indent = 0;
    char *startLine = curLine;

    while (isblank(*curLine)) {
      if (*curLine == '\t')
        indent += TAB_SIZE;
      else
        indent++;
      curLine++;
    }

    int header = is_header(curLine);
    if (header > 0 && indent <= TAB_SIZE * 2) {
      listLevel = reset_list(output, listLevel);
      sprintf(openTag, "<h%i>", header);
      sprintf(closeTag, "</h%i>", header);
      curLine += header + 1;
    } else if (is_html(curLine)) {
      // do nothing
    } else if (strncmp(curLine, "- ", 2) == 0) {
      strcpy(openTag, "<li>");
      strcpy(closeTag, "</li>");

      if (listLevel == 0 || indent - prevIndent == TAB_SIZE) {
        strcpy(openTag, "<ul>\n<li>");
        listLevel++;
      } else if (indent < prevIndent) {
        int i, tagIndex;
        for (i = 0, tagIndex = 0; i < (prevIndent - indent) / TAB_SIZE; i++) {
          if (i == 0) {
            strcpy(&openTag[0], "</ul>");
            tagIndex = 5;
          } else {
            strcpy(&openTag[tagIndex], "\n</ul>");
            tagIndex += 6;
          }
          listLevel--;
        }

        if (i == 0)
          strcpy(&openTag[0], "<li>");
        else
          strcpy(&openTag[tagIndex], "\n<li>");
      }

      curLine += 2;
    } else if (*curLine == '\0' && *(curLine + 1) == '\n') {
      listLevel = reset_list(output, listLevel);
      strcpy(openTag, "<br />");
    } else if (*curLine != '\0') {
      listLevel = reset_list(output, listLevel);
      strcpy(openTag, "<p>");
      strcpy(closeTag, "</p>");
    } else {
      listLevel = reset_list(output, listLevel);
    }

    fputc('\n', output);

    int i;
    for (i = 0; i < indent; i++) {
      fputc(' ', output);
    }
    fprintf(output, "%s", openTag);
    md_line(output, curLine, entry, entries);
    fprintf(output, "%s", closeTag);

    if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
    curLine = nextLine ? (nextLine + 1) : NULL;
    prevIndent = indent;
  }
}

int parse_content(Entries *entries) {
  int i;
  char contentBuffer[1024 * 1024];
  long contentBufferLength = 0;

  for (i = 0; i < entries->length; i++) {
    Entry *entryPtr = &entries->values[i];

    FILE *contentFile = get_file(CONTENT_DIR, entryPtr->id, ".md", "r");

    if (contentFile == NULL) {
      printf("File Not Found %s.md\n", entryPtr->id);
      continue;
    }

    fseek(contentFile, 0, SEEK_END);
    contentBufferLength = ftell(contentFile);
    fseek(contentFile, 0, SEEK_SET);
    fread(contentBuffer, contentBufferLength + 1, 1, contentFile);
    fclose(contentFile);

    FILE *output = get_file(TEMP_DIR, entryPtr->id, ".txt", "w+");

    if (output == NULL) {
      printf("File Not Found: %s.txt\n", entryPtr->id);
      return 0;
    }

    output_markdown(output, contentBuffer, entryPtr, entries);
    fseek(output, 0, SEEK_END);
    entryPtr->content_len = ftell(output);
    fseek(output, 0, SEEK_SET);
    fread(entryPtr->content, entryPtr->content_len + 1, 1, output);
    fclose(output);
    delete_file(TEMP_DIR, entryPtr->id, ".txt");
  }

  return 1;
}

#pragma endregion

int parse_entries(Entries *entries, Templates *templates) {
  FILE *entryFile = get_file(MODEL_DIR, "entries", ".tsv", "r");

  if (entryFile == NULL) {
    printf("entries.tsv File Not Found: \n");
    return 0;
  }

  char line[6 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, entryFile));  // skip header

  while (fgets(line, sizeof line, entryFile)) {
    char *id = strtok(line, "\t\n");
    char *name = strtok(NULL, "\t\n");
    char *parent = strtok(NULL, "\t\n");
    char *template = strtok(NULL, "\t\n");
    char *header = strtok(NULL, "\t\n");
    char *footer = strtok(NULL, "\t\n");

    Entry *entryPtr = create_entry(entries, id, name);

    entryPtr->parent = find_entry_in_entries(entries, parent);
    entryPtr->parent->children[entryPtr->parent->children_len++] = entryPtr;

    if (strcmp(template, "NULL") != 0) {
      entryPtr->template = find_template(templates, template);
      entryPtr->header = find_template(templates, header);
      entryPtr->footer = find_template(templates, footer);
    }
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
    char *id = strtok(line, "\t\n");

    Template *templatePtr = create_template(templates, id);

    if (templatePtr == NULL)
      return 0;

    FILE *templateFile = get_file(TEMPLATE_DIR, id, ".html", "r");

    if (templateFile == NULL) {
      printf("File Not Found: %s.html\n", id);
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

  if (!parse_content(entries)) {
    printf("Error Parsing Markdown\n");
    return 0;
  }

  return 1;
}

static void
html_breadcrumbs(FILE *file, Entry *entry) {
  fputs("<ul>\n", file);
  Entry *entry_ptr = entry;
  int i = 0;
  Entry *crumbs[ENTRY_COUNT];
  crumbs[i++] = entry_ptr;

  while (entry_ptr != entry_ptr->parent) {
    entry_ptr = entry_ptr->parent;
    crumbs[i++] = entry_ptr;
  }

  while (i > 1) {
    i--;
    if (crumbs[i]->content_len > 0) {
      fprintf(file, "<li><a href=\"./%s.html\">%s</a> // </li>\n", crumbs[i]->id, crumbs[i]->name);
    } else {
      fprintf(file, "<li>%s // </li>\n", crumbs[i]->name);
    }
  }

  fprintf(file, "<li>%s</li>\n", crumbs[0]->name);

  fputs("</ul>", file);
}

static void
html_nav(FILE *file, Entry *entry, Entry *root, int show_root) {
  int i;
  fputs("<ul>\n", file);

  for (i = 0; i < root->children_len; i++) {
    if (show_root == 0 && root == root->children[i]) {
      // skip
    } else if (root->children[i]->content_len > 0) {
      fprintf(
          file,
          "<li class=\"%s\"><a href=\"./%s.html\">%s</a></li>\n",
          root->children[i] == entry ? "selected" : "",
          root->children[i]->id, root->children[i]->name);
    } else {
      fprintf(file, "<li>%s</li>\n", root->children[i]->name);
    }

    if (root == root->children[i]) {
      continue;
    } else if (root->children[i]->children_len > 0) {
      html_nav(file, entry, root->children[i], show_root);
    }
  }

  fputs("</ul>", file);
}

static void
html_inc_links(FILE *file, Entry *entry) {
  if (entry->incoming_len <= 0) {
    return;
  }

  int i;
  fputs("\t<ul>\n", file);

  for (i = 0; i < entry->incoming_len; i++) {
    fprintf(file, "\t\t<li><a href=\"./%s.html\">%s</a></li>\n", entry->incoming[i]->id, entry->incoming[i]->name);

    if (entry == entry->incoming[i]) {
      continue;
    }
  }

  fputs("\t</ul>", file);
}

static void
output_html_line(FILE *output, char *curLine, Entry *entry, Entries *entries) {
  char *start_ptr = strchr(curLine, '{');
  if (start_ptr) {
    char *end_ptr = strchr(start_ptr + 1, '}');

    *start_ptr = '\0';
    fprintf(output, "%s", curLine);
    size_t size = end_ptr - start_ptr - 1;

    if (strncmp("title", start_ptr + 1, size) == 0) {
      fprintf(output, "%s", entry->name);
    } else if (strncmp("id", start_ptr + 1, size) == 0) {
      fprintf(output, "%s", entry->id);
    } else if ((strncmp("template", start_ptr + 1, size) == 0)) {
      fprintf(output, "%s", entry->template->id);
    } else if (strncmp("nav", start_ptr + 1, size) == 0) {
      html_nav(output, entry, &entries->values[0], 1);
    } else if (strncmp("nav|hide", start_ptr + 1, size) == 0) {
      html_nav(output, entry, &entries->values[0], 0);
    } else if (strncmp("...", start_ptr + 1, size) == 0) {
      html_breadcrumbs(output, entry);
    } else if (strncmp("inc", start_ptr + 1, size) == 0) {
      html_inc_links(output, entry);
    } else if (strncmp("content", start_ptr + 1, size) == 0) {
      fputs(entry->content, output);
    }

    *start_ptr = '{';
    output_html_line(output, end_ptr + 1, entry, entries);
  } else {
    fprintf(output, "%s\n", curLine);
  }
}

static void
output_html(FILE *output, char *curLine, Entry *entry, Entries *entries) {
  while (curLine) {
    char *nextLine = strchr(curLine, '\n');
    if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

    if (*curLine != '\0')
      output_html_line(output, curLine, entry, entries);
    else
      fputs("\n", output);

    if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
    curLine = nextLine ? (nextLine + 1) : NULL;
  }
}

int output_file(Entry *entry, Entries *entries) {
  if (entry->content_len > 0) {
    FILE *output = get_file(OUTPUT_DIR, entry->id, ".html", "w+");
    if (output == NULL) {
      printf("File Not Found: %s.html\n", entry->id);
      return 0;
    }

    output_html(output, entry->header->body, entry, entries);
    output_html(output, entry->template->body, entry, entries);
    output_html(output, entry->footer->body, entry, entries);

    fclose(output);
  }

  return 1;
}

int generate(Entries *entries, Templates *templates) {
  int i;

  for (i = 0; i < entries->length; i++) {
    if (!output_file(&entries->values[i], entries)) {
      return 0;
    }
  }

  return 1;
}

static Entries entries;
static Templates templates;

int main(void) {
  if (!parse(&entries, &templates)) {
    printf("Parsing Error\n");
    return 1;
  }

  if (!generate(&entries, &templates)) {
    printf("Generating Error\n");
    return 1;
  }

  return 0;
}
