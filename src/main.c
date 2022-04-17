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

#define OL "ol"
#define UL "ul"

#define EM "em"
#define STRONG "strong"
#define DEL "del"
#define MARK "code"

#define TRUE 1
#define FALSE 0

#define _S3(s1, R, s2) (strcmp((s1), (s2)) R 0)
#define _S2(s1, s2) (strcmp((s1), (s2)) == 0)
#define _SN4(s1, R, s2, n) (strncmp((s1), (s2), (n)) R 0)
#define _SN3(s1, s2, n) (strncmp((s1), (s2), (n)) == 0)
#define GET_S(_1, _2, _3, NAME, ...) NAME
#define STRCMP(...)            \
  GET_S(__VA_ARGS__, _S3, _S2) \
  (__VA_ARGS__)
#define GET_SN(_1, _2, _3, _4, NAME, ...) NAME
#define STRNCMP(...)              \
  GET_SN(__VA_ARGS__, _SN4, _SN3) \
  (__VA_ARGS__)

#define MAX(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

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
    if (STRCMP(id, entry_ptr->id)) {
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
    if (STRCMP(id, templates->values[i].id)) {
      return &templates->values[i];
    }
  }

  printf("Template Not Found: %s\n", id);

  return NULL;
}

typedef enum list_type {
  ul = 2,
  ol = 3
} list_type;

typedef enum inline_style_type {
  em = 1,
  strong = 2,
  del = 3,
  mark = 4
} inline_style_type;

typedef struct Queue {
  int count;
  int list[32];
} Queue;

static Queue *
create_queue() {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  memset(queue, 0, sizeof(Queue));
  queue->count = 0;
  return queue;
}

static void
queue_push(Queue *queue, int type) {
  queue->list[queue->count++] = type;
}

static int
queue_peek(Queue *queue) {
  return queue->list[(queue->count) - 1];
}

static char *
inline_style_queue_pop(Queue *queue) {
  int type = queue->list[--(queue->count)];
  switch (type) {
    case (int)em:
      return EM;
    case (int)strong:
      return STRONG;
    case (int)del:
      return DEL;
    case (int)mark:
      return MARK;
    default:
      return NULL;
  }
}

static char *
list_queue_pop(Queue *queue) {
  int type = queue->list[--(queue->count)];
  switch (type) {
    case (int)ol:
    case (int)ol + 1:
    case (int)ol + 2:
      return OL;
    case (int)ul:
      return UL;
    default:
      return NULL;
  }
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

static int
md_img(FILE *file, char *curLine) {
  char *start_ptr = strchr(curLine, '!');
  if (STRNCMP(start_ptr, "![", 2)) {
    char *end_ptr = strchr(start_ptr + 1, ']');
  } else {
    return FALSE;
  }

  // fprintf(file, "<a href=\"%s\">%s</a>", linkURL, lineName);

  return TRUE;
}

static int
is_html(char *curLine) {
  char *ptr = curLine;
  if (STRNCMP(ptr, "<", 1)) {
    ptr++;
    char *end_ptr = strchr(ptr, '>');
    if (end_ptr) {
      int length = end_ptr - curLine;

      if (STRNCMP(ptr, "table>", length) ||
          STRNCMP(ptr, "table style", 11) ||
          STRNCMP(ptr, "table class", 11) ||
          STRNCMP(ptr, "table id", 8) ||
          STRNCMP(ptr, "/table>", length) ||

          STRNCMP(ptr, "tr>", length) ||
          STRNCMP(ptr, "tr class", 8) ||
          STRNCMP(ptr, "tr style", 8) ||
          STRNCMP(ptr, "tr id", 5) ||
          STRNCMP(ptr, "/tr>", length) ||

          STRNCMP(ptr, "td>", length) ||
          STRNCMP(ptr, "td class", 8) ||
          STRNCMP(ptr, "td style", 8) ||
          STRNCMP(ptr, "td id", 5) ||
          STRNCMP(ptr, "/td>", length) ||

          STRNCMP(ptr, "div>", length) ||
          STRNCMP(ptr, "div class", 9) ||
          STRNCMP(ptr, "div style", 9) ||
          STRNCMP(ptr, "div id", 6) ||
          STRNCMP(ptr, "/div>", length) ||

          STRNCMP(ptr, "span>", length) ||
          STRNCMP(ptr, "span class", 10) ||
          STRNCMP(ptr, "span style", 10) ||
          STRNCMP(ptr, "span id", 7) ||
          STRNCMP(ptr, "/span>", length) ||

          STRNCMP(ptr, "br />", length) ||

          STRNCMP(ptr, "figure>", length) ||
          STRNCMP(ptr, "figure class", 12) ||
          STRNCMP(ptr, "figure style", 12) ||
          STRNCMP(ptr, "figure id", 9) ||
          STRNCMP(ptr, "/figure>", length) ||

          STRNCMP(ptr, "figcaption>", length) ||
          STRNCMP(ptr, "figcaption class", 16) ||
          STRNCMP(ptr, "figcaption style", 16) ||
          STRNCMP(ptr, "figcaption id", 13) ||
          STRNCMP(ptr, "/figcaption>", length)) {
        return length + 1;
      }
    }
  }

  return FALSE;
}

static int
markdown_image(void *output, char *curLine, int isFile) {
  char altText[1024] = {'\0'};
  char srcText[1024] = {'\0'};
  char *link = "<img src=\"%.*s\" alt=\"%.*s\" />";
  char *ptr = curLine;
  char *endPtr = curLine;

  if (STRNCMP(ptr, "![", 2)) {
    ptr += 2;
    char *altEndPtr = strchr(ptr, ']');
    if (STRNCMP(altEndPtr, "](", 2)) {
      int altLength = altEndPtr - ptr;
      altEndPtr += 2;
      endPtr = strchr(altEndPtr, ')');

      if (endPtr) {
        int srcLength = endPtr - altEndPtr;

        if (isFile) {
          fprintf((FILE *)output, link, srcLength, altEndPtr, altLength, ptr);
        } else {
          sprintf((char *)output, link, srcLength, altEndPtr, altLength, ptr);
        }

        return endPtr - curLine + 1;
      }
    }
  }
  return FALSE;
}

static int output_markdown_image(FILE *output, char *curLine) {
  return markdown_image(output, curLine, TRUE);
}

static int string_markdown_image(char *string, char *curLine) {
  return markdown_image(string, curLine, FALSE);
}

static int
output_markdown_link(FILE *output, char *curLine, Entry *entry, Entries *entries) {
  char linkName[1024] = {'\0'};
  char linkURL[1024] = {'\0'};
  int externalLink = FALSE;
  int linkNameCount = 0;

  char *start_ptr = curLine;  // strchr(curLine, '['),
  char *link_end_ptr;
  if (*start_ptr == '[') {
    linkNameCount = string_markdown_image(&linkName[0], curLine + 1);
    char *end_ptr = strchr(start_ptr + linkNameCount, ']');
    char *link_ptr;

    if (STRNCMP(end_ptr, "]({", 3)) {
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
    } else if (STRNCMP(end_ptr, "](#", 3)) {
      link_ptr = end_ptr + 2;  // include the #
      link_end_ptr = strchr(link_ptr, ')');
      int length = link_end_ptr - link_ptr;
      sprintf(linkURL, "%.*s", length, link_ptr);
    } else if (STRNCMP(end_ptr, "](", 2)) {
      link_ptr = end_ptr + 2;
      link_end_ptr = strchr(link_ptr, ')');
      int length = link_end_ptr - link_ptr;
      sprintf(linkURL, "%.*s", length, link_ptr);

      externalLink = TRUE;
    } else {
      return FALSE;
    }

    size_t nameSize = end_ptr - start_ptr - 1;

    if (linkNameCount > 0) {
      // Don't set linkName
    } else if (nameSize > 0) {
      strncpy(linkName, start_ptr + 1, nameSize);
    } else {
      strcpy(linkName, linkURL);
    }
  } else {
    return FALSE;
  }

  if (externalLink)
    fprintf(output, "<a href=\"%s\" target=\"_blank\">%s</a>", linkURL, linkName);
  else
    fprintf(output, "<a href=\"%s\">%s</a>", linkURL, linkName);

  if (link_end_ptr)
    return (link_end_ptr - start_ptr) + 1;

  return FALSE;
}

static void
output_stripped(FILE *output, char *curLine, int lineLength, int isEscaped, Entry *entry, Entries *entries) {
  int i;
  for (i = 0; i < lineLength; i++) {
    char curChar = *(curLine + i);

    switch (curChar) {
      case '<':
        if (isEscaped)
          fprintf(output, "%s", "&lt;");
        else
          fprintf(output, "%s", "<span class=\"tag\">&lt;</span><span class=\"element\">");
        break;
      case '>':
        if (isEscaped)
          fprintf(output, "%s", "&gt;");
        else
          fprintf(output, "%s", "</span><span class=\"tag\">&gt;</span>");
        break;
      default:
        fputc(curChar, output);
        break;
    }
  }
}

static int
is_inline_style(FILE *output, char *curLine, Queue *queue, char *md, char *html, int tag) {
  char *ptr = curLine;
  int length = strlen(md);
  if (STRNCMP(ptr, md, length)) {
    char *end_ptr = ptr + length;

    if (queue_peek(queue) == tag) {
      if (STRNCMP((curLine - length), md, length))
        return FALSE;
      else
        fprintf(output, "</%s>", inline_style_queue_pop(queue));
      return length;
    }

    while (*(end_ptr) != '\0' && *(end_ptr + 1) != '\0') {
      if (STRNCMP(end_ptr + 2, md, length)) {
        fprintf(output, "<%s>", html);
        queue_push(queue, tag);
        return length;
      }
      end_ptr++;
    }
  }
  return FALSE;
}

static void
output_markdown_line(FILE *output, char *curLine, int lineLength, int isEscaped, Entry *entry, Entries *entries) {
  int i, length;
  Queue *inlineStyleQueue = create_queue();

  for (i = 0; i < lineLength;) {
    char *curLinePtr = curLine + i;

    if (*curLinePtr == '\0') {
      break;
    } else if (!isEscaped && *curLinePtr == '\\') {
      isEscaped = TRUE;
      i++;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, inlineStyleQueue, "**", STRONG, strong))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, inlineStyleQueue, "*", EM, em))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, inlineStyleQueue, "~~", DEL, del))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, inlineStyleQueue, "`", MARK, mark))) {
      i += length;
    } else if (!isEscaped && (length = is_html(curLinePtr))) {
      fprintf(output, "%.*s", length, curLinePtr);
      i += length;
    } else if (!isEscaped && (length = output_markdown_image(output, curLinePtr))) {
      i += length;
    } else if (!isEscaped && (length = output_markdown_link(output, curLinePtr, entry, entries))) {
      i += length;
    } else {
      output_stripped(output, curLinePtr, 1, TRUE, entry, entries);
      isEscaped = FALSE;
      i++;
    }
  }

  free(inlineStyleQueue);
}

static Queue *
reset_list(FILE *output, Queue *listLevel, int listOpen) {
  while (listLevel->count > 0) {
    if (listOpen) {
      fputs("\n</li>", output);
    }
    fprintf(output, "\n</%s>", list_queue_pop(listLevel));
    if (!listOpen && listLevel->count > 0) {
      fputs("</li>", output);
    }
  }

  return listLevel;
}

static int
set_blockquote(FILE *output, char *curLine, int blockquoteOpen) {
  int i = 0;
  while (STRNCMP(curLine, ">>>>", i + 1) && i <= 4) {
    i++;
  }

  while (i > blockquoteOpen) {
    fprintf(output, "\n%s", "<blockquote>");
    blockquoteOpen++;
  }

  while (i < blockquoteOpen) {
    fprintf(output, "\n%s", "</blockquote>");
    blockquoteOpen--;
  }

  return blockquoteOpen;
}

static int
is_header(char *curLine) {
  if (STRNCMP(curLine, "# ", 2)) {
    return 1;
  } else if (STRNCMP(curLine, "## ", 3)) {
    return 2;
  } else if (STRNCMP(curLine, "### ", 4)) {
    return 3;
  } else if (STRNCMP(curLine, "#### ", 5)) {
    return 4;
  } else if (STRNCMP(curLine, "##### ", 6)) {
    return 5;
  } else if (STRNCMP(curLine, "###### ", 7)) {
    return 6;
  }

  return FALSE;
}

static int
is_list(char *curLine) {
  if (STRNCMP(curLine, "- ", 2)) {
    return (int)ul;
  } else if (isdigit(*curLine) && STRNCMP(curLine + 1, ". ", 2)) {
    return (int)ol;
  } else if (isdigit(*curLine) && isdigit(*(curLine + 1)) && STRNCMP(curLine + 2, ". ", 2)) {
    return (int)ol + 1;
  } else if (isdigit(*curLine) && isdigit(*(curLine + 1)) && isdigit(*(curLine + 2)) && STRNCMP(curLine + 3, ". ", 2)) {
    return (int)ol + 2;
  }

  return FALSE;
}

static void
output_markdown(FILE *output, char *buffer, Entry *entry, Entries *entries) {
  char *curLine = buffer;
  int prevIndent = 0;
  Queue *listLevel = create_queue();
  int codeOpen = FALSE, listOpen = FALSE, blockquoteOpen = FALSE;
  void (*output_line)(FILE * output, char *curLine, int lineLength, int isEscaped, Entry *entry, Entries *entries);

  while (curLine) {
    char *nextLine = strchr(curLine, '\n');
    if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

    char openTag[100] = {'\0'};
    char closeTag[100] = {'\0'};
    int indent = 0;
    output_line = output_markdown_line;
    int isEscaped = FALSE;

    if ((blockquoteOpen = set_blockquote(output, curLine, blockquoteOpen))) {
      curLine += blockquoteOpen;
    }

    while (isblank(*curLine)) {
      if (*curLine == '\t')
        indent += TAB_SIZE;
      else
        indent++;
      curLine++;
    }

    int header, listType;
    if (STRNCMP(curLine, "\\", 1)) {
      listLevel = reset_list(output, listLevel, listOpen);
      listOpen = FALSE;
      isEscaped = TRUE;
      strcpy(openTag, "<p>");
      strcpy(closeTag, "</p>");
      curLine++;
    } else if (STRNCMP(curLine, "```", 3)) {
      curLine += 3;
      while (isblank(*curLine)) {
        curLine++;
      }

      if (codeOpen) {
        strcpy(openTag, "</code>\n</pre>");
        codeOpen = FALSE;
      } else {
        sprintf(openTag, "<pre>\n<code class=\"%s\">", curLine);
        codeOpen = TRUE;
      }

      while (*curLine != '\0') {
        curLine++;
      }
    } else if (codeOpen) {
      output_line = output_stripped;
    } else if (is_html(curLine) > 0) {
      // do nothing
    } else if (STRNCMP(curLine, "![", 2)) {
      // do nothing
    } else if (STRNCMP(curLine, "***", MAX(nextLine ? nextLine - curLine : 0, 3))) {
      strcpy(closeTag, "<hr />");
      curLine += 3;
    } else if ((header = is_header(curLine)) > 0 && indent <= TAB_SIZE * 2) {
      listLevel = reset_list(output, listLevel, listOpen);
      listOpen = FALSE;
      sprintf(openTag, "<h%i>", header);
      sprintf(closeTag, "</h%i>", header);
      curLine += header + 1;
    } else if ((listType = is_list(curLine))) {
      if (listLevel->count == 0 || indent - prevIndent == TAB_SIZE) {
        strcpy(&openTag[0], "<");
        switch (listType) {
          case (int)ol:
            strcpy(&openTag[1], OL);
            break;
          case (int)ul:
            strcpy(&openTag[1], UL);
            break;
        }
        strcpy(&openTag[3], ">\n<li>");
        listOpen = TRUE;
        queue_push(listLevel, listType);
      } else if (indent < prevIndent) {
        int i, tagIndex;
        for (i = 0, tagIndex = 0; i < (prevIndent - indent) / TAB_SIZE; i++) {
          if (i == 0) {
            strcpy(&openTag[0], "</li>\n");
            strcpy(&openTag[6], "</");
            strcpy(&openTag[8], list_queue_pop(listLevel));
            strcpy(&openTag[10], ">");
            tagIndex = 11;
          } else {
            strcpy(&openTag[tagIndex], "\n</li>");
            strcpy(&openTag[tagIndex + 6], "\n</");
            strcpy(&openTag[tagIndex + 9], list_queue_pop(listLevel));
            strcpy(&openTag[tagIndex + 11], ">");
            tagIndex += 12;
          }
        }

        if (i == 0)
          strcpy(&openTag[0], "<li>");
        else
          strcpy(&openTag[tagIndex], "\n</li>\n<li>");

        listOpen = TRUE;
      } else {
        if (listOpen) {
          strcpy(openTag, "</li>\n<li>");
        } else {
          strcpy(openTag, "<li>");
        }

        listOpen = TRUE;
      }

      curLine += listType;
    } else if (*curLine == '\0' && *(curLine + 1) == '\n') {
      listLevel = reset_list(output, listLevel, listOpen);
      listOpen = FALSE;
      strcpy(openTag, "<br />");
    } else if (*curLine != '\0') {
      if (indent > prevIndent && listLevel->count > 0) {
        indent = prevIndent;
        strcpy(closeTag, "</p></li>");
        listOpen = FALSE;
      } else {
        listLevel = reset_list(output, listLevel, listOpen);
        listOpen = FALSE;
        strcpy(closeTag, "</p>");
      }
      strcpy(openTag, "<p>");
    } else {
      listLevel = reset_list(output, listLevel, listOpen);
      listOpen = FALSE;
    }

    fputc('\n', output);

    int i;
    for (i = 0; i < indent; i++) {
      fputc(' ', output);
    }

    int tagLength = strlen(openTag) - 4;
    if (STRNCMP(openTag + tagLength, "<li>", 4) && STRNCMP(curLine, "[ ] ", 4)) {
      fprintf(output, "%.*s", tagLength, openTag);
      fprintf(output, "%s", "<li class=\"task-list-item\"><input type=\"checkbox\" class=\"task-list-item-checkbox\" disabled />");
      curLine += 4;
    } else if (STRNCMP(openTag + tagLength, "<li>", 4) && STRNCMP(curLine, "[x] ", 4)) {
      fprintf(output, "%.*s", tagLength, openTag);
      fprintf(output, "%s", "<li class=\"task-list-item\"><input type=\"checkbox\" class=\"task-list-item-checkbox\" disabled checked/>");
      curLine += 4;
    } else {
      fprintf(output, "%s", openTag);
    }

    int lineLength = nextLine ? nextLine - curLine : 0;
    output_line(output, curLine, lineLength, isEscaped, entry, entries);

    fprintf(output, "%s", closeTag);

    isEscaped = FALSE;

    if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
    curLine = nextLine ? (nextLine + 1) : NULL;
    prevIndent = indent;
  }

  free(listLevel);
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

    memset(contentBuffer, 0, contentBufferLength);
    fseek(contentFile, 0, SEEK_END);
    contentBufferLength = ftell(contentFile);
    fseek(contentFile, 0, SEEK_SET);

    printf("File size: %ld bytes\n", contentBufferLength);

    if (contentBufferLength > (1024 * 1024)) {
      printf("Not Enough Memory - Content file: %s.md is too large\n", entryPtr->id);
      return FALSE;
    }

    fread(contentBuffer, contentBufferLength + 1, 1, contentFile);
    fclose(contentFile);

    FILE *output = get_file(TEMP_DIR, entryPtr->id, ".txt", "w+");

    if (output == NULL) {
      printf("File Not Found: %s.txt\n", entryPtr->id);
      return FALSE;
    }

    output_markdown(output, contentBuffer, entryPtr, entries);
    fseek(output, 0, SEEK_END);
    entryPtr->content_len = ftell(output);
    fseek(output, 0, SEEK_SET);
    fread(entryPtr->content, entryPtr->content_len + 1, 1, output);
    fclose(output);
    delete_file(TEMP_DIR, entryPtr->id, ".txt");
  }

  return TRUE;
}

#pragma endregion

int parse_entries(Entries *entries, Templates *templates) {
  FILE *entryFile = get_file(MODEL_DIR, "entries", ".tsv", "r");

  if (entryFile == NULL) {
    printf("entries.tsv File Not Found: \n");
    return FALSE;
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

    if (STRCMP(template, !=, "NULL")) {
      entryPtr->template = find_template(templates, template);
      entryPtr->header = find_template(templates, header);
      entryPtr->footer = find_template(templates, footer);
    }
  }

  fclose(entryFile);
  return TRUE;
}

int parse_templates(Templates *templates) {
  FILE *templateModel = get_file(MODEL_DIR, "templates", ".tsv", "r");

  if (templateModel == NULL) {
    printf("File Not Found: templates.tsv\n");
    return FALSE;
  }

  char line[2 * ENTRY_SIZE];
  printf("%s", fgets(line, sizeof line, templateModel));  // skip header

  while (fgets(line, sizeof line, templateModel)) {
    char *id = strtok(line, "\t\n");

    Template *templatePtr = create_template(templates, id);

    if (templatePtr == NULL)
      return FALSE;

    FILE *templateFile = get_file(TEMPLATE_DIR, id, ".html", "r");

    if (templateFile == NULL) {
      printf("File Not Found: %s.html\n", id);
      return FALSE;
    }

    fseek(templateFile, 0, SEEK_END);
    templatePtr->body_len = ftell(templateFile);
    fseek(templateFile, 0, SEEK_SET);
    templatePtr->body = (char *)malloc(templatePtr->body_len);
    if (templatePtr->body) {
      fread(templatePtr->body, 1, templatePtr->body_len, templateFile);
    } else {
      printf("Not enough memory");
      return FALSE;
    }

    fclose(templateFile);
  }

  fclose(templateModel);
  return TRUE;
}

int parse(Entries *entries, Templates *templates) {
  if (!parse_templates(templates)) {
    printf("Error Parsing Templates\n");
    return FALSE;
  }

  if (!parse_entries(entries, templates)) {
    printf("Error Parsing Entries\n");
    return FALSE;
  }

  if (!parse_content(entries)) {
    printf("Error Parsing Markdown\n");
    return FALSE;
  }

  return TRUE;
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
html_nav(FILE *file, Entry *entry, Entry *root, int show_root, int level) {
  int i;
  if (level > 0)
    fputs("\n<ul>\n", file);
  else
    fputs("<ul>\n", file);

  for (i = 0; i < root->children_len; i++) {
    if (!show_root && root == root->children[i]) {
      // skip
    } else if (root->children[i]->content_len > 0) {
      fprintf(
          file,
          "<li class=\"%s\"><a href=\"./%s.html\">%s</a>",
          root->children[i] == entry ? "selected" : "",
          root->children[i]->id, root->children[i]->name);
    } else {
      fprintf(file, "<li>%s", root->children[i]->name);
    }

    if (root == root->children[i]) {
      continue;
    } else if (root->children[i]->children_len > 0) {
      html_nav(file, entry, root->children[i], show_root, level + 1);
    }

    fputs("</li>\n", file);
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

    if (STRNCMP("title", start_ptr + 1, size)) {
      fprintf(output, "%s", entry->name);
    } else if (STRNCMP("id", start_ptr + 1, size)) {
      fprintf(output, "%s", entry->id);
    } else if ((STRNCMP("template", start_ptr + 1, size))) {
      fprintf(output, "%s", entry->template->id);
    } else if (STRNCMP("nav", start_ptr + 1, size)) {
      html_nav(output, entry, &entries->values[0], TRUE, 0);
    } else if (STRNCMP("nav|hide", start_ptr + 1, size)) {
      html_nav(output, entry, &entries->values[0], FALSE, 0);
    } else if (STRNCMP("...", start_ptr + 1, size)) {
      html_breadcrumbs(output, entry);
    } else if (STRNCMP("inc", start_ptr + 1, size)) {
      html_inc_links(output, entry);
    } else if (STRNCMP("content", start_ptr + 1, size)) {
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
      return FALSE;
    }

    output_html(output, entry->header->body, entry, entries);
    output_html(output, entry->template->body, entry, entries);
    output_html(output, entry->footer->body, entry, entries);

    fclose(output);
  }

  return TRUE;
}

int generate(Entries *entries, Templates *templates) {
  int i;

  for (i = 0; i < entries->length; i++) {
    if (!output_file(&entries->values[i], entries)) {
      return FALSE;
    }
  }

  return TRUE;
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
