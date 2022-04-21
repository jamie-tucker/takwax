#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MODEL_DIR "src/model/"
#define CONTENT_DIR "src/content/"
#define TEMPLATE_DIR "src/template/"
#define OUTPUT_DIR "output/site/"
#define TEMP_DIR "output/.temp/"

#define ENTRY_SIZE 32
#define ENTRY_COUNT 100
#define TAG_SIZE 100

// Markdown Settings
#define TAB_SIZE 2

#define OL "ol"
#define UL "ul"

#define EM "em"
#define STRONG "strong"
#define DEL "del"
#define CODE "code"
#define SUP "sup"
#define SUB "sub"
#define MARK "mark"

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
  time_t modified_date;
  char id[ENTRY_SIZE];
  char body[1024 * 1024];
} Template;

typedef struct Templates {
  int length;
  struct Template values[ENTRY_COUNT];
} Templates;

typedef struct Entry {
  int children_len, incoming_len;
  long content_len;
  time_t created_date, modified_date;
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
  snprintf(entry->id, ENTRY_SIZE, "%s", id);
  snprintf(entry->name, ENTRY_SIZE, "%s", name);
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
  snprintf(template->id, ENTRY_SIZE, "%s", id);

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
  code = 4,
  mark = 5,
  sup = 6,
  sub = 7
} inline_style_type;

typedef struct Queue {
  int count;
  int list[32];
} Queue;

static Queue *
create_queue(Queue *queue) {
  memset(queue->list, 0, 32);
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
    case (int)code:
      return CODE;
    case (int)sub:
      return SUB;
    case (int)sup:
      return SUP;
    default:
      printf("Inline Style Not Handled");
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

static int
get_file_stats(char *dir, char *filename, char *ext, struct stat *stats) {
  char fullpath[1024] = {"\0"};
  strcat(fullpath, dir);
  strcat(fullpath, filename);
  strcat(fullpath, ext);
  return stat(fullpath, stats);
}

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

static void get_formatted_date(char *s, int length, time_t *time, int local) {
  struct tm ct;
  if (local)
    ct = *(localtime(time));
  else
    ct = *(gmtime(time));
  snprintf(s, length, "%02d-%02d-%d %02d:%02d:%02d %s", ct.tm_mon + 1, ct.tm_mday, ct.tm_year + 1900, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_zone);
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

      if (STRNCMP(ptr, "table", 5) ||
          STRNCMP(ptr, "/table", 6) ||

          STRNCMP(ptr, "tr", 2) ||
          STRNCMP(ptr, "/tr", 3) ||

          STRNCMP(ptr, "td", 2) ||
          STRNCMP(ptr, "/td", 3) ||

          STRNCMP(ptr, "div", 3) ||
          STRNCMP(ptr, "/div", 4) ||

          STRNCMP(ptr, "span", 4) ||
          STRNCMP(ptr, "/span", 5) ||

          STRNCMP(ptr, "br />", length) ||

          STRNCMP(ptr, "figure", 6) ||
          STRNCMP(ptr, "/figure", 7) ||

          STRNCMP(ptr, "figcaption", 10) ||
          STRNCMP(ptr, "/figcaption", 11)) {
        return length + 1;
      }
    }
  }

  return FALSE;
}

static int
markdown_image(void *output, char *curLine, int isFile) {
  char altText[1024] = {0};
  char srcText[1024] = {0};
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
          snprintf((char *)output, 2048, link, srcLength, altEndPtr, altLength, ptr);
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
  char linkName[2048] = {0};
  char linkURL[1024] = {0};
  int externalLink = FALSE;
  int linkNameCount = 0;

  char *start_ptr = curLine;  // strchr(curLine, '['),
  char *link_end_ptr;
  if (*start_ptr == '[') {
    linkNameCount = string_markdown_image(&linkName[0], curLine + 1);
    char *end_ptr = strchr(start_ptr + linkNameCount, ']');
    char *link_ptr;

    if (STRNCMP(end_ptr, "]({", 3)) {
      char entryId[32] = {0};
      char linkTarget[64] = {0};
      int targetLength = 0;

      link_ptr = end_ptr + 3;
      char *entryEndPtr = strchr(link_ptr, '#');
      if (entryEndPtr) {
        char *targetEndPtr = strchr(link_ptr, '}');
        targetLength = targetEndPtr - entryEndPtr;
        if (targetLength > 0) {
          sprintf(linkTarget, "%.*s", targetLength, entryEndPtr);
        } else {
          entryEndPtr = targetEndPtr;
        }
      } else {
        entryEndPtr = strchr(link_ptr, '}');
      }

      int entryLength = entryEndPtr - link_ptr;
      sprintf(entryId, "%.*s", entryLength, link_ptr);

      link_end_ptr = strchr(link_ptr, '}');
      int length = link_end_ptr - link_ptr;
      Entry *linkedEntry = find_entry_in_entries(entries, entryId);
      if (linkedEntry) {
        Entry *incEntry = find_entry(linkedEntry->incoming[0], linkedEntry->incoming_len, entry->id);
        if (incEntry == NULL && linkedEntry != entry) {
          linkedEntry->incoming[linkedEntry->incoming_len++] = entry;
        }

        sprintf(linkURL, "./%s.html%s", entryId, linkTarget);
      } else {
        printf("Link Not Found: %s\n", entryId);
      }
      // *link_end_ptr = '}';
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
      case '&':
        fprintf(output, "%s", "&amp;");
        break;
      case '\'':
        fprintf(output, "%s", "&apos;");
        break;
      case '"':
        fprintf(output, "%s", "&quot;");
        break;
      default:
        fputc(curChar, output);
        break;
    }
  }
}

static int
is_inline_style(FILE *output, char *curLine, Queue *queue, char *md, char *html, int tag, int noSpace) {
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
    } else if (*(end_ptr + 1) == ' ') {
      return FALSE;
    } else if (noSpace && isblank(*(ptr - 1))) {
      return FALSE;
    }

    while (*(end_ptr) != '\0' && *(end_ptr + 1) != '\0' && !(noSpace && isblank(*(end_ptr + 1)))) {
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
  Queue inlineStyleQueue = *create_queue(&inlineStyleQueue);

  for (i = 0; i < lineLength;) {
    char *curLinePtr = curLine + i;

    if (*curLinePtr == '\0') {
      break;
    } else if (!isEscaped && *curLinePtr == '\\') {
      isEscaped = TRUE;
      i++;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "**", STRONG, strong, FALSE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "*", EM, em, FALSE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "~~", DEL, del, FALSE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "`", CODE, code, FALSE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "==", MARK, mark, FALSE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "^", SUP, sup, TRUE))) {
      i += length;
    } else if (!isEscaped && (length = is_inline_style(output, curLinePtr, &inlineStyleQueue, "~", SUB, sub, TRUE))) {
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
}

static void
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

  // return listLevel;
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
  Queue listLevel = *create_queue(&listLevel);
  int codeOpen = FALSE, listOpen = FALSE, blockquoteOpen = FALSE;
  void (*output_line)(FILE * output, char *curLine, int lineLength, int isEscaped, Entry *entry, Entries *entries);

  while (curLine) {
    char *nextLine = strchr(curLine, '\n');
    if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

    char openTag[TAG_SIZE] = {0};
    char closeTag[TAG_SIZE] = {0};
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
      reset_list(output, &listLevel, listOpen);
      listOpen = FALSE;
      isEscaped = TRUE;
      snprintf(openTag, TAG_SIZE, "<p>");
      snprintf(closeTag, TAG_SIZE, "</p>");
      curLine++;
    } else if (STRNCMP(curLine, "```", 3)) {
      curLine += 3;
      while (isblank(*curLine)) {
        curLine++;
      }

      if (codeOpen) {
        snprintf(openTag, TAG_SIZE, "</code>\n</pre>");
        codeOpen = FALSE;
      } else {
        snprintf(openTag, TAG_SIZE, "<pre>\n<code class=\"%s\">", curLine);
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
      snprintf(closeTag, TAG_SIZE, "<hr />");
      curLine += 3;
    } else if ((header = is_header(curLine)) > 0 && indent <= TAB_SIZE * 2) {
      reset_list(output, &listLevel, listOpen);
      listOpen = FALSE;
      snprintf(openTag, TAG_SIZE, "<h%i>", header);
      snprintf(closeTag, TAG_SIZE, "</h%i>", header);
      curLine += header + 1;

      char *idStart, *idEnd;
      if ((idStart = strchr(curLine, '{')) && *(idStart + 1) == '#' && (idEnd = strchr(idStart + 1, '}'))) {
        *idStart = '\0';
        idStart += 2;
        int length = idEnd - idStart;
        snprintf(openTag, TAG_SIZE, "<h%i id=\"%.*s\">", header, length, idStart);
      }

    } else if ((listType = is_list(curLine))) {
      if (listLevel.count == 0 || indent - prevIndent == TAB_SIZE) {
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
        queue_push(&listLevel, listType);
      } else if (indent < prevIndent) {
        int i, tagIndex;
        for (i = 0, tagIndex = 0; i < (prevIndent - indent) / TAB_SIZE; i++) {
          if (i == 0) {
            strcpy(&openTag[0], "</li>\n");
            strcpy(&openTag[6], "</");
            strcpy(&openTag[8], list_queue_pop(&listLevel));
            strcpy(&openTag[10], ">");
            tagIndex = 11;
          } else {
            strcpy(&openTag[tagIndex], "\n</li>");
            strcpy(&openTag[tagIndex + 6], "\n</");
            strcpy(&openTag[tagIndex + 9], list_queue_pop(&listLevel));
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
          snprintf(openTag, TAG_SIZE, "</li>\n<li>");
        } else {
          snprintf(openTag, TAG_SIZE, "<li>");
        }

        listOpen = TRUE;
      }

      curLine += listType;
    } else if (*curLine == '\0' && *(curLine + 1) == '\n') {
      reset_list(output, &listLevel, listOpen);
      listOpen = FALSE;
      snprintf(openTag, TAG_SIZE, "<br />");
    } else if (*curLine != '\0') {
      if (indent > prevIndent && listLevel.count > 0) {
        indent = prevIndent;
        snprintf(closeTag, TAG_SIZE, "</p></li>");
        listOpen = FALSE;
      } else {
        reset_list(output, &listLevel, listOpen);
        listOpen = FALSE;
        snprintf(closeTag, TAG_SIZE, "</p>");
      }
      snprintf(openTag, TAG_SIZE, "<p>");
    } else {
      reset_list(output, &listLevel, listOpen);
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
}

int parse_content(Entries *entries) {
  int i;
  char contentBuffer[1024 * 1024];
  long contentBufferLength = 0;

  for (i = 0; i < entries->length; i++) {
    printf("\n");
    Entry *entryPtr = &entries->values[i];

    FILE *contentFile = get_file(CONTENT_DIR, entryPtr->id, ".md", "r");

    if (contentFile == NULL) {
      printf("File Not Found %s.md\n", entryPtr->id);
      continue;
    } else {
      struct stat stats;
      if (get_file_stats(CONTENT_DIR, entryPtr->id, ".md", &stats) == 0) {
        entryPtr->created_date = stats.st_birthtime;
        entryPtr->modified_date = stats.st_mtime;

        char modifiedDate[24] = {0}, createdDate[24] = {0};
        get_formatted_date(&modifiedDate[0], COUNT(modifiedDate), &(entryPtr->modified_date), TRUE);
        get_formatted_date(&createdDate[0], COUNT(createdDate), &(entryPtr->created_date), TRUE);
        printf("Created on: %s\n", createdDate);
        printf("Modified on: %s\n", modifiedDate);
      }
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

    FILE *output = NULL;
    // struct stat stats;
    // if (get_file_stats(TEMP_DIR, entryPtr->id, ".html", &stats) == 0) {
    //   if (stats.st_mtime >= entryPtr->modified_date) {
    //     output = get_file(TEMP_DIR, entryPtr->id, ".html", "r");

    //     if (output != NULL) {
    //       int i;
    //       for (i = 0; i < contentBufferLength; i++) {
    //         output_markdown_link(output, &contentBuffer[i], entryPtr, entries);
    //       }
    //     }
    //   }
    // }

    if (output == NULL) {
      output = get_file(TEMP_DIR, entryPtr->id, ".html", "w+");

      if (output == NULL) {
        printf("File Not Found: %s.html\n", entryPtr->id);
        return FALSE;
      }

      output_markdown(output, contentBuffer, entryPtr, entries);
    }

    fseek(output, 0, SEEK_END);
    entryPtr->content_len = ftell(output);
    fseek(output, 0, SEEK_SET);
    fread(entryPtr->content, entryPtr->content_len + 1, 1, output);
    fclose(output);
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
    } else {
      struct stat stats;
      if (get_file_stats(TEMPLATE_DIR, id, ".html", &stats) == 0) {
        templatePtr->modified_date = stats.st_mtime;
      }
    }

    fseek(templateFile, 0, SEEK_END);
    templatePtr->body_len = ftell(templateFile);
    fseek(templateFile, 0, SEEK_SET);
    if (sizeof(templatePtr->body) >= templatePtr->body_len) {
      fread(templatePtr->body, 1, templatePtr->body_len, templateFile);
    } else {
      printf("Template Body not large enough: %s", templatePtr->id);
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
    } else if (STRNCMP("modified_date", start_ptr + 1, size)) {
      char date[24] = {0};
      get_formatted_date(&date[0], COUNT(date), &entry->modified_date, FALSE);
      fprintf(output, "%s", date);
    } else if (STRNCMP("created_date", start_ptr + 1, size)) {
      char date[24] = {0};
      get_formatted_date(&date[0], COUNT(date), &entry->created_date, FALSE);
      fprintf(output, "%s", date);
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
    struct stat stats;
    if (get_file_stats(OUTPUT_DIR, entry->id, ".html", &stats) == 0) {
      time_t lastModified = stats.st_mtime;
      if (lastModified >= entry->modified_date &&
          lastModified >= entry->header->modified_date &&
          lastModified >= entry->template->modified_date &&
          lastModified >= entry->footer->modified_date) {
        printf("File Not Modified: %s\n", entry->id);
        return TRUE;
      }
    }

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
