# **Kindle Manga Maker** #

## Author ##

[Vinícius de Ávila Jorge](mailto:vinicius.avila.jorge@gmail.com)

## Translation notes ##

The interface is written in English. Because I'm not native English speaker, mistakes can appear. If it happens, let me know emailing me. I will provide Brazilian Portuguese translation soon (I'm Brazilian). Help in translation is appreciated. If you want to translate the program to your language, open the kmm-en.ts file provided with this program in Qt Linguist, make the translation, save and email me the translated file. I will embed your translation and give you the credit, of course.

## Motivation ##

I've created this small application as a private project because I love mangas (Japanese Comics), Kindle and Linux. Since I bought a Kindle Paperwhite 2nd, I'm used to reading many books, both bought from Amazon Store and some free books. And also I have discovered that it's very good to read mangas in Kindle as well.

For around three years I was a little far from Linux community because I own a Macbook Air and using its trackpad in Linux was terrible because the lack of gesture support (and OSX was enough for me). But since mtrack drives were improved, and some gestures now is supported I came back to our freedom and I released myself from Apple's claws. Now I'm running Archlinux, my favorite distro (Keep It Simple Stupid!).

Back when I used OSX, I used to create my *.mobi manga files using the tool Kindle Comic Creator (KCC), that is provided by Amazon itself both for OSX and Windows. But with no support to Linux what is pretty strange, because I'm not sure but I think they use Qt as UI Framework so, compiling it for Linux would be "piece of cake". But for whatever reason Amazon did not this. Only the command line tool "kindlegen" is available for Linux.

That was MY reason to create this program. I want a KCC replacement that runs natively in Linux (no wine stuff), that is easy to use and lightweight (simple is better). It still requires kindlegen to compile the project files to MOBI format. Afterwards, you can transfer the file to the documents folder inside Kindle via USB or using Calibre or other tool. Or you can send the file to your Amazon email (**something_XX@kindle.com**) that you chan check accessing *Menu > Settings > Device Options > Customize your Kindle > Kindle email address*. This document will be available for download in your device.

This program is designed to make documents compatible with **Kindle Paperwhite 2nd Gen**. You can try to test if other devices will read correctly the documents generated by this program. I cannot do this tests so your help will be pretty appreciated. If any problem is detected, try to report a bug or send me an email [vinicius.avila.jorge@gmail.com](mailto:vinicius.avila.jorge@gmail.com)

## Version ##

The current version is named 0.1. It's a development version and can get new modifications without number change release. When all main function are implemented, I will lock these release numbers using the tag system from git.

## Install ##

The installation is pretty straight if you have already the right packages. Basically you have to install these programs from your distribution:
* qt5 (some distributions like Ubuntu make dev files, as includes, in a separated package terminated in -dev). Please refer to your distribution package manager.
* python2
* libzip
* compile tools (gcc, etc). Archlinux uses with *base-devel* group installed will have all necessary. Ubuntu users have to install the metapackage *build-essential*.

Having installed all necessary stuff, let's go to the steps

* Clone this repository with one of the two commands below:


```
#!bash
git clone git@bitbucket.org:viniavila/kindle-manga-maker.git
git clone https://viniavila@bitbucket.org/viniavila/kindle-manga-maker.git
```

So go to the folder:

```
#!bash
cd kindle-manga-maker/
```

* Compile the program with

```
#!bash

qmake kindle-manga-maker.pro
make

```

* Double click the file compiled.

I will make a better way to install the binary directly along a nice .desktop launcher.
For Arch users, an AUR package is planned. Any Ubuntu user is free to package this program into a .deb file. Other distros users are free to package the program as well.


You have to install kindlegen if you want to create MOBI files. Please visit [Amazon website](https://www.amazon.com/gp/feature.html?docId=1000765211) to download it. Archlinux users can also install kindlegem from [AUR](https://aur.archlinux.org/packages/kindlegen)

Having kindlegen installed is not mandatory as this program can generate .CBZ, .CBR and .EPUB files aside .MOBI files.