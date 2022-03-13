# Spec

## HTML Templates

### Title

{title}

```
<title>page title</title>
```

### Navigation

{nav}

```
<nav aria-label="primary">
  <ul>
    <li></li>
  </ul>
</nav>
```

### Breadcrumbs

{...}

```
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

```
<nav aria-labelledby="inc">
  <h6 id="inc">Incoming Links</h6>
  <ul>
    <li></li>
  </ul>
</nav>
```

## MD Content

### Links

[link name](http://) External Link
[link name]({index}) Internal Link

```
<a href="http://">link name</a>
<a href="link.html">{link name}</a>
```

### Headings

`# , ## , ### , #### , ##### , ######`

```
<h1>
<h2>
<h3>
<h4>
<h5>
<h6>
```

### Lists

- UL

```
<ul>
  <li>UL</li>
</ul>
```

1. OL 

```
<ol>
   <li>OL</li>
</ul>
```

### Text formatting

**word** Bold <strong>word<strong>
*word* Italic <em>word</em>
~~word~~ Strikethrough <del>word</del>

### Images

![alt text](http://image.jpg")
![alt text]({link.jpg})
![alt text]({link.jpg} "This is a caption")

```
<img src="link.jpg" alt="alt text" />

<figure>
   <img src=“link.jpg” alt=“alt text”>
   <figcaption>This is a caption</figcaption>
</figure>
```