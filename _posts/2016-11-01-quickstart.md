---
layout: post2
title: Quickstart 
subtitle: 
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-01-quickstart.html
nav:
- page: Download
  permalink: 2016-11-01-quickstart.html#1-get-the-code
- page: Requirements
  permalink: 2016-11-01-quickstart.html#2-check-the-requirements
- page: Compile
  permalink: 2016-11-01-quickstart.html#3-compile-the-code
- page: Sample
  permalink: 2016-11-01-quickstart.html#4-run-jpsreport
- page: Using an IDE
  permalink: 2016-11-01-quickstart.html#using-an-ide
- page: Back to top
  permalink: 2016-11-01-quickstart.html#top
---



To use `JPSreport` there are three steps to follow: 

## 1. Get the code

Git-clone the code from the GitLab repository 

```bash
git clone https://gitlab.version.fz-juelich.de/jupedsim/jpsreport
```

Or download a compressed file 

![structure]({{ site.baseurl}}/img/download.png)

## 2. Check the requirements

Before you get started please check if all [requirements](2016-11-03-requirements.html)  are fulfilled

```bash
cd jpsreport
mkdir build && cd build
cmake ..
```


In case a package is missing, `CMake` will report an error.

## 3. Compile the code

If the two first steps succeed then you can proceed with the compilation 

```bash 
make -j2
```

## 4. Run JPSreport

If the compilation succeeded a binary will be produced in the direction `path/to/jpsreport//bin`.

Run `JPSreport` using one of the ini files in the `demo` directory.

For example in a terminal run the following:

```bash 
./bin/jpsreport inifile.xml
```


## Using an IDE

- [CLion](https://www.jetbrains.com/clion/) (Windows, Linux, Mac)

    *CLion* has build in *CMake*-support so you only have to start a new project an set the project root to your `jpscore` directory.
    Check this [tutorial](https://www.youtube.com/watch?v=Achsd2EpJbI)


- [Visual Studio 2013](http://msdn.microsoft.com/de-de/library/dd831853.aspx) (v0.6+, Windows) 

    Run the *CMake-gui* with the generator *Visual Studio 12 (2013)* and click on the `*.sln` file to open it in Visual Studio.

    Then click on `build->build solution`.
    This should build your code.

    Current issues are known with the python test cases. Most of them are Unix specific.
    However the compilation of `jpscore` works fine. 

- XCode (v0.6+ Mac)

    Run the *CMake-gui* and generate an *XCode*-Project then open it with *XCode*.

    
