# Spec

## HTML Templates

### Title

{title}

```text
page title
```

{id}

```text
entry_id
```

{template}

```text
template_id
```

### Navigation

{nav}, {nav|hide}

Hides root entry from main navigation.

```html
<nav aria-label="primary">
  <ul>
    <li></li>
  </ul>
</nav>
```

### Breadcrumbs

{...}

```html
<nav aria-label="breadcrumbs">
  <ul>
    <li></li>
  </ul>
</nav>
```

### Content

{content}

outputs markdown content file

### Incoming Links

{inc}

hides h2 if there are no incoming links

```css
nav > h2 {
  display: block;
}

nav > h2:only-child {
  display: none;
}
```

```html
<nav aria-labelledby="inc">
  <h2 id="inc">Incoming Links</h2>
  <ul>
    <li></li>
  </ul>
</nav>
```

## MD Content

### Links

[link name](http://) External Link
[link name]({index}) Internal Link

```html
<a href="http://">link name</a>
<a href="link.html">{link name}</a>
```

### Headings

```text
#
##
###
####
#####
######
```

```html
<h1>
<h2>
<h3>
<h4>
<h5>
<h6>
```

### Lists

- UL

```html
<ul>
  <li>UL</li>
</ul>
```

1. OL

```html
<ol>
   <li>OL</li>
</ol>
```

### Text formatting

**word** Bold `<strong>word<strong>`
*word* Italic `<em>word</em>`
~~word~~ Strikethrough `<del>word</del>`

### Images

![alt text](http://image.jpg")
![alt text]({link.jpg})
![alt text]({link.jpg} "This is a caption")

```html
<img src="link.jpg" alt="alt text" />

<figure>
   <img src=“link.jpg” alt=“alt text”>
   <figcaption>This is a caption</figcaption>
</figure>
```
