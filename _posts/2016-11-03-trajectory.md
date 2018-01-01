---
layout: post2
title: Trajectory 
subtitle: file
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-03-trajectory.html
nav:
- page: XML format
  permalink: 2016-11-03-trajectory.html#xml-format
- page: TXT format
  permalink: 2016-11-03-trajectory.html#txt-format
- page: Back to top
  permalink: 2016-11-03-trajectory.html#top
---

`JPSreport` supports the formats `.xml` and `.txt` in current version. The format is the same with the output from `JPScore`. 

***

Note the unit of the data in trajectory file should be all in `m`.

## XML format

The file should include at least two main sections: header and frames.

- Header

         <header version = "0.5">
            <agents> 1 </agents>
            <frameRate> 8 </frameRate>
         </header>

     - agents: The total number of agents in the trajectory data.

     - frameRate: The frame rate. 

- Frames: gives trajectory information in each frame.

		<frame ID="0">
		<agent ID="1" x="6.60" y="3.33"	z="0.30" 
                      rA="0.17" rB="0.24" eO="-1.68" eC="0"/>
		</frame>
		
		<frame ID="1">
		<agent ID="1" x="6.58" y="3.32" z="0.30" 
                      rA="0.31" rB="0.23" eO="-1.75" eC="54"/>
		</frame>

	- **ID**: mandatory, the id of the pedestrians starting with 1.
	- **x, y, z**: mandatory, the position of the agent.
	- **xVel, yVel, zVel**: Optional, the instantaneous velocity. They are not used in `JPSreport`.
	- **rA, rB**: Optional. The shape which is defined by a circle (ellipse) drawn around a human. They are not used in `JPSreport`.
	- **eO, eC**: Optional. They are the "ellipseOrientation" and the "ellipseColo"'. They are not used in `JPSreport`.


- Sample trajectory file

  ```xml

    <?xml version="1.0" encoding="UTF-8"?>
		
		<trajectories>
			<header version = "0.5">
				<agents>1</agents>
				<frameRate>8</frameRate>
			</header>
		
			<frame ID="0">
				<agent ID="1"	x="6.60"	y="3.33"	z="0.30"/>
			</frame>
		
			<frame ID="1">
				<agent ID="1"	x="6.58"	y="3.32"	z="0.30"/>
			</frame>
		</trajectories>

  ```

## TXT format

A sample trajectory in .txt format is present as below:

```xml
#description: optional description
#framerate: 16
#geometry: /home/sim/corridor.xml
#ID: the agent ID
#FR: the current frame
#X,Y,Z: the agents coordinates in meters
				
#ID	FR	X		Y		Z
1	0	8.21	131.57	0.00
2	0	8.41	133.42	0.00
1	1	8.21	131.57	0.00
2	1	8.41	133.42	0.00
```

The line starting with `#framerate` and `#ID	FR	X	Y	Z` are necessary. 

The order of each column is irrelevant. JPSreport will check the meaning of each column from the comments 

```xml
#ID	 FR	 X	Y  Z
```

**Hint:**
Use the script `scripts/txt2txt.py` to format a txt-trajectory according to the  above-mentioned requirements.
