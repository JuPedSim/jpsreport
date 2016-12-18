---
layout: post
title: Contributing
subtitle: to JuPedSim
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-04-contributing.html
---





This project is mainly developed by a small group of researchers and students from [Jülich Research Center](http://www.fz-juelich.de/en) and [BUW](http://www.uni-wuppertal.de/).
However, you are kindly invited not only to use `JuPedSim` but also contributing to this project.
It does not matter if you are a researcher, student or just interested in pedestrian dynamics.


## New contributors

### First steps
* Clone/fork our `gitlab` repository

  ~~~
  git clone https://gitlab.version.fz-juelich.de/jupedsim/jpsreport.git
  ~~~

* Change to the developement branch and create a branch with your feature.

  ~~~
  git checkout developement 
  git checkout -b feature_name
  ~~~

* Assuming you are in the `jpscore` folder type 

  ```
  mkdir build && cd build 
  cmake .. 
  ```

* Download all [dependencies](2016-11-03-requirements.html and check the output of `CMake`
    
* If everything compiles fine, you are free to start. :-)

### Workflow
The branches **master** and **develop** are **protected**. You can **only push to your feature-branch.**  
Whenever you want to add a feature or fix an issue create a new feature-branch and only work on this branch.  
Once you finish your fixes/changes merge the branch `develop` in your branch.
Make sure all tests are running before merging and you provided new tests for your feature.
After doing that open a merge request.

If your fix/feature is accepted it will be merged into the develop-branch.



## Reporting bugs and requesting features
If you have a question or a problem and need support from our team feel free to contact us.
You can do this via [email](mailto:dev@jupedsim.org).

If you think you found an issue or bug in `JuPedSim` please use our issue tracker.

### Using the issue tracker

The issue tracker is the preferred channel for bug reports, features requests and submitting pull requests, but please respect the following restrictions:

- Please do not use the issue tracker for personal support requests. [Mail us](mailto:dev@jupedsim.org) if you need personal support.

- Please do not derail or troll issues. Keep the discussion on topic and respect the opinions of others.

If you use the issue tracker we have a list of labels you should use.

### Reporting bugs

If you find a bug in the source code or a mistake in the documentation, you can help us by submitting an issue to our repository.
Even better you can submit a pull or merge request with a fix. Please use the following template and make sure you provide us as much information as possible:

~~~.md
[Short description of problem here]

**Reproduction Steps:**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Expected behavior:**

[Describe expected behavior here]

**Observed behavior:**

[Describe observed behavior here]

**Screenshots and GIFs**

![Screenshots and GIFs which follow reproduction steps to demonstrate the problem](url)

**JuPedSim version:** [Enter JuPedSim version here]
**OS and version:** [Enter OS name and version here]
**Compiler and version:** [Enter compiler name and version here]

**Installed Libraries:**
[Enter Boost version here]
[Enter Vtk version here]
[Enter Qt version here]

**Additional information:**

* Problem started happening recently, didn't happen in an older version of JuPedSim: [Yes/No]
* Problem can be reliably reproduced, doesn't happen randomly: [Yes/No]
* Problem happens with all files and projects, not only some files or projects: [Yes/No]
* Problem happens with the attached ini and geometry files: [Yes/No]
~~~

### Requesting features

Enhancement suggestions are tracked as issues. After you've determined which repository your enhancement suggestions is related to, create an issue on that repository and provide the following information:

 * Use a clear and descriptive title for the issue to identify the suggestion.
 * Provide a step-by-step description of the suggested enhancement in as many details as possible.
 * Provide specific examples to demonstrate the steps. Include copy/pasteable snippets which you use in those examples, as Markdown code blocks.
 * Describe the current behavior and explain which behavior you expected to see instead and why.

If you want to support us by writing the enhancement yourself consider what kind of change it is:

- **Major changes** that you wish to contribute to the project should be discussed first on our **dev mailing list** so that we can better coordinate our efforts, prevent duplication of work, and help you to craft the change so that it is successfully accepted into the project.
- **Small changes** can be crafted and submitted to our repository as a **pull or merge request**.

Nevertheless open an issue for documentation purposes with the following template:

~~~.md
[Short description of suggestion]

**Steps which explain the enhancement**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Current and suggested behavior**

[Describe current and suggested behavior here]

**Why would the enhancement be useful to most users**

[Explain why the enhancement would be useful to most users]

[List some other text editors or applications where this enhancement exists]

**Screenshots and GIFs**

![Screenshots and GIFs which demonstrate the steps or part of JuPedSim the enhancement suggestion is related to](url)

**JuPedSim Version:** [Enter JuPedSim version here]
**OS and Version:** [Enter OS name and version here]~

~~~

## Writing Code

### Coding style

In JuPedSim we try to code according to the *Stroustrup* style of formatting/indenting.
If you want (or have) to write code in JuPedSim you really **need** to respect that style.
This is important not just aesthetically but also practically. Diff commits are much more clearer and cleaner.

The code is formatted using the automatic formatter [astyle](http://astyle.sourceforge.net/astyle.html)  with the option `--style=stroustrup`:

> Stroustrup style formatting/indenting uses stroustrup brackets.
> Brackets are broken from function definitions only.
> Brackets are attached to everything else including
> namespaces, classes, and statements within a function, arrays, structs, and enums.
> This style frequently is used with an indent of 5 spaces.

Here is an **example:**

```c++
int Foo(bool isBar)
{
     if (isBar) {
          bar();
          return 1;
     } else
          return 0;
}
```

#### Tabs vs Spaces
This can be a long and religious discussion, to make it short *DO NOT* use tabs, just spaces.

Here are some hints to configure your editor in order to use the *stroustrup* style

- **Emacs**:

  Add this to your ```.emacs```
  ```shell
  (setq c-default-style "stroustrup" c-basic-offset 5)
  (setq indent-tabs-mode nil)
  ```
- **Vim**:

  Set in your config file these variables
  ```javascript
  :set autoindent
  :set cindent
  :set expandtab
  :set shiftwidth=5
  :set softtabstop=5
  ```

- **Eclipse**:
    Here is a [plugin](http://astyleclipse.sourceforge.net/) for astyle in eclipse.
    Read also
    [How to change indentation width in eclipse?](https://superuser.com/questions/462221/how-do-i-reliably-change-the-indentation-width-in-eclipse)

- **Clion**: preferences -> Editor -> Code Style --> C++ --> predefined style --> Stroustrup.  See also following screen-shot. 
 ![clion-screenshot]({{ site.baseurl }}/img/screenshot.png)

### Unit tests

Testing should be enabled by `CMake` as follows:
```javascript
cmake -DBUILD_TESTING=ON ..
```

After compiling (`make`) the tests can be called as follows:

```javascript
ctest
```

This will take some hours time (depending on your machine). Threfore, a quick testing could be used:
```javascript
ctest -R Boost
```

which run a limited set of compiled unit tests on the code.

## Writing a unit test
If you write a unit test the first lines in your file should be

```c++
#define BOOST_TEST_MODULE MyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include classtotest.h

BOOST_AUTO_TEST_SUITE(MyTestClass)
```

Then you can start implementing your test cases by using
```c++
BOOST_AUTO_TEST_CASE(MyTestCase) 
{
  ...
}
```

For each method or function you want to test you should write a new test case and give it a speaking name.

### Verification and validation

Besides unit testing, we use in JuPedSim python-driven tests for verification and validation purposes.
Several validation and verification tests for *JuPedSim* (jpscore) are defined in the following section (e.g. Rimea and NIST).
In order to make the nightly builds run automatically, consider the following steps, before adding new tests.
This procedure is also recommended to make simulations with several inifiles e.g. different seeds.

To write additional tests, create a directory under *Utest/*.


- Create in *./jpscore/Utest/* a new direct with a descriptive name. For
   example: */Utest/test\_case1/*
- Put in that directory an ini-file (referred to as "master-inifile")
   and all the relevant files for a simulation, e.g. geometry file, etc. In the master-inifile you can use python syntax
   
   Example:
```xml
   <max_sim_time>[3,4]</max_sim_time> 
   <seed>range(1, 10)</seed>
```
- run the script `makeini.py` with the obligatory option `-f`: Using the
    aforementioned example the call is:
```bash
   python makeini.py -f test_case1/inifile.xml
```

The Script is going to create two directories:
- test\_case/trajectories: Here live the simulation trajectories.
- test\_case/inifiles: and here the inifiles, that will be produced  based on the master inifile (in this case test\_case1/inifile.xml).
    Note, that the geometry file and the trajectory files are all relative to the project files in the directory *inifiles*.


- Write a runtest-script. Here you have to define the logic of your test. What should be tested? When does the file succeed or file? etc.

Your script has to start with the following lines:

```python
#!/usr/bin/env python
import os
import sys
from sys import *
sys.path.append(os.path.abspath(os.path.dirname(sys.path[0])))
from JPSRunTest import JPSRunTestDriver
from utils import *
```

After including these lines you can write the test you want to perform for every ini-file generated from the makeini.py script.
The method signature must contain at least two arguments to receive the inifile and the trajectory file from the simulation.

```python
def runtest(inifile, trajfile):
```

You can also use more than these arguments like this:

```python
def runtest(inifile, trajfile, x, y, z):
```

If you need a more complex example of how to use more arguments for further calculations please see runtest_14.py.
If a test has to fail because an error occurs or a necessary condition is not fulfilled  you can simply exit the script by using something like

```python
if condition_fails: 
  exit(FAILURE)
```
Once you have written your test you have to make your script executable, so it has to contain a main function which calls the test:

```python
if __name__ == "__main__":
  test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0])
  test.run_test(testfunction=runtest)
  logging.info("%s exits with SUCCESS" % (argv[0]))
  exit(SUCCESS)
```


## Writing Documentation

### Comments
Comments have to be written in **English** everywhere. Please use markdown where applicable.

### Documenting new features
Please update the changelog with every feature/fix you provide so we can keep track of changes for new versions of JuPedSim.

### Sample Templates
```
/*!
 * \file [filename]
 *
 * \author [your name]
 * \date
 * \version     v[x.y]
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 * [your comment here]
 */
```

```
/*!
 * \class [class name]
 *
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \author [your name]
 * \date
 */
```

```
/*!
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \param[in] [name of input parameter] [its description]
 * \param[out] [name of output parameter] [its description]
 * \return [information about return value]
 * \sa [see also section]
 * \note [any note about the function you might have]
 * \warning [any warning if necessary]
 */
```

## Commiting Code

### Commiting guidelines
Please write clear and concise commit messages so that
your co-developers can directly grasp what changes on the code are you committing/pushing. Please do this with respect
to the following points:
- Name every class (file if you only change a single file) you changed and right after that a brief description of your
change.
- Use markdown if you want do make a longer description than two sentences.
- Reference issues and pull requests liberally if your commit is connected to one.
- When only changing documentation start with `:memo:`
- Consider starting the commit message with an applicable emoji:  
    - ![:new:]({{ site.baseurl }}/img/new.png) `:new:` when adding a new feature  
    - ![:art:]({{ site.baseurl }}/img/art.png) `:art:` when improving the format/structure of the code  
    - ![:racehorse:]({{ site.baseurl }}/img/racehorse.png) `:racehorse:` when improving performance  
    - ![:memo:]({{ site.baseurl }}/img/memo.png) `:memo:` when writing docs  
    - ![:penguin:]({{ site.baseurl }}/img/penguin.png) `:penguin:` when fixing something on Linux  
    - ![:apple:]({{ site.baseurl }}/img/apple.png) `:apple:` when fixing something on Mac OS  
    - ![:checkered_flag:]({{ site.baseurl }}/img/checkered_flag.png) `:checkered_flag:` when fixing something on Windows  
    - ![:bug:]({{ site.baseurl }}/img/bug.png) `:bug:` when fixing a bug  
    - ![:fire:]({{ site.baseurl }}/img/fire.png) `:fire:` when removing code or files  
    - ![:green_heart:]({{ site.baseurl }}/img/green_heart.png) `:green_heart:` when fixing the CI build  
    - ![:white_check_mark:]({{ site.baseurl }}/img/white_check_mark.png) `:white_check_mark:` when adding tests  
