# Takwax

Takwax is the [Munsee](https://en.wiktionary.org/wiki/takwax) word for Turtle 🐢
The world is said to have been created on the back of a turtle as a place for sky-woman to live when she came down from the sky-world. Thus I have decided to name this project `Takwax` as I intend to use it as a foundation and system for the creation of future projects.

This first iteration of takwax is a static website creation tool.

## Intent

The primary inspiration for this project has come from the [Oscean](https://github.com/XXIIVV/oscean) wiki engine which runs [wiki.xxiivv.com](https://wiki.xxiivv.com/site/home.html) created by [Devine Lu Linvega](https://merveilles.town/@neauoire).

I had been thinking about the problem of organizing the journals/notes that I write on my computer. From Devine, I learned about the concept of the [Memex](https://en.wikipedia.org/wiki/Memex), which was first theorized in 1945 by Vannevar Bush.

```text
Consider a future device … in which an individual stores all his books, records, and communications, and which is mechanized so that it may be consulted with exceeding speed and flexibility. It is an enlarged intimate supplement to his memory.
```

- [As We May Think](https://www.theatlantic.com/magazine/archive/1945/07/as-we-may-think/303881/)

The intent of this site is to become a depot for my journals, research, and projects. The entire body of work should exist in plain text. The creation of a new page should have minimal overhead with the option to embed multi-media. Each entry should be interconnected, so that one may use the site more akin to flipping through pages of a physical notebook.

## TODO

- [x] Basic Model
  - [x] TSV for Entry
  - [x] Rootless entries (i.e.: lists, supporting documents, images, etc...)
- [x] File reading
  - [x] Print to screen.
- Parse MD
  - Block elements
    - Headers
      - [x] H1-H6
      - [x] Heading Ids ### My Great Heading {#custom-id}
    - Lists
      - [x] UL Lists
        - [x] Nested lists
      - [x] OL Lists
      - [x] nested paragraphs
      - [x] valid HTML
      - [x] Checkboxes
    - Definition lists
      - [ ] Definition list \<dl>
      - [ ] Definition title \<dt>
      - [ ] Definition \<dd>
    - [x] Paragraphs
    - [x] line breaks
    - [x] ~~ID (NTH)~~
    - [x] ~~Class (NTH)~~
    - [x] HR tag
    - Block quotes
      - [x] \<blockquote>
      - [x] nested
      - [x] work with lists, p, header, inline
  - Inline Elements
    - Links
      - [x] Internal
      - [x] Internal with page anchors
      - [x] External
      - [x] same-page anchors
    - Images
      - [x] markdown images
      - [ ] New Model for Images/Media
      - [x] Support captions with \<figure> tags
      - [x] Links can handle images [![image description](file.png)](link.htm)
      - [ ] Gallery view / image viewer
    - Text formatting
      - [x] escaped characters '\\'
      - [x] **strong**
      - [x] ~~del~~
      - [x] *em*
      - [x] `code`
      - [x] ==mark==
      - [x] ~sub~
      - [x] ^sup^
    - Footnotes
      - [ ] \[^1] Inline part
      - [ ] \[^1]: Block part
    - [x] HTML formatting / `<code>` tags
    - [x] refactor markdown output checks to return int
  - Strip characters
    - [x] \& `&amp;`
    - [x] \' `&apos;`
    - [x] \" `&quot;`
    - [x] \> `&gt;`
    - [x] \< `&lt;`
- [x] Move MD to parse stage in order to capture Incoming links?
- [x] Parse HTML Template.
- [x] Create HTML Output.
- [ ] Add support for {blog} so that they can be output as \<article> in a loop on a category page with links to pages with formatting.
- RSS feed
- Sitemap
  - [ ] xml
    - [ ] robots.txt
  - [ ] human readable
- Error handling
  - [ ] Throw errors when parsing... Too many silent fails.
  - [ ] Watch for sprintf, and buffer sizes.
- [x] Get dates of files
  - [x] Don't process files that don't need to be updated
- Template tags
  - [x] Add {modified_date}
  - [x] Add {created_date}

## LICENSE

The source code is supplied as-is under the [MIT License](https://github.com/jamie-tucker/takwax/blob/main/LICENSE) and all media and written site content are [BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) unless otherwise noted.
