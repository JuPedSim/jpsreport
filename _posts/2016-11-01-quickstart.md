---
layout: post
title: Quickstart 
subtitle: 
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-01-quickstart.html
---


The organization of `JPSreport` is as follows

![structure]({{ site.baseurl}}/img/usage_JPSreport_scaled.png)

## Preparing the input files

Three input files are required to run `JPSreport`:

- [Trajectory file](2016-11-03-trajectory): Pedestrian's 2D position information over time. 
`.xml` and `.txt` formats are supported.
* [Geometry file](2016-11-02-geometry): Geometry for a certain trajectory data. 
  This file should be in `.xml` format.
* [Configuration file](2016-11-01-inifile): This inifile gives some information related to each measurement method. e. g. 
  the location of measurement areas, the chosen method etc. 
  This file should be in `.xml` format.
  


## Run JPSreport

After succesfully [compiling](2016-11-01-configuration) the code you can just run
`jpsreport` in a terminal as follows:

```bash
./bin/jpsreport inifile.xml
```


`inifile.xml` defines all the needed information to start a analysis with
`JPSreport`. 


## Results

Possible output of `JPSreport` includes data for plotting fundamental diagrams, 
Voronoi diagrams and profiles of pedestrians etc. in a given geometry. 
All the output data, e.g. density and speed, 
are stored in different folders as plain text in ASCII format.


After a successful analysis additional folder named `Output` will be created in
the same directory as the used inifile. 
It contains the basic data including plain 
text and eventually figures (depending on your specifications in the inifile).
