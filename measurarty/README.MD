MEASURARTY as of iGEM 2014 Team Aachen

measurarty is a collection of Matlab functions for the analysis of
the chip-images taken from our device.
This logic is meant to identify any fluorescence signal on the chips in a
robust manner.

The interesting part of this algorithm is the concatenation of quite simple
algorithms. We use a lot of thresholding to bring the images into a shape we
like to process with the smoothness index, our similarity score for classifying
the regions in the image.
Even SRM is quite lightweight. Unfortunately we do not use its full power of
hierarchical edge/region creation.
Yet it is an interesting method for clustering, which in contrast to k-means, is also deterministic.
Additionally there has already been knowledge about this algorithm in the group.

In order to run our code we would like to ask you to download, install (and maybe compile)

http://www.mathworks.com/matlabcentral/fileexchange/25619-image-segmentation-using-statistical-region-merging

Our adapted version of srm.m is singlesrm.m and equals that version except for a removed loop.
For questions regarding the compilation of the mex-file I point to the comments
section of FileExchange for this code, particularly to my post from 12 Dec. 2013.