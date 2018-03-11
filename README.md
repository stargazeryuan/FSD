### FS_detection
#Fire and smoke detection collaboration repository

### Milesones
**Dataset** :

1- dataset annotation.

**Motion Detection** :

2- motion detection algorithm suited for smoke and fire in early stage - robust and fast.

**Motion-based FS detection** :

3- training cnns for indirect fire and smomke detection with motion by motion-masked, cropped patches of video frames.

**Motion-less FS detection** :

4- training cnns for direct fire and smoke detection without motion. 

### Done so far:

1- finished labeling MIVIS dataset.




### Tasks as of meeting on 11.03.2018 for 1st and 2nd milestone:

1.Try different background subtraction (BGS) algorithms and make comparison.

1.1 dynamic background update and dark channel prior.

2.2 from BGS Library (https://github.com/andrewssobral/bgslibrary).

2. with a valid BGS output, give the cropped, masked patch to AlexNet for training.

3. finalizing first milestone by finishing dataset annotation.
